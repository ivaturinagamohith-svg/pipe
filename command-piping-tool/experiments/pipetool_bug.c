/*
 * Command Piping Tool
 * Connects the STDOUT of Command 1 to the STDIN of Command 2
 * using pipe(), fork(), dup2(), execvp() and waitpid().
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

/* Split a command line into an argv[] array (handles "quoted words"). */
static int parse_command(char *line, char *argv[])
{
    int argc = 0;
    char *p = line;
    while (*p && argc < MAX_ARGS - 1) {
        while (*p == ' ' || *p == '\t' || *p == '\n') p++;
        if (!*p) break;
        if (*p == '"' || *p == '\'') {
            char q = *p++;
            argv[argc++] = p;
            while (*p && *p != q) p++;
        } else {
            argv[argc++] = p;
            while (*p && *p != ' ' && *p != '\t' && *p != '\n') p++;
        }
        if (*p) *p++ = '\0';
    }
    argv[argc] = NULL;
    return argc;
}

static void read_line(const char *prompt, char *buf)
{
    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(buf, MAX_LINE, stdin)) {
        fprintf(stderr, "Input error\n");
        exit(EXIT_FAILURE);
    }
    buf[strcspn(buf, "\n")] = '\0';
}

/* Print how a child finished: normal exit code or terminating signal. */
static void report(const char *name, pid_t pid, int status)
{
    if (WIFEXITED(status))
        printf("[parent] %s (pid %d) exited with status %d\n",
               name, (int)pid, WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("[parent] %s (pid %d) terminated by signal %d (%s)\n",
               name, (int)pid, WTERMSIG(status), strsignal(WTERMSIG(status)));
}

int main(void)
{
    char line1[MAX_LINE], line2[MAX_LINE];
    char *cmd1[MAX_ARGS], *cmd2[MAX_ARGS];
    int pfd[2];
    pid_t c1, c2;
    int status1, status2;

    printf("===== COMMAND PIPING TOOL =====\n");
    read_line("Command 1: ", line1);
    read_line("Command 2: ", line2);

    if (parse_command(line1, cmd1) == 0 || parse_command(line2, cmd2) == 0) {
        fprintf(stderr, "Error: both commands must be non-empty\n");
        return EXIT_FAILURE;
    }

    printf("Creating anonymous pipe...\n");
    if (pipe(pfd) == -1) { perror("pipe"); return EXIT_FAILURE; }

    printf("Forking child 1...\n");
    printf("Forking child 2...\n");
    printf("Redirecting STDOUT -> pipe\n");
    printf("Redirecting STDIN  <- pipe\n");
    printf("Executing commands...\n\nFinal output:\n");
    fflush(stdout);

    c1 = fork();
    if (c1 == -1) { perror("fork"); return EXIT_FAILURE; }
    if (c1 == 0) {                         /* Child 1: producer */
        close(pfd[0]);                     /* unused read end   */
        if (dup2(pfd[1], STDOUT_FILENO) == -1) { perror("dup2"); _exit(1); }
        close(pfd[1]);
        execvp(cmd1[0], cmd1);
        perror("execvp (command 1)");
        _exit(127);
    }

    c2 = fork();
    if (c2 == -1) { perror("fork"); return EXIT_FAILURE; }
    if (c2 == 0) {                         /* Child 2: consumer */
        close(pfd[1]);                     /* unused write end  */
        if (dup2(pfd[0], STDIN_FILENO) == -1) { perror("dup2"); _exit(1); }
        close(pfd[0]);
        execvp(cmd2[0], cmd2);
        perror("execvp (command 2)");
        _exit(127);
    }

    /* BUG (for experiment): parent forgets to close pipe ends */
    waitpid(c1, &status1, 0);
    waitpid(c2, &status2, 0);

    printf("\n");
    report("child 1", c1, status1);
    report("child 2", c2, status2);
    return EXIT_SUCCESS;
}
