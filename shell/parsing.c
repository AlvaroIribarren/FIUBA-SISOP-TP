#include "parsing.h"

// parses an argument of the command stream input
static char* get_token(char* buf, int idx) {

	char* tok;
	int i;
	
	tok = (char*)calloc(ARGSIZE, sizeof(char));
	i = 0;

	while (buf[idx] != SPACE && buf[idx] != END_STRING) {
		tok[i] = buf[idx];
		i++; idx++;
	}

	return tok;
}

bool is_cd_command(const char* cmd){
    char aux[3];
    aux[0] = cmd[0];
    aux[1] = cmd[1];
    aux[2] = '\0';
    return strings_are_equal(aux, "cd");
}

// parses and changes stdin/out/err if needed
static bool parse_redir_flow(struct execcmd* c, char* arg) {

	int inIdx, outIdx;

	// flow redirection for output
	if ((outIdx = block_contains(arg, '>')) >= 0) {
		switch (outIdx) {
			// stdout redir
			case 0: {
			    char* argAux = arg+1;
				strcpy(c->out_file, argAux);
				break;
			}
			// stderr redir
			case 1: {
				strcpy(c->err_file, &arg[outIdx + 1]);
				break;
			}
		}
		
		free(arg);
		c->type = REDIR;
		
		return true;
	}
	
	// flow redirection for input
	if ((inIdx = block_contains(arg, '<')) >= 0) {
		// stdin redir
		strcpy(c->in_file, arg + 1);
		
		c->type = REDIR;
		free(arg);
		
		return true;
	}
	
	return false;
}

// parses and sets a pair KEY=VALUE
// environment variable
static bool parse_environ_var(struct execcmd* c, char* arg) {

	// sets environment variables apart from the
	// ones defined in the global variable "environ"
	if (block_contains(arg, '=') > 0) {

		// checks if the KEY part of the pair
		// does not contain a '-' char which means
		// that it is not a environ var, but also
		// an argument of the program to be executed
		// (For example:
		// 	./prog -arg=value
		// 	./prog --arg=value
		// )
		if (block_contains(arg, '-') < 0) {
			c->eargv[c->eargc++] = arg;
			return true;
		}
	}
	
	return false;
}

int get_kind_of_pseudo_var(char* env_var){
    //implemented: ?, $, !
    if (strings_are_equal(env_var, "?"))
        return 0;
    else if (strings_are_equal(env_var, "$"))
        return 1;
    else if (strings_are_equal(env_var, "!"))
        return 2;
    else
        return -1;
}

static char* expand_environ_var(char* arg) {
    if (arg[0] == '$'){
        arg++;
        int pseudo_type;
        if ((pseudo_type = get_kind_of_pseudo_var(arg)) != -1){
            arg--;
            empty_buffer(arg);
            switch (pseudo_type) {
                case 0: {
                    sprintf(arg, "%d", status);
                    return arg;
                }
                case 1: {
                    sprintf(arg, "%d", getpid());
                    return arg;
                }
                case 2: {
                    sprintf(arg, "%d", back_pid);
                    return arg;
                }
                default: {
                    return arg;
                }
            }
        }

        char* aux = getenv(arg);
        arg--;

        if (aux == NULL){
            memset(arg, 0, strlen(arg));
            return NULL;
        }

        if (strlen(aux) > ARGSIZE) {
            arg = realloc(arg, strlen(aux));
        }

        for (unsigned int i=0; i<strlen(aux); i++){
            arg[i] = aux[i];
        }
    }
	return arg;
}

// parses one single command having into account:
// - the arguments passed to the program
// - stdin/stdout/stderr flow changes
// - environment variables (expand and set)
static struct cmd* parse_exec(char* buf_cmd) {

	struct execcmd* c;
	char* tok;
	int idx = 0, argc = 0;
	
	c = (struct execcmd*)exec_cmd_create(buf_cmd);
	
	while (buf_cmd[idx] != END_STRING) {
	
		tok = get_token(buf_cmd, idx);
		idx = idx + strlen(tok);
		
		if (buf_cmd[idx] != END_STRING)
			idx++;
		
		if (parse_redir_flow(c, tok))
            continue;

		if (parse_environ_var(c, tok))
			continue;
		
		expand_environ_var(tok);
		
		c->argv[argc++] = tok;
	}
	
	c->argv[argc] = (char*)NULL;
	c->argc = argc;
	
	return (struct cmd*)c;
}

// parses a command knowing that it contains
// the '&' char
static struct cmd* parse_back(char* buf_cmd) {

	int i = 0;
	struct cmd* e;

	while (buf_cmd[i] != '&')
		i++;
	
	buf_cmd[i] = END_STRING;
	
	e = parse_exec(buf_cmd);

	return back_cmd_create(e);
}

// parses a command and checks if it contains
// the '&' (background process) character
static struct cmd* parse_cmd(char* buf_cmd) {

	if (strlen(buf_cmd) == 0)
		return NULL;
		
	int idx;

	if ((idx = block_contains(buf_cmd, '&')) >= 0 &&
			buf_cmd[idx - 1] != '>')
		return parse_back(buf_cmd);

	//Esto sirve para crear comandos con n pipes recursivamente.
	if ((idx = block_contains(buf_cmd, '|')) >= 0) {
        return parse_line(buf_cmd);
    }
		
	return parse_exec(buf_cmd);
}

// parses the command line
// looking for the pipe character '|'
struct cmd* parse_line(char* buf) {
	
	struct cmd *r, *l;
	
	char* right = split_line(buf, '|');
	
	l = parse_cmd(buf);
	r = parse_cmd(right);
	
	return pipe_cmd_create(l, r);
}

//only valid if executed after is_cd_command returned true.
//parses cmd and puts the cd path into path.
void get_cd_path(char *cmd, char* path) {
    char auxCmd[ARGSIZE];
    memset(auxCmd, 0, ARGSIZE);
    strncpy(auxCmd, cmd, strlen(cmd));
    struct cmd* command = parse_line(auxCmd);
    struct execcmd* e = (struct execcmd*) command;
    if (e->argv[1] != NULL)
        strncpy(path, e->argv[1], strlen(e->argv[1]));
    else
        memset(path, '\0', ARGSIZE);
    free_command(command);
}

void erase_last_path(char *path) {
    char buf[ARGSIZE];
    if (getcwd(buf, ARGSIZE) == NULL){
        return;
    }

    int length = strlen(buf);
    if (!strings_are_equal(buf, "/")){
        int i = length;
        while (buf[i] != '/'){
            buf[i] = '\0';
            i--;
        }
    }
    strncpy(path, buf, strlen(buf));
}

