# CH-API

## Telegram alike, minimum implementation / Toy project.

In case you want to know what are we following:

https://core.telegram.org/methods

As the description says: we aim to do a minimal implementation.

#### Important note: use localhost to navigate the site.

### Requirements:

## Packages for Arch Linux:
* base-devel (pacman) 
* kore.io (manual)
* mongodb (pacman)
* mongo-c-driver (pacaur)
* libconfig (pacman)

## Useful readings / APIs

* https://kore.io/doc
* http://api.mongodb.org/c/current
* http://www.hyperrealm.com/libconfig/libconfig_manual.html#Using-the-Library-from-a-C-Program

## Build / Run
You can make an alias or run it like this:

kore clean; env CFLAGS="$(pkg-config --cflags --libs libmongoc-1.0)" LDFLAGS="-lconfig -lmongoc-1.0" kore build; kore run
