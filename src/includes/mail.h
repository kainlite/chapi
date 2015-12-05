#define DEBUG_CURL 0L

#define CODE_LENGTH 8
#define TEMPLATE_NAME_LENGTH 64
#define EMAIL_LENGTH 64
#define MESSAGEID_LENGTH 32

int send_mail(struct kore_task *);
