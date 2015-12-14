#!/bin/bash

export ENVIRONMENT="chapi_development"
export MAIL_HOST="smtp.gmail.com:587"
export MAIL_USER="xxxxxx@gmail.com"
export MAIL_PASSWORD="xxxxxx"
export MAIL_FROM="support@chapi.me"
export ORGANIZATION="CHAPI"
export DOMAIN="chapi.me"


export CFLAGS="$(pkg-config --cflags --libs libmongoc-1.0) $(pkg-config --cflags --libs hiredis)"
export LDFLAGS="-lconfig -lcurl -lflate -lmongoc-1.0 -lscrypt -lhiredis"

sudo systemctl start mongodb
sudo systemctl start redis

kore clean

kore build

kore run
