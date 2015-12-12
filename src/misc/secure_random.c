#include <unistd.h>
#include <syscall.h>
#include <errno.h>
#include <misc/secure_random.h>
#include <kore/kore.h>

int get_random(char *buff, int length)
{
	/* We may use ISAAC at some point, but for now this should be enough */
	int i;
	char str[] = "0123456789ABCDEF";

	srand((unsigned int) time(NULL) + getpid());

	for (i = 0; i < length; i++) {
		buff[i] = str[rand() % 16];
		srand(rand());
	}

	if (strlen(buff) > 0) {
		return KORE_RESULT_OK;
	} else {
		return KORE_RESULT_ERROR;
	}
}
