#include <kore/kore.h>
#include <kore/http.h>
#include <kore/tasks.h>

#include <includes/chapi.h>
#include <includes/registration.h>
#include <includes/user.h>
#include <includes/mail.h>

#include <misc/defaults.h>
#include <misc/database.h>

struct	rstate {
	struct		kore_task	task;
};

int	serve_sign_up(struct http_request *req)
{
	int		p;
	char		*html_template_path;
	char		*msg;
	char		*email;
	char		*password;
	char		*alias;
	char		*firstname;
	char		*lastname;
	u_int32_t	len;
	u_int32_t	email_len;
	u_int32_t	password_len;
	u_int32_t	alias_len;
	u_int32_t	firstname_len;
	u_int32_t	lastname_len;
	char		result[64];
	char		*expected_result = "success";

	struct		rstate		*state;

	html_template_path = "templates/sign_up.html";

	if (req->hdlr_extra == NULL) {
		p = http_populate_arguments(req);

		if (p == false) {
			msg = "400 Invalid request";
			http_response(req, 400, msg, strlen(msg));
			return (KORE_RESULT_OK);
		}

		if (!http_argument_get_string("email", &email, &email_len)) {
			msg = "400 Invalid request";
			http_response(req, 400, msg, strlen(msg));
			return (KORE_RESULT_OK);
		}

		if (!http_argument_get_string("password", &password, &password_len)) {
			msg = "400 Invalid request";
			http_response(req, 400, msg, strlen(msg));
			return (KORE_RESULT_OK);
		}

		if (!http_argument_get_string("alias", &alias, &alias_len)) {
			msg = "400 Invalid request";
			http_response(req, 400, msg, strlen(msg));
			return (KORE_RESULT_OK);
		}

		if (!http_argument_get_string("firstname", &firstname, &firstname_len)) {
			msg = "400 Invalid request";
			http_response(req, 400, msg, strlen(msg));
			return (KORE_RESULT_OK);
		}

		if (!http_argument_get_string("lastname", &lastname, &lastname_len)) {
			msg = "400 Invalid request";
			http_response(req, 400, msg, strlen(msg));
			return (KORE_RESULT_OK);
		}

		state = kore_malloc(sizeof(*state));
		req->hdlr_extra = state;

		kore_task_create(&state->task, create_user);
		kore_task_bind_request(&state->task, req);

		kore_task_run(&state->task);

		kore_task_channel_write(&state->task, html_template_path, strlen(html_template_path));

		kore_task_channel_write(&state->task, email, email_len);
		kore_task_channel_write(&state->task, password, password_len);
		kore_task_channel_write(&state->task, alias, alias_len);
		kore_task_channel_write(&state->task, firstname, firstname_len);
		kore_task_channel_write(&state->task, lastname, lastname_len);

		return (KORE_RESULT_RETRY);
	} else {
		state = req->hdlr_extra;
	}

	if (kore_task_state(&state->task) != KORE_TASK_STATE_FINISHED) {
		http_request_sleep(req);
		return (KORE_RESULT_RETRY);
	}

	if (kore_task_result(&state->task) != KORE_RESULT_OK) {
		kore_task_destroy(&state->task);

		msg = "500 Internal error";
		http_response(req, 500, msg, strlen(msg));

		return (KORE_RESULT_OK);
	}

	len = kore_task_channel_read(&state->task, result, sizeof(result));
	if (len > sizeof(result)) {
		msg = "500 Internal error";
		http_response(req, 500, msg, strlen(msg));
	} else {
		if (result == expected_result) {
			msg = "200 Code sent";
			http_response(req, 200, msg, strlen(msg));
		} else {
			msg = "500 Internal error";
			http_response(req, 500, msg, strlen(msg));
		}
	}

	kore_mem_free(state);
	kore_task_destroy(&state->task);

	return (KORE_RESULT_OK);
}
