#include <misc/defaults.h>

typedef struct contact { 
  unsigned long int user_id;
} contact;

typedef struct contacts { 
  contact contact[CONTACTS_LENGTH]; 
} contacts;

contacts get_contacts(void);
contacts import_contacts(void);
bool block(void);
bool unblock(void);
