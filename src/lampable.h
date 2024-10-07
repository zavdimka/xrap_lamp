#pragma once
#include "simple-gattserv.h"
#include "M701.h"
#include <functional>
#include <utility>


class LampaBle: public SimpleGattServ{
    Q_OBJECT
    using LampCallback = std::function<void(uint8_t*)>;
    //typedef void (*LampCallback)(uint8_t*);

    protected:
        uint16_t lamp_handle;
        uint16_t meteo_handle;
        uint16_t luz_handle;
        uint16_t hrap_handle;
        

        void populate_user_service() override ; 

        float hrap_prediction[3];
        M701::M701_data meteo_data;
        LampCallback lamp_callback;

        static void meteo_ccc_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data);

        static void luz_ccc_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data);

        static void hrap_ccc_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data);

        static void luz_ccc_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data);

        static void gatt_svc_hrap_ccc_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data);

        static void gatt_svc_hrap_ccc_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data);

    public:
        LampaBle(QObject* parent = 0):
            SimpleGattServ(parent){
                lamp_callback = NULL;
                hrap_notfi_en = false;
            };

        void set_lamp_callback(LampCallback callback){
            lamp_callback = callback;
        }

        bool hrap_notfi_en;
        std::vector<uint8_t> hrap_history;

    public slots:
        void update_hrap_prediction(float a, float b, float c);
        void update_meteo_data(M701::M701_data data);

    signals:
        void update_led_control(uint8_t cmd, uint8_t arg1, uint8_t arg2, uint8_t arg3);
        
};