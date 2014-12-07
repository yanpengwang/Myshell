/*
 * by wangyanpeng@gmail.com
 * 2014.12.07
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAXARGS 128
#define MAXLINE 8192

void unix_error(char *msg)
{
	fprintf(stderr, "%s \n", msg);
	exit(0);
}

pid_t Fork(void)
{
	pid_t pid;
	if ((pid = fork()) < 0)
		unix_error("Fork error");
	return pid;
}

extern char **environ; /* defined by libc */

void app_err(char * msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(0);
}

char * Fgets(char * ptr, int n, FILE * stream)
{
	char * rptr;
	if (((rptr = fgets(ptr, n, stream)) == NULL) && ferror(stream))
		app_err("Fgets error!");

	return rptr;
}

void eval(char *cmdline);
int parseline(char *buf, char **argv, int *arg_n);
int buildin_command(char **argv);

int main()
{
	char cmdline[MAXARGS];

	printf("===command you input should with its absolute path name.===\n");

	while (1) {
		printf("> ");
		Fgets(cmdline, MAXLINE, stdin);
		if (feof(stdin))
			exit(0);
			
		eval(cmdline);
	}
}

void eval(char *cmdline)
{
	char *argv[MAXARGS];
	char buf[MAXLINE];
	int bg;
	int arg_num;
	pid_t pid;

	strcpy(buf, cmdline);
	bg = parseline(buf, argv, &arg_num);
	/* printf("bg = %d, arg_nmu=%d\n", bg, arg_num); */

	
	if (argv[0] == NULL)
		return; /* Ignore empty line */	


	if (!builtin_command(argv)) {
		if ((pid = Fork()) == 0) { /* Child process run user job */
			if (execve(argv[0], argv, environ) < 0) {
				printf("%s: Command not found.\n", argv[0]);
				exit(0);
			}	
		}
	
		/* parent waits for foreground job to terminate */
		if (!bg) {
			int status;
			if (waitpid(pid, &status, 0) < 0)
				unix_error("waitfg: waitpid error");
		}
		else
			printf("%d %s", pid, cmdline);
	}
	return;
}

int parseline(char *buf, char **argv, int *arg_n)
{
	char *delim;
	int argc;
	int bg;
	
	buf[strlen(buf)-1] = ' '; /* Replace trailing '\n' with space */
	while (*buf && (*buf == ' '))
		buf++;
	
	/* Build the argv list */

	argc = 0;
	while ((delim = strchr(buf, ' '))) {
		argv[argc++] = buf;
		*delim = '\0';
		buf = delim + 1;
		while (*buf && (*buf == ' ')) /* Ignore spaces */
			buf++;
	}

	argv[argc] = NULL;

	if (argc == 0) /* Ignore blank line */
	{
		*arg_n = 0;
		return 0;
	}

	/* Should the job run in background? */
	if ((bg = (*argv[argc-1] == '&')) != 0)
		argv[--argc] = NULL;
	
	*arg_n = argc;

	return bg;

}

int builtin_command(char **argv)
{
	if (!strcmp(argv[0], "quit"))
		exit(0);
	if (!strcmp(argv[0], "&"))
		return 1;
	return 0;
}













