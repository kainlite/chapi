#include <misc/defaults.h>

typedef struct user {
  char username[USERNAME_LENGTH];
} user; 

user update_username(void);
bool check_username(void);
