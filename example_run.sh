#!/bin/bash

# Using gmail as an example, we can use whatever.
export MAIL_HOST=smtp.gmail.com:587
export MAIL_USER=xxxxxx@gmail.com
export MAIL_PASSWORD=xxxxxx
export MAIL_FROM=CHAPI
export ORGANIZATION=CHAPI
export DOMAIN=chapi.me

export CFLAGS=""
export LDFLAGS="-lconfig -lcurl -lflate"

kore clean

kore build

kore run
