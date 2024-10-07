#pragma once
#include <QtCore>
#include <QLoggingCategory>
#include <QThread>
#include <QTime>
#include <iostream>

extern "C" {

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"
#include "lib/l2cap.h"
#include "lib/uuid.h"

#include "src/shared/mainloop.h"
#include "src/shared/util.h"
#include "src/shared/att.h"
#include "src/shared/queue.h"
#include "src/shared/timeout.h"
#include "src/shared/gatt-db.h"
#include "src/shared/gatt-server.h"


}
#define ATT_CID 4
#define UUID_GAP			0x1800
#define UUID_GATT			0x1801
#define test_device_name "zavdimka"

class SimpltGattRunner: public QThread {
    Q_OBJECT
    
    protected:
        void run();
    signals:
        void finish();
};

class SimpleGattServ : public QObject{
    Q_OBJECT

    protected:

        QThread* thread;
        SimpltGattRunner* thread_ble;
        bool is_run;

        int fd;
        struct bt_att *att;
        struct gatt_db *db;
        struct bt_gatt_server *gatt;
        uint16_t gatt_svc_chngd_handle;

        uint8_t *device_name;
        size_t name_len;

        bool svc_chngd_enabled;
        

        int l2cap_le_att_listen_and_accept(bdaddr_t *src, int sec,
							uint8_t src_type);
        void populate_db();
        void populate_gap_service();
        void populate_gatt_service();
        bool server_create();

        static void att_disconnect_cb(int err, void *user_data);

        static void gap_device_name_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data);

        static void gap_device_name_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data);

        static void gap_device_name_ext_prop_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data);
        static void gatt_service_changed_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data);

        static void gatt_svc_chngd_ccc_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data);

        static void gatt_svc_chngd_ccc_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data);

        


        virtual void populate_user_service() = 0;

    public:

        SimpleGattServ(QObject* parent = 0);

        void wait();


    public slots:
        void stop();

    private slots:
        void doWork();

};