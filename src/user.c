#include <kore/kore.h>
#include <kore/http.h>
#include <kore/tasks.h>

#include <bcon.h>
#include <bson.h>
#include <mongoc.h>

#include <includes/chapi.h>
#include <includes/mail.h>

#include <misc/database.h>

int save_confirmed_email(char *rcode) {
	/* Mongo */
	mongoc_collection_t *collection;
	mongoc_cursor_t *cursor;

	/* Find vars */
	const bson_t *doc;
	bson_t *query;

	/* Update vars*/
	bson_t *update = NULL;
	bson_error_t error;

	/* Find code and if it exists mark as valid email */
	collection = mongoc_client_get_collection(client, getenv("ENVIRONMENT"), "users");
	query = bson_new();
	BSON_APPEND_UTF8 (query, "code", rcode);
	cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

	/* If the user exists update otherwise throw error */
	if (mongoc_cursor_next(cursor, &doc))
	{
		collection = mongoc_client_get_collection (client, getenv("ENVIRONMENT"), "users");


		update = BCON_NEW ("$set", "{",
				   "active", BCON_BOOL (true),
				   "}");

		if (!mongoc_collection_update (collection, MONGOC_UPDATE_NONE, query, update, NULL, &error)) {
			fprintf (stderr, "%s\n", error.message);
		}
		return (KORE_RESULT_OK);
	} else {
		return (KORE_RESULT_ERROR);
	}

	bson_destroy(query);
	bson_destroy(update);

	return (KORE_RESULT_OK);
}

int	confirm_email(struct kore_task *t) {
	int		retval;
	u_int32_t	len;

	char		rcode[CODE_LENGTH];
	char		dst[BUFFER_LENGTH];

	/* Read channel/buffer */
	len = kore_task_channel_read(t, dst, sizeof(dst));
	if (len > sizeof(dst))
		return (KORE_RESULT_ERROR);

	kore_snprintf(rcode, (len+1),
		    NULL, "%s", dst);

	retval = save_confirmed_email(rcode);
	if (retval == KORE_RESULT_ERROR) {
		kore_task_set_result(t, KORE_RESULT_ERROR);

		return (KORE_RESULT_ERROR);
	}

	kore_task_set_result(t, KORE_RESULT_OK);

	return (KORE_RESULT_OK);
}
