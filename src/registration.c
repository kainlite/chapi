#include <kore/kore.h>
#include <kore/http.h>
#include <kore/tasks.h>
#include <chapi.h>
#include <misc/defaults.h>
#include <includes/registration.h>
#include <includes/user.h>
#include <includes/mail.h>
#include <misc/database.h>

struct rstate {
	struct		kore_task	task;
};	

int	serve_sign_up(struct http_request *req)
{
	int		p;
	char		*html_template_path;
	char		*msg;
	char		*email;
	u_int32_t	len;
	char		result[64];

	struct		rstate		*state;	

	html_template_path = "templates/sign_up.html";

	if (req->hdlr_extra == NULL) {
		p = http_populate_arguments(req);

		if (p == false) {
			msg = "400 Invalid information";
			http_response(req, 400, msg, strlen(msg));
			return (KORE_RESULT_OK);
		}
		
		if (!http_argument_get_string("email", &email, &len)) {
			msg = "400 Invalid information";
			http_response(req, 400, msg, strlen(msg));
			return (KORE_RESULT_OK);
		}

		state = kore_malloc(sizeof(*state));
		req->hdlr_extra = state;

		kore_task_create(&state->task, send_mail);
		kore_task_bind_request(&state->task, req);

		kore_task_run(&state->task);
		kore_task_channel_write(&state->task, email, len);
		kore_task_channel_write(&state->task, html_template_path, strlen(html_template_path));

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
		http_response(req, 500, NULL, 0);
		return (KORE_RESULT_OK);
	}

	len = kore_task_channel_read(&state->task, result, sizeof(result));
	if (len > sizeof(result)) {
		http_response(req, 500, NULL, 0);
	} else {
		msg = "200 Code sent";
		http_response(req, 200, msg, strlen(msg));
	}

	kore_task_destroy(&state->task);

	return (KORE_RESULT_OK);
}
