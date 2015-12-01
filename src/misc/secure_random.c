#include <unistd.h>
#include <syscall.h>
#include <errno.h>
#include <linux/random.h>
#include <misc/secure_random.h>
#include <kore/kore.h>

void generate_random(unsigned char *buff, int length)
{
	/*
	 * Here we invoke the getrandom system call. The fourth argument is the flag that will
	 * be passed to getrandom. Available flags are 0, GRND_RANDOM, GRND_NONBLOCK.
	 * GRND_RANDOM stands for /dev/random instead of urandom
	 * GRND_NONBLOCK well it does not block, and returns EAGAIN
	 * 
	 * Why are we using this?, 'cause we can change the implementation later,
	 * but we already have the interface.
	 */
	int n = syscall(SYS_getrandom, buff, length, NULL);

	if(n == -1)
	{

		kore_log(LOG_NOTICE, "Reading random bytes failed: %s", errno);
	}
	else
	{
		kore_log(LOG_NOTICE, "Everything went fine key code: %x%x%x%x", buff[0], buff[1], buff[2], buff[3]);
	}

	return NULL;
}
