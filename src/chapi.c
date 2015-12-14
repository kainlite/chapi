#include <kore/kore.h>
#include <kore/http.h>
#include <kore/tasks.h>

#include <bson.h>
#include <mongoc.h>

#include <includes/chapi.h>
#include <includes/mail.h>

#include <misc/database.h>

/*
 * TODO: http response text could be json, or something else.
 */

int	serve_index(struct http_request *req)
{
	http_response(req, 501, "501 Not Implemented", 19);
	return (KORE_RESULT_OK);
}

int	serve_apiw1(struct http_request *req)
{
	http_response(req, 501, "501 Not Implemented", 19);
	return (KORE_RESULT_OK);
}

int	init(int state)
{
	/* TODO: Improve db connection handling. */
	switch (state) {
	case KORE_MODULE_LOAD:
		kore_log(LOG_NOTICE, "Initializing db");

		mongoc_init();

		client = mongoc_client_new("mongodb://localhost:27017/");

		redisClient = redisConnect("127.0.0.1", 6379);
		if (redisClient != NULL && redisClient->err) {
			printf("Error: %s\n", redisClient->errstr);
		}

		break;
	case KORE_MODULE_UNLOAD:
		kore_log(LOG_NOTICE, "Freeing resources");

		mongoc_client_destroy(client);

		mongoc_cleanup ();

		redisFree(redisClient);

		break;
	default:
		kore_log(LOG_NOTICE, "state %d unknown!", state);
		break;
	}
	return (KORE_RESULT_OK);
}

int	validate_session(struct http_request *req, char *data)
{
	redisReply	*reply;

	reply = redisCommand(redisClient, "GET %s", data);

	if (reply->str) {
		return (KORE_RESULT_OK);
	} else {
		return (KORE_RESULT_ERROR);
	}

	freeReplyObject(reply);
}
