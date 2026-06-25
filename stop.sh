#!/bin/bash

systemctl stop death.service
systemctl disable death.service
rm -rf /etc/systemd/system/death.service