#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>
#include <kore/kore.h>
#include <includes/mail.h>
#include <misc/secure_random.h>
#include <stdbool.h>
#include <external/flate.h>

/* 
 * TODO: Remove magic numbers, and refactor if possible.
 * Fix code (random number), and template replacement. 
 * */
static char *payload_text;

struct upload_status {
	bool read;
};

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *stream)
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

int send_mail(char *to, char *text_template, char *html_template)
{
	unsigned	char		code[4];
	char *scode;

	CURL *curl;
	CURLcode res = CURLE_OK;
	struct curl_slist *recipients = NULL;

	struct upload_status upload_ctx;

	upload_ctx.read = false;

	generate_random(code, 4);
	scode = (char*) code;


	Flate *f = NULL;

	flateSetFile(&f, html_template);

	flateSetVar(f, "email", to);
	flateSetVar(f, "code", scode);
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

		/* Set mail from*/
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, getenv("MAIL_FROM"));

		/* Add recipients */ 
		recipients = curl_slist_append(recipients, to);
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

		/* curl_easy_setopt(curl, CURLOPT_READDATA, buf); */
		/* curl_easy_setopt(curl, CURLOPT_READFUNCTION, (const char *)payload_source); */
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		/* debug information */ 
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

		/* Send the message */ 
		res = curl_easy_perform(curl);

		/* Check for errors */ 
		if(res != CURLE_OK)
			kore_log(LOG_NOTICE, "Mail error: %s", curl_easy_strerror(res));

		/* Free the list of recipients */ 
		curl_slist_free_all(recipients);

		/* Always cleanup */ 
		curl_easy_cleanup(curl);

		/* We close the file handle when curl has finished doing it's thing*/
		flateFreeMem(f);
	}

	return (int)res;
}
