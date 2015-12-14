#include <kore/kore.h>
#include <kore/http.h>
#include <kore/tasks.h>

#include <includes/chapi.h>
#include <includes/mail.h>

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
	char		*password_confirmation;
	char		*alias;
	char		*firstname;
	char		*lastname;
	u_int32_t	email_len;
	u_int32_t	password_len;
	u_int32_t	password_confirmation_len;
	u_int32_t	alias_len;
	u_int32_t	firstname_len;
	u_int32_t	lastname_len;

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

		if (!http_argument_get_string("password", &password_confirmation, &password_confirmation_len)) {
			msg = "400 Invalid request";
			http_response(req, 400, msg, strlen(msg));
			return (KORE_RESULT_OK);
		}

		if(password_len != password_confirmation_len && password != password_confirmation) {
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

		msg = "400 Invalid input";
		http_response(req, 400, msg, strlen(msg));

		return (KORE_RESULT_OK);
	} else {
		msg = "200 Code sent";
		http_response(req, 200, msg, strlen(msg));

		return (KORE_RESULT_OK);
	}

	kore_mem_free(state);
	kore_task_destroy(&state->task);

	return (KORE_RESULT_OK);
}

int	serve_confirm_email(struct http_request *req)
{
	int		p;
	char		*msg;
	char		*code;
	u_int32_t	code_len;

	struct		rstate		*state;

	if (req->hdlr_extra == NULL) {
		p = http_populate_arguments(req);

		if (p == false) {
			msg = "400 Invalid request";
			http_response(req, 400, msg, strlen(msg));
			return (KORE_RESULT_OK);
		}

		if (!http_argument_get_string("code", &code, &code_len)) {
			msg = "400 Invalid request";
			http_response(req, 400, msg, strlen(msg));
			return (KORE_RESULT_OK);
		}

		state = kore_malloc(sizeof(*state));
		req->hdlr_extra = state;

		kore_task_create(&state->task, confirm_email);
		kore_task_bind_request(&state->task, req);

		kore_task_run(&state->task);

		kore_task_channel_write(&state->task, code, code_len);

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

		msg = "400 Invalid input";
		http_response(req, 400, msg, strlen(msg));

		return (KORE_RESULT_OK);
	} else {
		msg = "200 Valid code";
		http_response(req, 200, msg, strlen(msg));

		return (KORE_RESULT_OK);
	}

	kore_mem_free(state);
	kore_task_destroy(&state->task);

	return (KORE_RESULT_OK);
}

int	serve_sign_in(struct http_request *req)
{
	int		p;
	char		*msg;
	char		*extra;
	char		*email;
	char		*password;
	char		session_id[DHASH_LENGTH];
	char		dst[DHASH_LENGTH];

	u_int32_t	len;
	u_int32_t	email_len;
	u_int32_t	password_len;

	struct		rstate		*state;

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

		state = kore_malloc(sizeof(*state));
		req->hdlr_extra = state;

		kore_task_create(&state->task, sign_in);
		kore_task_bind_request(&state->task, req);

		kore_task_run(&state->task);

		kore_task_channel_write(&state->task, email, email_len);
		kore_task_channel_write(&state->task, password, password_len);

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

		msg = "403 You shall not pass";
		http_response(req, 400, msg, strlen(msg));

		return (KORE_RESULT_OK);
	} else {
                len = kore_task_channel_read(&state->task, dst, sizeof(dst));
                if (len > sizeof(dst))
                        return (KORE_RESULT_ERROR);

		extra = "session_id=";
                kore_snprintf(session_id, (len + strlen(extra) + 1),
                                              NULL, "%s%s", extra, dst);

		http_response_header(req, "set-cookie", session_id);

		msg = "200 Valid user";
		http_response(req, 200, msg, strlen(msg));

		return (KORE_RESULT_OK);
	}

	kore_mem_free(state);
	kore_task_destroy(&state->task);

	return (KORE_RESULT_OK);
}
