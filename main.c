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

void task(char exec[]) {
	execlp(exec, "", NULL);
	fprintf(stderr, "%s is not a valid executable path!\n", exec);
}

void write_maps(pid_t pid) {
	int t_pid = fork();
	if(t_pid == 0) {
		char buf[256];
		char content[40960];
		sprintf(buf, "sudo cat /proc/%d/maps > fmaps", child_pid);
		system(buf);
		exit(0);
	} else waitpid(t_pid, NULL, 0);
}

void sigint_handler(int sig) {
	fprintf(stderr, "\nOK: %d steps in total\n", steps);
	write_maps(child_pid);
	ptrace(PTRACE_DETACH, child_pid, NULL, NULL);
	exit(0);
}

int main(int argc, char *argv[]) {
	char exec[256];
	if(argc == 1) {
		fprintf(stderr, "Usage: sudo ./trace [pid] (recommended)\n");
		fprintf(stderr, "       sudo ./trace [exec abs-path]\n");
		exit(-1);
	} else {
		if(atoi(argv[1]) == 0) {
			child_pid = fork();
			strcpy(exec, argv[1]);
			if(child_pid != 0) {
				fprintf(stderr, "Tracing executable '%s' [%d]\n", exec, child_pid);
			}
		} else {
			child_pid = atoi(argv[1]);
			if(child_pid != 0)
				fprintf(stderr, "Tracing process [%d]\n", child_pid);
		}
	}
	int wstatus;
	struct user_regs_struct regs;
	struct iovec iov;
	iov.iov_base = (void*) &regs;
	iov.iov_len = sizeof(regs);
	FILE *f = fopen("pc_data", "w");

	if(child_pid != 0) /*tracer process*/ {
		signal(SIGINT, sigint_handler);
		if(ptrace(PTRACE_ATTACH, child_pid, NULL, NULL) < 0) {
			fprintf(stderr, "Cannot attach %d, %s\n", child_pid, strerror(errno));
			exit(-1);
		}
		fprintf(stderr, "Attach to process [%d]\n", child_pid);
		while(steps++ < TRACE_STEPS) {
			if(steps % 100000 == 0) { 
				fprintf(stderr, "%d finished...", steps);
				write_maps(child_pid);
				fprintf(stderr, "fmaps updated\n");
			}
			waitpid(child_pid, &wstatus, 0);
			if(WIFEXITED(wstatus)) {
				fprintf(stderr, "Child Exited.\n");
				fprintf(stderr, "OK: %d steps in total\n", steps);
				exit(-1);
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
		write_maps(child_pid);
	} else /*child (tracee) process*/ {
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		task(exec);
	}
	return 0;
}

