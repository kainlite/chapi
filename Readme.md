# CH-API

## Telegram alike, minimum implementation / Toy project.

In case you want to know what are we following:

https://core.telegram.org/methods

As the description says: we aim to do a minimal implementation.

#### Important note: use localhost to navigate the site.

### Requirements:

## Packages for Arch Linux:
* base-devel (pacman)
* kore.io (manual, it must be compiled with TASKS=1)
* curl (pacman)
* mongodb (pacman)
* libconfig (pacman)
* libscrypt (pacaur) (requires some manual work -fPIC in LDFLAGS)
* mongo-c-driver (pacaur)
* libflate (pacaur)

## Useful readings / APIs

* https://kore.io/doc
* http://api.mongodb.org/c/current
* http://www.hyperrealm.com/libconfig/libconfig_manual.html#Using-the-Library-from-a-C-Program
* https://github.com/technion/libscrypt/blob/master/main.c

## Build / Run
You can make an alias or run it like this:

kore clean; env CFLAGS="" LDFLAGS="-lconfig -lcurl -lflate" kore build; kore run

Or set your env vars in run.sh (copy of example_run.sh) and just run ./run.sh

## To test requests better use curl, for example:

curl -X POST -k --data \
  "email=jdoe@gmail.com&firstname=John&lastname=Doe&password=123456&password_confirmation" \
  https://localhost:8888/sign_up
