#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <elf.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/ptrace.h>

#define TRACE_STEPS 10000000

pid_t child_pid;
int steps = 0;

void task() {
	printf("children [%d]\n", getpid());
	execl("/home/parallels/workspace/perf/build/main", "main", NULL);
}

void writemaps(pid_t pid) {
	if(fork() == 0) {
		char buf[256];
		char content[40960];
		sprintf(buf, "sudo cat /proc/%d/maps > maps", child_pid);
		system(buf);
		exit(0);
	} else wait(NULL);
}

void sigint_handler(int sig) {
	printf("\nOK: %d steps finished\n", steps);
	writemaps(child_pid);
	ptrace(PTRACE_DETACH, child_pid, NULL, NULL);
	exit(0);
}

int main(int argc, char *argv[]) {
	if(argc == 1)
		child_pid = fork();
	else child_pid = atoi(argv[1]);

	int wstatus;
	struct user_regs_struct regs;
	struct iovec iov;
	iov.iov_base = (void*) &regs;
	iov.iov_len = sizeof(regs);
	FILE *f = fopen("data", "w");

	if(child_pid != 0) /*tracer process*/ {
		signal(SIGINT, sigint_handler);
		ptrace(PTRACE_ATTACH, child_pid, NULL, NULL);
		while(steps++ < TRACE_STEPS) {
			if(steps % 100000 == 0) 
				printf("%d finished...\n", steps);
			waitpid(child_pid, &wstatus, 0);
			if(WIFEXITED(wstatus)) {
				fprintf(stderr, "child exited without writing maps\n");
				fprintf(stderr, "you are NOT supposed to let child exited before TRACE_STEPS\n");
				break;
			}
			//printf("Parent: (%d) received %s\n", child_pid, strsignal(WSTOPSIG(wstatus)));

#ifndef __x86_64__
			ptrace(PTRACE_GETREGSET, child_pid, NT_PRSTATUS, &iov);
			fprintf(f, "%llx ", regs.pc);
#else
			ptrace(PTRACE_GETREGS, child_pid, NULL, &regs);
			fprintf(f, "%llx ", regs.rip);
#endif

			ptrace(PTRACE_SINGLESTEP, child_pid, NULL, NULL);
		}
		writemaps(child_pid);
	} 
	else /*child (tracee) process*/ {
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		task();
	}
	return 0;
}
