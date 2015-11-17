#include <kore/kore.h>
#include <kore/http.h>
#include <misc/defaults.h>
#include <registrations/authorizations.h>
#include <contacts/contacts.h>
#include <messages/messages.h>
#include <notifications/notifications.h>
#include <secretmessages/secretmessages.h>
#include <users/users.h>
#include <curl/curl.h>
#include <sms/encode.h>
#include <sms/twilio.h>

int serve_index(struct http_request *);
int serve_apiw1(struct http_request *);

int serve_index(struct http_request *req)
{
	http_response(req, 501, "501 Not Implemented", 19);
	return (KORE_RESULT_OK);
}

int serve_apiw1(struct http_request *req)
{
	http_response(req, 501, "501 Not Implemented", 19);
	return (KORE_RESULT_OK);
}
