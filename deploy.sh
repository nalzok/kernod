#!/usr/bin/env bash

mkdir -p /usr/local/var/www/fcgi-bin /usr/local/var/www/run
install -m 0555 cmake-build-debug/kernod /usr/local/var/www/fcgi-bin
echo $LOGIN_PASSWD | sudo -S : 2>/dev/null  # do nothing
sudo /usr/local/sbin/kfcgi -d -r \
    -s /usr/local/var/www/run/httpd.sock \
    -p /usr/local/var/www \
    -u `whoami` -U `whoami` -- /fcgi-bin/kernod
