#include <misc/defaults.h>

typedef struct message {
  unsigned long int user_id;
  char message[MESSAGE_LENGTH];
  unsigned long int random_id;
} message;

typedef struct messages { 
  message message[MESSAGES_LENGTH];
} messages; 

typedef struct sent_message {
  unsigned long int id;
  unsigned long int chat_id;
  unsigned long int seq;
  time_t date;
} sent_message; 

bool set_typing(void);
sent_message send_message(void);
messages get_history(void);
messages delete_messages(void);
messages get_messages(void);
