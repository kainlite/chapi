#include <kore/kore.h>
#include <kore/http.h>
#include <kore/tasks.h>
#include <chapi.h>
#include <misc/defaults.h>
#include <includes/registration.h>
#include <includes/contact.h>
#include <includes/message.h>
#include <includes/notification.h>
#include <includes/user.h>
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

int	init(void)
{
	return (KORE_RESULT_OK);
}
