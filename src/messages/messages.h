#include <misc/defaults.h>

typedef struct messages message[MESSAGES_LENGTH]; 

typedef struct message {
  unsigned long int user_id;
  char message[MESSAGE_LENGTH];
  unsigned long int random_id;
} message;

typedef struct sent_message {
  unsigned long int id;
  unsigned long int chat_id;
  unsigned long int seq;
  time_t date;
} sent_message; 

bool set_typing();
sent_message send_message();
messages get_history();
messages delete_messages();
messages get_messages();
