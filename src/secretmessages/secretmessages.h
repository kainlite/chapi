#include <misc/defaults.h>

typedef struct secretmessages secretmessage[MESSAGES_LENGTH]; 

typedef struct secretmessage {
  unsigned long int user_id;
  char message[MESSAGE_LENGTH];
  unsigned long int random_id;
} secretmessage;

typedef struct sent_secretmessage {
  unsigned long int id;
  unsigned long int chat_id;
  unsigned long int seq;
  time_t date;
} sent_secretmessage; 

sent_secretmessage send_secretmessage();
messages get_secretmessages();
