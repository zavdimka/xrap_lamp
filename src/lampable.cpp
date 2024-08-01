#include "lampable.h"


void LampaBle::populate_user_service(){
    bt_uuid_t uuid;
	struct gatt_db_attribute *service, *mt, *lz, *hr;

    qInfo() << "populate_user_service";

    bt_uuid32_create(&uuid, UUID_LAMP);
	service = gatt_db_add_service(db, &uuid, true, 8);
	lamp_handle = gatt_db_attribute_get_handle(service);

	bt_uuid32_create(&uuid, UUID_METEO);
	mt = gatt_db_service_add_characteristic(service, &uuid,
						BT_ATT_PERM_READ,
						BT_GATT_CHRC_PROP_READ,
						meteo_ccc_read_cb, NULL, (void*)this);
	meteo_handle = gatt_db_attribute_get_handle(mt);

    bt_uuid32_create(&uuid, UUID_LUZ);
    lz = gatt_db_service_add_characteristic(service, &uuid,
						BT_ATT_PERM_WRITE,
						BT_GATT_CHRC_PROP_WRITE,
						NULL, luz_ccc_write_cb, (void*)this);
	luz_handle = gatt_db_attribute_get_handle(lz);

    bt_uuid32_create(&uuid, UUID_HRAP);
    hr = gatt_db_service_add_characteristic(service, &uuid,
						BT_ATT_PERM_READ,
						BT_GATT_CHRC_PROP_READ,
						hrap_ccc_read_cb, NULL, (void*)this);
	hrap_handle = gatt_db_attribute_get_handle(lz);

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

	if (!value || len != 3) {
		ecode = BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN;
		goto done;
	}

	if (offset) {
		ecode = BT_ATT_ERROR_INVALID_OFFSET;
		goto done;
	}

    
    emit self->update_led_control(value[0], value[1], value[2]);

done:
	gatt_db_attribute_write_result(attrib, id, ecode);
}

void LampaBle::hrap_ccc_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	LampaBle *self = (LampaBle*)user_data;
	uint8_t value[12];
    memcpy(value, self->hrap_prediction, 12);
    qInfo() << "hrap_ccc_read_cb";

	gatt_db_attribute_read_result(attrib, id, 0, value, 12);
}

void LampaBle::update_hrap_prediction(float a, float b, float c){
    hrap_prediction[0] = a;
    hrap_prediction[1] = b;
    hrap_prediction[2] = c;
	qInfo() << "update_hrap_prediction";
}

void LampaBle::update_meteo_data(M701::M701_data data){
	memcpy(&meteo_data,&data, sizeof(data));
	qInfo() << "update_meteo_data";
}
