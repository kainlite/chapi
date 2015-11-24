#include <kore/kore.h>
#include <kore/http.h>
#include <chapi.h>
#include <misc/defaults.h>
#include <registrations/authorizations.h>
#include <contacts/contacts.h>
#include <messages/messages.h>
#include <notifications/notifications.h>
#include <secretmessages/secretmessages.h>
#include <users/users.h>
#include <bson.h>
#include <bcon.h>
#include <mongoc.h>
#include <misc/database.h>


int	serve_index(struct http_request *req)
{
	init(1);
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
	mongo_connect("chapi_development");

	return (KORE_RESULT_OK);
}

