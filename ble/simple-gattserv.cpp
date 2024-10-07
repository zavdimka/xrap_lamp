#include "simple-gattserv.h"


SimpleGattServ::SimpleGattServ(QObject* parent)//:
   // QObject(parent)
{
	thread = new QThread(parent);
	thread_ble = new SimpltGattRunner;
    is_run = true;
    connect(thread, SIGNAL(started()), this, SLOT(doWork()));
	connect(thread_ble, SIGNAL(finished()), this, SLOT(doWork()));
    this->moveToThread(thread);
    thread->start();
}

void SimpltGattRunner::run(){

	qInfo() << "Go to mainloop_run";
	mainloop_run();
	qInfo() << "Finish mainloop_run";
	quit();
}

void SimpleGattServ::doWork(){
	qInfo() << "doWorks";
	bdaddr_t src_addr;
    int sec = BT_SECURITY_LOW;
	uint8_t src_type = BDADDR_LE_PUBLIC;
	int dev_id = hci_devid("hci0");

    if (dev_id == -1){
		qWarning() << "hci0 not found";
		bacpy(&src_addr, BDADDR_ANY);
	}
	else if (hci_devba(dev_id, &src_addr) < 0) {
		qCritical() << "Adapter not available";
	}

    fd = l2cap_le_att_listen_and_accept(&src_addr, sec, src_type);
    if (fd < 0) {
		qCritical() << "Failed to accept L2CAP ATT connection";
		return ;
	}

	qInfo() << "Go to mainloop init";

	mainloop_init();

	qInfo() << "Go to server_create";

	if (!server_create()) {
		close(fd);
		qCritical() << "Failed to create server";
		return;
	}
	//mainloop_run();
	thread_ble->start();
}

void SimpleGattServ::stop(){
	is_run = false;
	mainloop_quit();
	emit thread->quit();
}


void SimpleGattServ::wait(){
	thread_ble->wait();
	thread->wait();
}

bool SimpleGattServ::server_create(){
	int mtu = 0;
	size_t name_len = strlen(test_device_name);

	att = bt_att_new(fd, false);
	if (!att){
		qCritical() << "Failed to initialze ATT transport layer";
		goto fail;
	}

	if (!bt_att_set_close_on_unref(att, true)) {
		qCritical() << "Failed to set up ATT transport layer";
		goto fail;
	}

	if (!bt_att_register_disconnect(att, att_disconnect_cb, NULL,
									NULL)) {
		qCritical() << "Failed to set ATT disconnect handler";
		goto fail;
	}
	device_name = (uint8_t*)malloc(name_len + 1);
	memcpy(device_name, test_device_name, name_len);
	device_name[name_len + 1] = '\0';

	db = gatt_db_new();
	//gatt_db_clear(db);

	if (!db) {
		qCritical() << "Failed to create GATT database";
		goto fail;
	}

	gatt = bt_gatt_server_new(db, att, mtu, 0);
	if (!gatt) {
		qCritical() << "Failed to create GATT server";
		goto fail;
	}

	populate_db();

	return true;

fail:
	gatt_db_unref(db);
	bt_att_unref(att);
	return false;
}

int SimpleGattServ::l2cap_le_att_listen_and_accept(bdaddr_t *src, int sec,
							uint8_t src_type)
{
	int sk, nsk;
	struct sockaddr_l2 srcaddr, addr;
	socklen_t optlen;
	struct bt_security btsec;
	char ba[18];

	sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (sk < 0) {
		qCritical() << "Failed to create L2CAP socket";
		return -1;
	}

	/* Set up source address */
	memset(&srcaddr, 0, sizeof(srcaddr));
	srcaddr.l2_family = AF_BLUETOOTH;
	srcaddr.l2_cid = htobs(ATT_CID);
	srcaddr.l2_bdaddr_type = src_type;
	bacpy(&srcaddr.l2_bdaddr, src);

	if (bind(sk, (struct sockaddr *) &srcaddr, sizeof(srcaddr)) < 0) {
		qCritical() << "Failed to bind L2CAP socket";
		goto fail;
	}

	/* Set the security level */
	memset(&btsec, 0, sizeof(btsec));
	btsec.level = sec;
	if (setsockopt(sk, SOL_BLUETOOTH, BT_SECURITY, &btsec,
							sizeof(btsec)) != 0) {
		qCritical() << "Failed to set L2CAP security level";
		goto fail;
	}

	if (listen(sk, 1) < 0) {
		qCritical() << "Listening on socket failed";
		goto fail;
	}

	qInfo() << "Started listening on ATT channel. Waiting for connections";

	while (is_run) {
		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(sk, &read_fds);

		struct timeval timeout;
		int select_status;
		timeout.tv_sec = 0;  // 1s timeout
		timeout.tv_usec = 1000;
		select_status = select(sk+1, &read_fds, NULL, NULL, &timeout);
		if (select_status == -1) {
			// ERROR: do something
			qCritical() << "error";
		} else if (select_status > 0) {
			qInfo() << "we have dat";
			break;  // we have data, we can accept now
		}
		// otherwise (i.e. select_status==0) timeout, continue
	}
	if (!is_run) goto fail;


	memset(&addr, 0, sizeof(addr));
	optlen = sizeof(addr);
	nsk = accept(sk, (struct sockaddr *) &addr, &optlen);
	if (nsk < 0) {
		qCritical() << "Accept failed";
		goto fail;
	}

	ba2str(&addr.l2_bdaddr, ba);
	qInfo() << "Connect from " << ba;
	close(sk);

	return nsk;

fail:
	close(sk);
	return -1;
}

void SimpleGattServ::populate_db(){
	populate_gap_service();
	populate_gatt_service();
	populate_user_service();
}

void SimpleGattServ::populate_gap_service(){
	bt_uuid_t uuid;
	struct gatt_db_attribute *service, *tmp;
	uint16_t appearance;

	/* Add the GAP service */
	bt_uuid16_create(&uuid, UUID_GAP);
	service = gatt_db_add_service(db, &uuid, true, 6);

	/*
	 * Device Name characteristic. Make the value dynamically read and
	 * written via callbacks.
	 */
	bt_uuid16_create(&uuid, GATT_CHARAC_DEVICE_NAME);
	gatt_db_service_add_characteristic(service, &uuid,
					BT_ATT_PERM_READ | BT_ATT_PERM_WRITE,
					BT_GATT_CHRC_PROP_READ |
					BT_GATT_CHRC_PROP_EXT_PROP,
					gap_device_name_read_cb,
					gap_device_name_write_cb,
					this);

	bt_uuid16_create(&uuid, GATT_CHARAC_EXT_PROPER_UUID);
	gatt_db_service_add_descriptor(service, &uuid, BT_ATT_PERM_READ,
					gap_device_name_ext_prop_read_cb,
					NULL, this);

	/*
	 * Appearance characteristic. Reads and writes should obtain the value
	 * from the database.
	 */
	bt_uuid16_create(&uuid, GATT_CHARAC_APPEARANCE);
	tmp = gatt_db_service_add_characteristic(service, &uuid,
							BT_ATT_PERM_READ,
							BT_GATT_CHRC_PROP_READ,
							NULL, NULL, this);

	/*
	 * Write the appearance value to the database, since we're not using a
	 * callback.
	 */
	put_le16(128, &appearance);
	gatt_db_attribute_write(tmp, 0, (const uint8_t*) &appearance,
							sizeof(appearance),
							BT_ATT_OP_WRITE_REQ,
							NULL,   NULL, //confirm_write,
							NULL);

	gatt_db_service_set_active(service, true);
}

void SimpleGattServ::populate_gatt_service(){
	bt_uuid_t uuid;
	struct gatt_db_attribute *service, *svc_chngd;

	/* Add the GATT service */
	bt_uuid16_create(&uuid, UUID_GATT);
	service = gatt_db_add_service(db, &uuid, true, 4);

	bt_uuid16_create(&uuid, GATT_CHARAC_SERVICE_CHANGED);
	svc_chngd = gatt_db_service_add_characteristic(service, &uuid,
			BT_ATT_PERM_READ,
			BT_GATT_CHRC_PROP_READ | BT_GATT_CHRC_PROP_INDICATE,
			gatt_service_changed_cb,
			NULL, this);
	gatt_svc_chngd_handle = gatt_db_attribute_get_handle(svc_chngd);

	bt_uuid16_create(&uuid, GATT_CLIENT_CHARAC_CFG_UUID);
	gatt_db_service_add_descriptor(service, &uuid,
				BT_ATT_PERM_READ | BT_ATT_PERM_WRITE,
				gatt_svc_chngd_ccc_read_cb,
				gatt_svc_chngd_ccc_write_cb, this);

	gatt_db_service_set_active(service, true);
}



/*CallBacks section*/


void SimpleGattServ::gap_device_name_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	SimpleGattServ *self = (SimpleGattServ*)user_data;
	uint8_t error = 0;
	size_t len = 0;
	const uint8_t *value = NULL;
	qInfo() << "GAP Device Name Read called";
	len = self->name_len;
	if (offset > len) {
		error = BT_ATT_ERROR_INVALID_OFFSET;
		goto done;
	}

	len -= offset;
	value = len ? &self->device_name[offset] : NULL;

done:
	gatt_db_attribute_read_result(attrib, id, error, value, len);
}

void SimpleGattServ::gap_device_name_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	SimpleGattServ *self = (SimpleGattServ*)user_data;
	uint8_t error = 0;

	qInfo() << "GAP Device Name Write called";

	/* If the value is being completely truncated, clean up and return */
	if (!(offset + len)) {
		free(self->device_name);
		self->device_name = NULL;
		self->name_len = 0;
		goto done;
	}

	/* Implement this as a variable length attribute value. */
	if (offset > self->name_len) {
		error = BT_ATT_ERROR_INVALID_OFFSET;
		goto done;
	}

	if (offset + len != self->name_len) {
		uint8_t *name;

		name = (uint8_t*)realloc((void*)self->device_name, offset + len);
		if (!name) {
			error = BT_ATT_ERROR_INSUFFICIENT_RESOURCES;
			goto done;
		}

		self->device_name = name;
		self->name_len = offset + len;
	}

	if (value)
		memcpy(self->device_name + offset, value, len);

done:
	gatt_db_attribute_write_result(attrib, id, error);
}

void SimpleGattServ::gap_device_name_ext_prop_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	uint8_t value[2];

	qInfo() << "Device Name Extended Properties Read called";

	value[0] = BT_GATT_CHRC_EXT_PROP_RELIABLE_WRITE;
	value[1] = 0;

	gatt_db_attribute_read_result(attrib, id, 0, value, sizeof(value));
}

void SimpleGattServ::gatt_service_changed_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	qInfo() << "Service Changed Read called";

	gatt_db_attribute_read_result(attrib, id, 0, NULL, 0);
}

void SimpleGattServ::gatt_svc_chngd_ccc_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	//struct server *server = user_data;
	uint8_t value[2];

	qInfo() << "Service Changed CCC Read called";

	value[0] = /*server->svc_chngd_enabled*/ 0 ? 0x02 : 0x00;
	value[1] = 0x00;

	gatt_db_attribute_read_result(attrib, id, 0, value, sizeof(value));
}

void SimpleGattServ::gatt_svc_chngd_ccc_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	SimpleGattServ *self = (SimpleGattServ*)user_data;
	uint8_t ecode = 0;

	qInfo() << "Service Changed CCC Write called";

	if (!value || len != 2) {
		ecode = BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN;
		goto done;
	}

	if (offset) {
		ecode = BT_ATT_ERROR_INVALID_OFFSET;
		goto done;
	}

	if (value[0] == 0x00)
		self->svc_chngd_enabled = false;
	else if (value[0] == 0x02)
		self->svc_chngd_enabled = true;
	else
		ecode = 0x80;

	qInfo() << "Service Changed Enabled: " <<
				(self->svc_chngd_enabled ? "true" : "false");

done:
	gatt_db_attribute_write_result(attrib, id, ecode);
}

void SimpleGattServ::att_disconnect_cb(int err, void *user_data)
{
	qInfo() << "Device disconnected: " <<strerror(err);
	mainloop_quit();
}