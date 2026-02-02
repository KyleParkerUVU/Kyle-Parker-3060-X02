/*
 * Kyle Parker
 * CS 3060-X02
 *
 * Project 2: fork() and exec()
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static int exit_code_from_status(int status) {
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        /* common convention: 128 + signal number */
        return 128 + WTERMSIG(status);
    }
    return 1;
}

int main(int argc, char *argv[]) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        /* Child */
        if (argc == 1) {
            printf("CHILD started. No arguments provided. Terminating child.\n");
            fflush(stdout);
            return 0;
        }

        if (argc == 2) {
            printf("CHILD started. One argument provided. Calling execlp(), never to return (sniff)\n");
            fflush(stdout);

            /* argv[1] is the program to run */
            execlp(argv[1], argv[1], (char *)NULL);

            /* Only reaches here if execlp fails */
            perror("execlp");
            return 1;
        }

        /* argc > 2 */
        printf("CHILD started. More than one argument provided. Calling execvp(), never to return (sniff)\n");
        fflush(stdout);

        /*
         * execvp wants: file, argv[]
         * We pass argv[1] as the file, and &argv[1] as the argument vector
         * so the new program sees argv[0] = original argv[1].
         */
        execvp(argv[1], &argv[1]);

        /* Only reaches here if execvp fails */
        perror("execvp");
        return 1;
    } else {
        /* Parent */
        int status = 0;

        printf("PARENT started, now waiting for process ID#%d\n", (int)pid);
        fflush(stdout);

        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            return 1;
        }

        int code = exit_code_from_status(status);
        printf("PARENT resumed. Child exit code of %d. Now terminating parent\n", code);
        fflush(stdout);

        return 0;
    }
}
