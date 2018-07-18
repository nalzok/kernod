#!/usr/bin/env bash

echo $LOGIN_PASSWD | sudo -S : 2>/dev/null  # authenticate myself
sudo mount -t devfs devfs /usr/local/var/www/dev
mkdir -p /usr/local/var/www/fcgi-bin
mkdir -p /usr/local/var/www/run
install -m 0555 cmake-build-debug/kernod /usr/local/var/www/fcgi-bin
cp -R static /usr/local/var/www
sudo /usr/local/sbin/kfcgi -d -r -v \
    -s /usr/local/var/www/run/httpd.sock \
    -p /usr/local/var/www \
    -u `whoami` -U `whoami` -- /fcgi-bin/kernod
