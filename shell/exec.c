#include "exec.h"
#include "parsing.h"

// sets the "key" argument with the key part of
// the "arg" argument and null-terminates it
static void get_environ_key(const char* arg, char* key) {
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets the "value" argument with the value part of
// the "arg" argument and null-terminates it
static void get_environ_value(const char* arg, char* value, int idx) {
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void set_environ_vars(const char** eargv, int eargc) {
    for (int i=0; i<eargc; i++) {
        char key[ARGSIZE];
        char value[ARGSIZE];
        get_environ_key(eargv[i], key);
        int idx = block_contains(eargv[i], '=');
        get_environ_value(eargv[i], value, idx);
        int overwrite = 1;
        setenv(key, value, overwrite);
    }
} 

// opens the file in which the stdin/stdout or
// stderr flow will be redirected, and returns
// the file descriptor
// 
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int open_redir_fd(const char* file, int flags) {
	if (flags == READ){
        return open(file, O_RDONLY, O_CLOEXEC);
	} else if (flags == WRITE || flags == ERR_OUT) {
        return open(file, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, S_IWUSR | S_IRUSR);
	} else {
	    return -1;
	}
}

static int check_what_redir_to_perform(struct execcmd* r){
    bool in_redir = (strlen(r->in_file) > 0);
    bool out_redir = (strlen(r->out_file) > 0);
    bool err_redir = (strlen(r->err_file) > 0);
    if (in_redir)
        return READ;
    else if (out_redir && err_redir)
        return ERR_OUT;
    else if (out_redir)
        return WRITE;
    else
        return ERROR;
}

pid_t executeCommand(struct cmd* cmd, int inFd, int outFd){
    pid_t actualPid;
    actualPid = fork();
    if (actualPid == 0){
        if (inFd != -1){
            dup2 (inFd, 0);
            close (inFd);
        }

        if (outFd != 1){
            dup2(outFd, 1);
            close(outFd);
        }
        struct execcmd* e = (struct execcmd*) cmd;
        set_environ_vars((const char **) e->eargv, e->eargc);
        if (cmd->type != EXEC){
            fprintf(stderr, "Error, comando no EXEC");
            _exit(-1);
        }
        return execvp (e->argv[0], e->argv);
    }
    return actualPid;
}

void write_redir(const struct execcmd *r) {
    //Write only. Creates the file if it didn't exist. If it did, it erases everything
    //it had. It is created with read/write permissions.
    int fd = open_redir_fd(r->out_file, WRITE);
    if (fd != -1){
        dup2(fd, 1);
        close(fd);
        execvp(r->argv[0], r->argv);
    } else {
        fprintf(stderr, "Error al abrir el archivo: %d\n", errno);
    }
}

void read_redir(const struct execcmd *r) {
    int fd = open_redir_fd(r->in_file, READ);
    if (fd != -1){
        dup2(fd, 0);
        execvp(r->argv[0], r->argv);
    } else {
        fprintf(stderr, "Error al abrir el archivo: %d\n", errno);
    }
}

int err_out_redir(const struct execcmd* r){
    //Write only. Creates the file if it didn't exist. If it did, it erases everything
    //it had. It is created with read/write permissions.
    int fdOut;
    if ((fdOut = open_redir_fd(r->out_file, ERR_OUT)) == -1){
        fprintf(stderr, "Error al abrir el archivo de out");
        return -1;
    }

    dup2(fdOut, 1);
    int fdErr;
    int outIdx = block_contains(r->err_file, '&');

    if (outIdx >= 0) {
        dup2(fdOut, 2);
    } else {
        if ((fdErr = open_redir_fd(r->err_file, ERR_OUT)) == -1) {
            fprintf(stderr,"Error al abrir el archivo de out");
            return -1;
        }
        dup2(fdErr, 2);
    }

    execvp(r->argv[0], r->argv);
    return 1;
}

void pipe_cmd(struct pipecmd *p) {
    int pipeFd[2];
    int inFd = 0;

    bool execute = true;
    while (execute){
        if (pipe2 (pipeFd, O_CLOEXEC) == -1){
            return;
        }
        executeCommand(p->leftcmd, inFd, pipeFd[1]);
        close (pipeFd[1]);
        inFd = pipeFd[0];
        execute = p->rightcmd->type == PIPE;
        if (execute)
            p = (struct pipecmd *) p->rightcmd;
    }

    if (inFd != 0)
        dup2(inFd, 0);

    struct execcmd* eFinal = (struct execcmd *) p->rightcmd;
    execvp(eFinal->argv[0], eFinal->argv);
}

void back_cmd(const struct backcmd *b) {
    struct execcmd *e;
    struct cmd* c = b->c;
    e = (struct execcmd*) c;

    int err = execvp(e->argv[0], e->argv);
    if (err) {
        printf("Error en exec\n");
    }
}

void exec_command(const struct execcmd *e) {
    set_environ_vars((const char **) e->eargv, e->eargc);
    int err = execvp(e->argv[0], e->argv);
    if (err) {
        fprintf(stderr, "Error en EXEC\n");
    }
}

void exec_cmd(struct cmd* cmd) {
    struct execcmd *e;
    struct backcmd *b;
    struct execcmd *r;
    struct pipecmd *p;

    switch (cmd->type) {
        case EXEC: {
            e = (struct execcmd *) cmd;
            exec_command(e);
            break;
        }

        case BACK: {
            b = (struct backcmd*) cmd;
            back_cmd(b);
            break;
        }

        case REDIR: {
            r = (struct execcmd *) cmd;
            int redir = check_what_redir_to_perform(r);
            switch (redir) {
                case READ: {
                    read_redir(r);
                    break;
                }
                case WRITE:{
                    write_redir(r);
                    break;
                }
                case ERR_OUT:{
                    int err_result = err_out_redir(r);
                    if (err_result == -1)
                        return;
                    break;
                }
                default:{
                    fprintf(stderr, "Error al redireccionar");
                    break;
                }
            }
            break;
        }
        case PIPE: {
            p = (struct pipecmd*) cmd;
            pipe_cmd(p);
            break;
        }
    }
}

