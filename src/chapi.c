#include <kore/kore.h>
#include <kore/http.h>
#include <misc/defaults.h>
#include <registrations/authorizations.h>
#include <contacts/contacts.h>
#include <messages/messages.h>
#include <notifications/notifications.h>
#include <secretmessages/secretmessages.h>
#include <users/users.h>

int		page(struct http_request *);

int
page(struct http_request *req)
{
	http_response(req, 200, NULL, 0);
	return (KORE_RESULT_OK);
}
