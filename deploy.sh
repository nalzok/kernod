#!/usr/bin/env bash

echo $LOGIN_PASSWD | sudo -S mkdir -p /var/www/fcgi-bin /var/www/run 2>/dev/null
sudo install -m 0555 cmake-build-debug/kernod /var/www/fcgi-bin/
sudo -S /usr/local/sbin/kfcgi -d -r -v -u `whoami` -U `whoami` -- /fcgi-bin/kernod
