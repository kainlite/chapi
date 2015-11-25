#include <misc/defaults.h>

typedef struct authorization {
  unsigned long int user_id;
  char first_name[NAME_LENGTH];
  char last_name[LNAME_LENGTH];
  char phone[PHONE_LENGTH];
  bool active;
} authorization; 

typedef struct authorized {
  unsigned long int expires;
  authorization authorization;
} authorized;

authorization sign_up(void);
void send_code(void);
authorized sing_in(void);
bool log_out(void);
