#include "includes/user.h"

#define DEBUG_CURL 0L

#define	BUFFER_LENGTH 32
#define TEMPLATE_NAME_LENGTH 32
#define MESSAGEID_LENGTH 32

int	create_user(struct kore_task *);

int send_mail(User user);
int parse_template(User user, char *html_template_path);
int create_user_record (User user);
