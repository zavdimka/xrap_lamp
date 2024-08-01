#!/bin/sh -e

btmgmt -i hci0 power off
btmgmt -i hci0 le on
btmgmt -i hci0 connectable on
btmgmt -i hci0 name "zavdimka"
btmgmt -i hci0 advertising on
btmgmt -i hci0 power on