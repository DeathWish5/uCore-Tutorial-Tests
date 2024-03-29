#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

uint64 len = 0x1000;
uint64 prot = 3;

int count = 0;

int check(void *start, int shmem_id)
{
	int pid = getpid();
	int ppid = getppid();
	uint64 *s = start;
	int r = mmap(start, len, prot, MAP_SHARED, shmem_id);
	assert_eq(r, shmem_id);

	for (int try = 0; *s != ppid; try++) {
		sched_yield();
		if (try >= 3) {
			exit(-1);
		}
	}
	*s = pid;
	printf("share memory %d -> %d\n", ppid, pid);
	munmap(start, len);
	count++;
}

int main()
{
	uint64 *start = (void *)0x10000000;
	int xstate = 0;
	int shmem_id = mmap(start, len, prot, MAP_SHARED, -1);
	*(uint64 *)start = getpid();
	for (int i = 0; i < 10; i++) {
		int pid = fork();
		if (pid == 0) {
			check((void *)start + 0x1000 * (i + 1), shmem_id);
			if (count >= 5) {
				printf("ch6_shmem2 OK!");
				exit(0);
			}
		} else {
			wait(&xstate);
			return 0;
		}
	}
}