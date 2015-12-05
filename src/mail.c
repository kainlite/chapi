#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>
#include <kore/kore.h>
#include <includes/mail.h>
#include <misc/secure_random.h>
#include <stdbool.h>
#include <external/flate.h>

#define MULTI_PERFORM_HANG_TIMEOUT 60 * 1000

/* 
 * TODO: Remove magic numbers, and refactor.
 * Fix code (random number), and template replacement. 
 * Add real message-id generator
 * */
int handle_count;
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

static struct timeval tvnow(void)
{
	struct timeval now;

	/* time() returns the value of time in seconds since the epoch */ 
	now.tv_sec = (long)time(NULL);
	now.tv_usec = 0;

	return now;
}

static long tvdiff(struct timeval newer, struct timeval older)
{
	return (newer.tv_sec - older.tv_sec) * 1000 +
		(newer.tv_usec - older.tv_usec) / 1000;
}


int send_mail(char *to, char *text_template, char *html_template)
{
	unsigned	char		code[4];
	char *scode;

	CURL *curl;
	CURLM	*mcurl;
	int still_running = 1;
	struct timeval mp_start;
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
	if(!curl)
		return 1;

	mcurl = curl_multi_init();
	if(!mcurl)
		return 2;

	if(curl == NULL)
		return (KORE_RESULT_ERROR);

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

	curl_multi_add_handle(mcurl, curl);

	mp_start = tvnow();

	/* debug information */ 
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

	/* Send the message */ 
	/* res = curl_easy_setopt(curl, &handle_count); */
	curl_multi_perform(mcurl, &still_running);


	while(still_running) {
		struct timeval timeout;
		fd_set fdread;
		fd_set fdwrite;
		fd_set fdexcep;
		int maxfd = -1;
		int rc;
		CURLMcode mc; /* curl_multi_fdset() return code */ 

		long curl_timeo = -1;

		/* Initialise the file descriptors */ 
		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdexcep);

		/* Set a suitable timeout to play around with */ 
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		curl_multi_timeout(mcurl, &curl_timeo);
		if(curl_timeo >= 0) {
			timeout.tv_sec = curl_timeo / 1000;
			if(timeout.tv_sec > 1)
				timeout.tv_sec = 1;
			else
				timeout.tv_usec = (curl_timeo % 1000) * 1000;
		}

		/* get file descriptors from the transfers */ 
		mc = curl_multi_fdset(mcurl, &fdread, &fdwrite, &fdexcep, &maxfd);

		if(mc != CURLM_OK)
		{
			fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
			break;
		}

		/* On success the value of maxfd is guaranteed to be >= -1. We call
		   select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
		   no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
		   to sleep 100ms, which is the minimum suggested value in the
		   curl_multi_fdset() doc. */ 

		if(maxfd == -1) {
#ifdef _WIN32
			Sleep(100);
			rc = 0;
#else
			/* Portable sleep for platforms other than Windows. */ 
			struct timeval wait = { 0, 100 * 1000 }; /* 100ms */ 
			rc = select(0, NULL, NULL, NULL, &wait);
#endif
		}
		else {
			/* Note that on some platforms 'timeout' may be modified by select().
			   If you need access to the original value save a copy beforehand. */ 
			rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
		}

		if(tvdiff(tvnow(), mp_start) > MULTI_PERFORM_HANG_TIMEOUT) {
			fprintf(stderr,
					"ABORTING: Since it seems that we would have run forever.\n");
			break;
		}

		switch(rc) {
			case -1:  /* select error */ 
				break;
			case 0:   /* timeout */ 
			default:  /* action */ 
				curl_multi_perform(mcurl, &still_running);
				break;
		}
	}

	/* Free the list of recipients */ 
	curl_slist_free_all(recipients);

	/* Always cleanup */ 
	curl_multi_remove_handle(mcurl, curl);
	curl_multi_cleanup(mcurl);
	curl_easy_cleanup(curl);
	curl_global_cleanup();

	/* Check for errors */ 
	/* if(res != CURLE_OK) */
	/* 	kore_log(LOG_NOTICE, "Mail error: %s", curl_multi_strerror(res)); */

	/* /1* Free the list of recipients *1/ */ 
	/* curl_slist_free_all(recipients); */

	/* /1* Always cleanup *1/ */ 
	/* curl_multi_cleanup(curl); */

	/* We close the file handle when curl has finished doing it's thing*/
	flateFreeMem(f);

	/* return (int)res; */
	return 0;
}
