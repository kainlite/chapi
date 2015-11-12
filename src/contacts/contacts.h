#include <misc/defaults.h>

typedef struct contact { 
  unsigned long int user_id;
} contact;

typedef struct contacts contact[CONTACTS_LENGTH]; 

contacts get_contacts();
contacts import_contacts();
bool block();
bool unblock();
