#include "builtin.h"
#include "utils.h"

int exit_shell(char* cmd) {
    if (strings_are_equal(cmd, "exit")){
        return EXIT_SHELL;
    }
	return 0;
}

int cd(char* cmd) {
    if (is_cd_command(cmd)){
        char path[ARGSIZE];
        memset(path, 0, ARGSIZE);
        get_cd_path(cmd, path);
        int result;
        if (string_is_empty(path)){ //HOME
            char* home = getenv("HOME");
            char aux[ARGSIZE];
            if (getcwd(aux, ARGSIZE) == NULL){
                return -1;
            }
            if (!strings_are_equal(home, aux))
                result = chdir(home);
            else
                return 1;
        } else if (strings_are_equal(path, "..")) {
            erase_last_path(path);
            result = chdir(path);
        } else {
            result = chdir(path);
        }

        char buf[PRMTLEN] = {0};
        if (getcwd(buf, PRMTLEN) == NULL){
            return -1;
        }
        strncpy(promt, buf, PRMTLEN);

        if (result == 0) {
            return 1;
        } else {
            fprintf(stderr, "Error en change_directory");
            return -1;
        }
    }
    return 0;
}


int pwd(char* cmd) {
    if (strings_are_equal(cmd, "pwd")){
        void* result = getcwd(cmd, ARGSIZE);
        if (result != NULL){
            printf("%s\n", cmd);
            return 1;
        } else {
            return -1;
        }
    }
    return 0;
}

