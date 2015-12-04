#include <kore/kore.h>
#include <kore/http.h>
#include <kore/tasks.h>
#include <chapi.h>
#include <misc/defaults.h>
#include <includes/registration.h>
#include <includes/user.h>
#include <includes/mail.h>
#include <misc/database.h>

int	serve_sign_up(struct http_request *req)
{
	int		p, r;
	char		*msg;
	char		*email;
	u_int32_t	len;
	struct		kore_buf	*buf;

	p = http_populate_arguments(req);

	if (p == 0) {
		msg = "400 Invalid information";
		http_response(req, 400, msg, strlen(msg));
		return (KORE_RESULT_OK);
	}

	buf = kore_buf_create(128);
	
	if (http_argument_get_string("email", &email, &len))
		kore_buf_appendf(buf, "email as a string: '%s' (%d)\n", email, len);

	r = send_mail("kainlite@gmail.com", "templates/sign_up.text", "templates/sign_up.html");

	if (r == 1) {
		msg = "400 Could not process email information";
		http_response(req, 400, msg, strlen(msg));
		return (KORE_RESULT_OK);
	}

	msg = "200 Code sent";

	http_response(req, 200, msg, strlen(msg));
	return (KORE_RESULT_OK);
}
