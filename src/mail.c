#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include <kore/kore.h>
#include <kore/tasks.h>

#include <bson.h>
#include <mongoc.h>

#include "libscrypt.h"
#include "external/crypto_scrypt-hexconvert.h"

#include <curl/curl.h>

#include <includes/mail.h>

#include <misc/secure_random.h>
#include <misc/database.h>

#include <external/flate.h>

/*
 * TODO: Refactor (this should be in caps rofl).
 * If we need to send more mails than just the sign up, we could extract the
 * template portion out of here to be handled from the calling point.
 *
 * We can use this scheduled-task pattern which seems a good aproach
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

int parse_template(User user, char *html_template_path) {
	char		message_id[MESSAGEID_LENGTH];
	Flate		*f = NULL;

	kore_snprintf(message_id, sizeof(message_id),
		    NULL, "%x.%x", getpid(),(unsigned int) time(NULL));

	/* Process template */
	flateSetFile(&f, html_template_path);

	flateSetVar(f, "email", user.email);
	flateSetVar(f, "code", user.code);
	flateSetVar(f, "message_id", message_id);
	flateSetVar(f, "from", getenv("MAIL_FROM"));
	flateSetVar(f, "domain", getenv("DOMAIN"));
	flateSetVar(f, "organization", getenv("ORGANIZATION"));

	payload_text = flatePage(f);

	flateFreeMem(f);

	if (strlen(payload_text) > 0) {
		return KORE_RESULT_OK;
	} else {
		return KORE_RESULT_ERROR;
	}
}

int create_user_record(User user) {
	/* Scrypt */
	int		retval;
	char		outbuf[132];

	/* Mongo */
	mongoc_collection_t *collection;
	mongoc_cursor_t *cursor;

	/* Find vars */
	const bson_t *doc;
	bson_t *query;

	/* Insert vars*/
	bson_error_t error;
	bson_oid_t oid;
	bson_t *bdoc;

	/* Find user and if it does not exist create it*/
	collection = mongoc_client_get_collection(client, getenv("ENVIRONMENT"), "users");
	query = bson_new();
	BSON_APPEND_UTF8 (query, "email", user.email);
	cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

	/* If the user exists we don't do anything else */
	if (mongoc_cursor_next(cursor, &doc))
	{
		return (KORE_RESULT_ERROR);
	} else {
		retval = libscrypt_hash(
			 outbuf,
			 user.password,
			 SCRYPT_N,
			 SCRYPT_r,
			 SCRYPT_p
		);

		if(retval < 0)
		{
			kore_log(LOG_NOTICE, "Couldn't generate hash %s", outbuf);

			return KORE_RESULT_ERROR;
		}

		bdoc = bson_new();
		bson_oid_init(&oid, NULL);
		BSON_APPEND_OID(bdoc, "_id", &oid);
		BSON_APPEND_UTF8(bdoc, "email", user.email);
		BSON_APPEND_UTF8(bdoc, "code", user.code);
		BSON_APPEND_UTF8(bdoc, "password", outbuf);
		BSON_APPEND_UTF8(bdoc, "alias", user.alias);
		BSON_APPEND_UTF8(bdoc, "firstname", user.firstname);
		BSON_APPEND_UTF8(bdoc, "lastname", user.lastname);

		if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, bdoc, NULL, &error)) {
			fprintf (stderr, "%s\n", error.message);
		}
	}

	/* Cleanup release all used memory */
	bson_destroy(bdoc);
	bson_destroy(query);
	mongoc_cursor_destroy(cursor);

	return KORE_RESULT_OK;
}

int send_mail(User user) {
	/* Proceed sending the email */
	CURL		*curl;
	CURLcode	res = CURLE_OK;

	struct curl_slist *recipients = NULL;
	struct upload_status upload_ctx;
	upload_ctx.read = false;

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

		recipients = curl_slist_append(recipients, user.email);
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

		curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, DEBUG_CURL);

		/* This sends the mail out */
		res = curl_easy_perform(curl);

		if(res != CURLE_OK) {
			kore_log(LOG_NOTICE, "Mail error: %s", curl_easy_strerror(res));

			return (KORE_RESULT_ERROR);
		}

		curl_slist_free_all(recipients);

		curl_easy_cleanup(curl);
	}

	return KORE_RESULT_OK;
}

int	create_user(struct kore_task *t)
{
	/* App vars */
	int		retval;
	u_int32_t	len;
	User		user;
	char		rcode[CODE_LENGTH];

	char		html_template_path[TEMPLATE_NAME_LENGTH];
	char		dst[BUFFER_LENGTH];

	/* Read channel/buffer */
	len = kore_task_channel_read(t, dst, sizeof(dst));
	if (len > sizeof(dst))
		return (KORE_RESULT_ERROR);

	kore_snprintf(html_template_path, (len+1),
		    NULL, "%s", dst);

	len = kore_task_channel_read(t, dst, sizeof(dst));
	if (len > sizeof(dst))
		return (KORE_RESULT_ERROR);

	kore_snprintf(user.email, (len+1),
		    NULL, "%s", dst);

	len = kore_task_channel_read(t, dst, sizeof(dst));
	if (len > sizeof(dst))
		return (KORE_RESULT_ERROR);

	kore_snprintf(user.password, (len+1),
		    NULL, "%s", dst);

	len = kore_task_channel_read(t, dst, sizeof(dst));
	if (len > sizeof(dst))
		return (KORE_RESULT_ERROR);

	kore_snprintf(user.alias, (len+1),
		    NULL, "%s", dst);

	len = kore_task_channel_read(t, dst, sizeof(dst));
	if (len > sizeof(dst))
		return (KORE_RESULT_ERROR);

	kore_snprintf(user.firstname, (len+1),
		    NULL, "%s", dst);

	len = kore_task_channel_read(t, dst, sizeof(dst));
	if (len > sizeof(dst))
		return (KORE_RESULT_ERROR);

	kore_snprintf(user.lastname, (len+1),
		    NULL, "%s", dst);

	retval = get_random(rcode, CODE_LENGTH);
	if (retval == KORE_RESULT_ERROR) {
		kore_task_set_result(t, KORE_RESULT_ERROR);

		return (KORE_RESULT_ERROR);
	}

	kore_snprintf(user.code, (CODE_LENGTH+1),
		    NULL, "%s", rcode);

	retval = parse_template(user, html_template_path);
	if (retval == KORE_RESULT_ERROR) {
		kore_task_set_result(t, KORE_RESULT_ERROR);

		return (KORE_RESULT_ERROR);
	}

	retval = create_user_record(user);
	if (retval == KORE_RESULT_ERROR) {
		kore_task_set_result(t, KORE_RESULT_ERROR);

		return (KORE_RESULT_ERROR);
	}

	retval = send_mail(user);
	if (retval == KORE_RESULT_ERROR) {
		kore_task_set_result(t, KORE_RESULT_ERROR);

		return (KORE_RESULT_ERROR);
	}

	kore_task_set_result(t, KORE_RESULT_OK);

	return (KORE_RESULT_OK);
}
