#ifndef EXEC_H
#define EXEC_H

#include "defs.h"
#include "types.h"
#include "utils.h"
#include "freecmd.h"

extern struct cmd* parsed_pipe;

void exec_cmd(struct cmd* c);

pid_t executeCommand(struct cmd* cmd, int inFd, int outFd);

void write_redir(const struct execcmd *r);

void read_redir(const struct execcmd *r);

int err_out_redir(const struct execcmd* r);

void pipe_cmd(struct pipecmd *p);

void back_cmd(const struct backcmd *b);

void exec_command(const struct execcmd *e);

#endif // EXEC_H
