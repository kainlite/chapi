#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>
#include <kore/kore.h>
#include <kore/tasks.h>
#include <includes/mail.h>
#include <misc/secure_random.h>
#include <stdbool.h>
#include <external/flate.h>

/* 
 * TODO: Refactor. 
 * If we need to send more mails than just the sign up, we could extract the 
 * template portion out of here to be handled from the calling point.
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

int	send_mail(struct kore_task *t)
{
	char		code[CODE_LENGTH];
	u_int32_t	len;		
	
	char		message_id[MESSAGEID_LENGTH];

	char	to[EMAIL_LENGTH], html_template_path[TEMPLATE_NAME_LENGTH];

	CURL *curl;
	CURLcode res = CURLE_OK;
	struct curl_slist *recipients = NULL;

	struct upload_status upload_ctx;

	upload_ctx.read = false;

	generate_random(code, CODE_LENGTH);

	Flate *f = NULL;

	len = kore_task_channel_read(t, to, sizeof(to));
	if (len > sizeof(to))
		return (KORE_RESULT_ERROR);

	len = kore_task_channel_read(t, html_template_path, sizeof(html_template_path));
	if (len > sizeof(html_template_path))
		return (KORE_RESULT_ERROR);

	kore_snprintf(message_id, sizeof(message_id),
		    NULL, "%x.%x", getpid(),(unsigned int) time(NULL));

	flateSetFile(&f, html_template_path);

	flateSetVar(f, "email", to);
	flateSetVar(f, "code", code);
	flateSetVar(f, "message_id", message_id);
	flateSetVar(f, "from", getenv("MAIL_FROM"));
	flateSetVar(f, "domain", getenv("DOMAIN"));
	flateSetVar(f, "organization", getenv("ORGANIZATION"));

	payload_text = flatePage(f);

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

		res = curl_easy_perform(curl);

		if(res != CURLE_OK)
			kore_log(LOG_NOTICE, "Mail error: %s", curl_easy_strerror(res));

		curl_slist_free_all(recipients);

		curl_easy_cleanup(curl);

		flateFreeMem(f);
	}

	return (int)res;
}
