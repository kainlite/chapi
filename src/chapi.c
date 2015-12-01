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
#include <misc/secure_random.h>

/*
 * TODO: http response text could be json, or something else.
 * TODO: Remove all magic numbers :/
 * TODO: Move random stuff to mailer, when the mailer exists :/
 * */

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
	kore_log(LOG_NOTICE, "Initializing db, and stuff...");
	return (KORE_RESULT_OK);
}

int	serve_sign_up(struct http_request *req)
{
	int		p;
	char		*msg;
	char		*email;
	unsigned	char		random[4];
	u_int32_t	len;
	struct		kore_buf	*buf;
	
	generate_random(random, 4);

	p = http_populate_arguments(req);

	if (p == 0) {
		msg = "400 Invalid information";
		http_response(req, 400, msg, strlen(msg));
		return (KORE_RESULT_OK);
	}

	buf = kore_buf_create(128);
	
	if (http_argument_get_string("email", &email, &len))
		kore_buf_appendf(buf, "email as a string: '%s' (%d)\n", email, len);

	kore_log(LOG_NOTICE, "Email: %s", email);
	kore_log(LOG_NOTICE, "Code: %x%x%x%x", random[0], random[1], random[2], random[3]);

	msg = "200 Code sent";

	http_response(req, 200, msg, strlen(msg));
	return (KORE_RESULT_OK);
}
