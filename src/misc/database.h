#include <bson.h>
#include <mongoc.h>

#include <hiredis.h>

mongoc_client_t *client;
redisContext *redisClient;
