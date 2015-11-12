#include <misc/defaults.h>

typedef struct account {
  char token[TOKEN_LENGTH];
} account; 

bool register_device();
bool unregister_device();
