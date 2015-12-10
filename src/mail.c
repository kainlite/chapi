#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <kore/kore.h>
#include <kore/tasks.h>

#include <bson.h>
#include <mongoc.h>

#include <curl/curl.h>

#include <includes/mail.h>

#include <misc/secure_random.h>
#include <misc/database.h>
#include <external/flate.h>

/*
 * TODO: Refactor (this should be in caps rofl).
 * If we need to send more mails than just the sign up, we could extract the
 * template portion out of here to be handled from the calling point.
 * Or we can use this scheduled-task pattern which seems a better aproach
 * overall.
 */

static	char	*payload_text;

struct	upload_status {
	bool read;
};

static	size_t	payload_source(void *ptr, size_t size, size_t nmemb, void *stream)
{
	struct upload_status *upload_ctx = (struct upload_status *)stream;
	const char *data;

	if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
		return 0;
	}

	data = payload_text;

	if(data && !(upload_ctx->read)) {
		size_t len = strlen(data);
		memcpy(ptr, data, len);

		upload_ctx->read = true;

		return len;
	}

	return 0;
}

int	create_user(struct kore_task *t)
{
	u_int32_t	len;

	char		code[CODE_LENGTH];
	char		message_id[MESSAGEID_LENGTH];
	char		to[EMAIL_LENGTH], html_template_path[TEMPLATE_NAME_LENGTH];

	CURL		*curl;

	CURLcode res = CURLE_OK;

	struct curl_slist *recipients = NULL;
	struct upload_status upload_ctx;

	upload_ctx.read = false;

	/* Read channel/buffer */
	len = kore_task_channel_read(t, to, sizeof(to));
	if (len > sizeof(to))
		return (KORE_RESULT_ERROR);

	len = kore_task_channel_read(t, html_template_path, sizeof(html_template_path));
	if (len > sizeof(html_template_path))
		return (KORE_RESULT_ERROR);

	kore_snprintf(message_id, sizeof(message_id),
		    NULL, "%x.%x", getpid(),(unsigned int) time(NULL));

	/* Mongo */
	mongoc_collection_t *collection;
	mongoc_cursor_t *cursor;

	/* Find vars */
	const bson_t *doc;
	bson_t *query;
	char *str;

	/* Insert vars*/
	bson_error_t error;
	bson_oid_t oid;
	bson_t *bdoc;

	/* Find user and if it does not exist create it*/
	collection = mongoc_client_get_collection(client, getenv("ENVIRONMENT"), "users");
	query = bson_new();
	BSON_APPEND_UTF8 (query, "email", to);
	cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

	/* If the user exists we don't care about doing anything else */
	if (mongoc_cursor_next(cursor, &doc)) {
		str = bson_as_json (doc, NULL);
		printf("%s\n", str);
		bson_free (str);
		return NULL;
	} else {
		bdoc = bson_new();
		bson_oid_init(&oid, NULL);
		BSON_APPEND_OID(bdoc, "_id", &oid);
		BSON_APPEND_UTF8(bdoc, "email", to);

		if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, bdoc, NULL, &error)) {
			fprintf (stderr, "%s\n", error.message);
		}

		bson_destroy (bdoc);
	}

	/* Cleanup release all used memory */
	bson_destroy(bdoc);
	bson_destroy(query);
	mongoc_cursor_destroy(cursor);

	generate_random(code, CODE_LENGTH);

	/* Process template */
	Flate *f = NULL;
	flateSetFile(&f, html_template_path);

	flateSetVar(f, "email", to);
	flateSetVar(f, "code", code);
	flateSetVar(f, "message_id", message_id);
	flateSetVar(f, "from", getenv("MAIL_FROM"));
	flateSetVar(f, "domain", getenv("DOMAIN"));
	flateSetVar(f, "organization", getenv("ORGANIZATION"));

	payload_text = flatePage(f);

	/* Proceed sending the email */
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_USERNAME, getenv("MAIL_USER"));
		curl_easy_setopt(curl, CURLOPT_PASSWORD, getenv("MAIL_PASSWORD"));

		curl_easy_setopt(curl, CURLOPT_URL, getenv("MAIL_HOST"));

		/*
		 * Ssl stuff
		 *   curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		 *   curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		 *   curl_easy_setopt(curl, CURLOPT_CAINFO, "/path/to/certificate.pem");
		*/

		curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);

		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, getenv("MAIL_FROM"));

		recipients = curl_slist_append(recipients, to);
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

		curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, DEBUG_CURL);

		/* res = curl_easy_perform(curl); */

		if(res != CURLE_OK)
			kore_log(LOG_NOTICE, "Mail error: %s", curl_easy_strerror(res));

		curl_slist_free_all(recipients);

		curl_easy_cleanup(curl);

		flateFreeMem(f);
	}

	return (int)res;
}
