#include <misc/defaults.h>

typedef struct user {
  char username[USERNAME_LENGTH];
} user; 

user update_username();
bool check_username();
