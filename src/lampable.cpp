#include "lampable.h"

// #define UUID_LAMP  "18EEA900"
// #define UUID_METEO "18EEA901"
// #define UUID_LUZ   "18EEA902"
// #define UUID_HRAP  "18EEA903"

#define UUID_LAMP  "18eea900-1234-5432-1234-abcdef000110"
#define UUID_METEO "18eea901-1234-5432-1234-abcdef000110"
#define UUID_LUZ   "18eea902-1234-5432-1234-abcdef000110"
#define UUID_HRAP  "18eea903-1234-5432-1234-abcdef000110"


void LampaBle::populate_user_service(){
    bt_uuid_t uuid;
	struct gatt_db_attribute *service, *mt, *lz, *hr;
	char b[64];

    qInfo() << "populate_user_service";

    bt_string_to_uuid(&uuid, UUID_LAMP);
	bt_uuid_to_string(&uuid, b, 64);
	qInfo() << b;
	service = gatt_db_add_service(db, &uuid, true, 16);
	lamp_handle = gatt_db_attribute_get_handle(service);

	bt_string_to_uuid(&uuid, UUID_METEO);
	bt_uuid_to_string(&uuid, b, 64);
	qInfo() << b;
	mt = gatt_db_service_add_characteristic(service, &uuid,
						BT_ATT_PERM_READ,
						BT_GATT_CHRC_PROP_READ,
						meteo_ccc_read_cb, NULL, (void*)this);
	meteo_handle = gatt_db_attribute_get_handle(mt);

    bt_string_to_uuid(&uuid, UUID_LUZ);
    lz = gatt_db_service_add_characteristic(service, &uuid,
						BT_ATT_PERM_WRITE | BT_ATT_PERM_READ,
						BT_GATT_CHRC_PROP_WRITE | BT_GATT_CHRC_PROP_READ,
						luz_ccc_read_cb, luz_ccc_write_cb, (void*)this);
	luz_handle = gatt_db_attribute_get_handle(lz);

    bt_string_to_uuid(&uuid, UUID_HRAP);
    hr = gatt_db_service_add_characteristic(service, &uuid,
						BT_ATT_PERM_WRITE,
						BT_GATT_CHRC_PROP_WRITE | BT_GATT_CHRC_PROP_NOTIFY,
						NULL, hrap_ccc_write_cb, (void*)this);
	hrap_handle = gatt_db_attribute_get_handle(hr);

	bt_uuid16_create(&uuid, GATT_CLIENT_CHARAC_CFG_UUID);
	gatt_db_service_add_descriptor(service, &uuid,
				BT_ATT_PERM_READ | BT_ATT_PERM_WRITE,
				gatt_svc_hrap_ccc_read_cb,
				gatt_svc_hrap_ccc_write_cb,  (void*)this);

    gatt_db_service_set_active(service, true);
};

void LampaBle::meteo_ccc_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	LampaBle *self = (LampaBle*)user_data;
	
    qInfo() << "meteo_ccc_read_cb";

	gatt_db_attribute_read_result(attrib, id, 0, (const uint8_t*)&self->meteo_data, sizeof(self->meteo_data));
}

void LampaBle::luz_ccc_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	LampaBle *self = (LampaBle*)user_data;
	uint8_t ecode = 0;
    qInfo() << "luz_ccc_write_cb len" << len;

	if (!value || len != 4) {
		ecode = BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN;
		goto done;
	}

	if (offset) {
		ecode = BT_ATT_ERROR_INVALID_OFFSET;
		goto done;
	}

    
    emit self->update_led_control(value[0], value[1], value[2], value[3]);

done:
	gatt_db_attribute_write_result(attrib, id, ecode);
}

void LampaBle::luz_ccc_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	LampaBle *self = (LampaBle*)user_data;
	uint8_t value[16];
	memset(value, 0, 16);
    self->lamp_callback(value);
    qInfo() << "luz_ccc_read_cb";
	gatt_db_attribute_read_result(attrib, id, 0, value, sizeof(value));
}

void LampaBle::hrap_ccc_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	LampaBle *self = (LampaBle*)user_data;
	uint8_t ecode = 0;
    qInfo() << "hrap_ccc_write_cb len" << len;

	if (!value) {
		ecode = BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN;
		goto done;
	}

	if (offset) {
		ecode = BT_ATT_ERROR_INVALID_OFFSET;
		goto done;
	}

	if (self->hrap_notfi_en) {
		if (value[0] == 0x01){
			uint32_t len = self->hrap_history.size();
			qInfo() << "Send length " << len;
			bt_gatt_server_send_notification(self->gatt,
						self->hrap_handle,
						(const uint8_t*)&len, 4, false);
		} else 
		if (value[0] == 0x02){
			uint32_t *offset = (uint32_t*)(value + 1);
			qInfo() << "data slice offset : " <<  *offset; 
			int len = self->hrap_history.size() - *offset;
			qInfo() << "len is : " <<  len; 
			if (len) {
				if (len > 20) len = 20;
				uint8_t buff[20];
				for(int i=0; i < len; i++)
					buff[i] = self->hrap_history[i + *offset];
				qInfo() << "send " << len; 
				bt_gatt_server_send_notification(self->gatt,
						self->hrap_handle,
						(const uint8_t*)buff, len, false);
			}
		}
	}

done:
	gatt_db_attribute_write_result(attrib, id, ecode);
}

void LampaBle::update_hrap_prediction(float a, float b, float c){
    hrap_prediction[0] = a;
    hrap_prediction[1] = b;
    hrap_prediction[2] = c;
	hrap_history.push_back(int(c * 256));
	qInfo() << "update_hrap_prediction";
}

void LampaBle::update_meteo_data(M701::M701_data data){
	memcpy(&meteo_data,&data, sizeof(data));
	qInfo() << "update_meteo_data";
}

void LampaBle::gatt_svc_hrap_ccc_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	LampaBle *self = (LampaBle*)user_data;
	uint8_t value[2];

	qInfo() << "Service Changed CCC Read called";

	value[0] = self->hrap_notfi_en ? 0x02 : 0x00;
	value[1] = 0x00;

	gatt_db_attribute_read_result(attrib, id, 0, value, sizeof(value));
}

void LampaBle::gatt_svc_hrap_ccc_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	LampaBle *self = (LampaBle*)user_data;
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
		self->hrap_notfi_en = false;
	else if (value[0] == 0x01)
		self->hrap_notfi_en = true;
	else
		ecode = 0x80;

	qInfo() << "Service Changed Enabled:" <<
				(self->hrap_notfi_en ? "true" : "false");

done:
	gatt_db_attribute_write_result(attrib, id, ecode);
}
