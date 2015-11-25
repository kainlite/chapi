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
#include <misc/database.h>


int	serve_index(struct http_request *req)
{
	init();
	http_response(req, 501, "501 Not Implemented", 19);
	return (KORE_RESULT_OK);
}

int	serve_apiw1(struct http_request *req)
{
	http_response(req, 501, "501 Not Implemented", 19);
	return (KORE_RESULT_OK);
}

int	init(void)
{
	return (KORE_RESULT_OK);
}

