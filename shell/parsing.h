#ifndef PARSING_H
#define PARSING_H

#include "defs.h"
#include "types.h"
#include "createcmd.h"
#include "utils.h"
#include "printstatus.h"
#include "freecmd.h"

struct cmd* parse_line(char* b);

bool is_cd_command(const char* cmd);

void get_cd_path(char* cmd, char* path);

void erase_last_path(char* path);

int get_kind_of_pseudo_var(char* env_var);

#endif // PARSING_H
