#include "runcmd.h"

int status = 0;
int back_pid = 0;
struct cmd* parsed_pipe;


// runs the command in 'cmd'
int run_cmd(char* cmd) {
    int result;
    //chequeo la existencia de zombies.
    do {
        result = waitpid(-1, &status, WNOHANG);
        if (result != 0 && result != -1)
            fprintf(stderr, "Exited with status: %d\n", result);
    } while (result != 0 && result != -1);

	pid_t p;
	struct cmd *parsed;

	if (cmd[0] == END_STRING)
		return 0;

    int cdValue;
    if ((cdValue = cd(cmd)) == 1)
        return 0;
    else if (cdValue == -1){
        return -1;
    }

	if (exit_shell(cmd))
		return EXIT_SHELL;

	int pwdValue;
	if ((pwdValue = pwd(cmd)) == 1)
		return 0;
	else if (pwdValue == -1){
	    fprintf(stderr, "Error al obtener el directorio actual");
	    return pwdValue;
	}

	parsed = parse_line(cmd);

	if ((p = fork()) == 0) {
		exec_cmd(parsed);
	}

	parsed->pid = p;

	if (parsed->type == BACK){
        print_back_info(parsed);
	} else {
        waitpid(p, &status, 0);
        print_status_info(parsed);
    }

	free_command(parsed);
	return 0;
}

