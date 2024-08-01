#pragma once
#include "simple-gattserv.h"
#include "M701.h"

#define UUID_LAMP  0x18EEA800
#define UUID_METEO 0x18EEA801
#define UUID_LUZ   0x18EEA802
#define UUID_HRAP  0x18EEA803

class LampaBle: public SimpleGattServ{
    Q_OBJECT

    protected:
        uint16_t lamp_handle;
        uint16_t meteo_handle;
        uint16_t luz_handle;
        uint16_t hrap_handle;

        void populate_user_service() override ; 

        float hrap_prediction[3];
        M701::M701_data meteo_data;
        

        static void meteo_ccc_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data);

        static void luz_ccc_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data);

        static void hrap_ccc_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data);

    public:
        LampaBle(QObject* parent = 0):
            SimpleGattServ(parent){

            };

    public slots:
        void update_hrap_prediction(float a, float b, float c);
        void update_meteo_data(M701::M701_data data);

    signals:
        void update_led_control(uint8_t saturation, uint8_t effect, uint8_t color);
        
};