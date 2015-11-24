#include <bson.h>
#include <bcon.h>
#include <mongoc.h>
#include <misc/database.h>

void	mongo_connect(char *db_name)
{
	mongoc_client_t      *client;
	bson_t               *insert;
	bson_error_t          error;
	mongoc_collection_t  *collection;

	printf("Connecting to database: %s...\n", db_name);

	client = mongoc_client_new("mongodb://localhost:27017");

	mongoc_client_get_database(client, db_name);


	collection = mongoc_client_get_collection(client, db_name, "coll_name");

	insert = BCON_NEW ("hello", BCON_UTF8 ("world"));

	if (!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, insert, NULL, &error)) {
		fprintf (stderr, "%s\n", error.message);
	}

	bson_destroy(insert);

	mongoc_client_destroy(client);
	mongoc_cleanup();
	return NULL;
}
