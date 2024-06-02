#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "builtin.h"
#include "strextra.h"
#include "command.h"
#include "tests/syscall_mock.h"

// "cd ../.."
// scommnad -> {[cd, ../..], null, null}

bool builtin_is_internal(scommand cmd){
    assert(cmd != NULL);
    return (strcmp(scommand_front(cmd), "cd") == 0 ||
            strcmp(scommand_front(cmd), "help") == 0 ||
            strcmp(scommand_front(cmd), "exit") == 0);
}

bool builtin_alone(pipeline p) {
    assert (p != NULL);
    return pipeline_length(p) == 1 && builtin_is_internal(pipeline_front(p));
}

void builtin_run(scommand cmd){
    assert(builtin_is_internal(cmd));
    if(strcmp(scommand_front(cmd), "cd") == 0){
        scommand_pop_front(cmd); // sacamos el "cd"
        int error = chdir(scommand_front(cmd));
        if (error != 0)
        {
            printf("Error al cambiar de directorio.\n");
        }
    }
    else if(strcmp(scommand_front(cmd), "help") == 0){
        printf("__________________________________________________________________\n\n"
               "             ▒█▀▄▀█ ▒█░░▒█ ▒█▀▀█ ░█▀▀█ ▒█▀▀▀█ ▒█░▒█               \n"
               "             ▒█▒█▒█ ▒█▄▄▄█ ▒█▀▀▄ ▒█▄▄█ ░▀▀▀▄▄ ▒█▀▀█               \n"
               "             ▒█░░▒█ ░░▒█░░ ▒█▄▄█ ▒█░▒█ ▒█▄▄▄█ ▒█░▒█               \n"
               "__________________________________________________________________\n"
               "Autores: Felipe Oliva - Ivan Godoy - Tomas Loyber - Ezequiel Cohn\n"
               "__________________________________________________________________\n\n"
               "Comandos internos:\n"
               "•cd: Cambiar de directorio actual al especificado.\n\n"
               "•help: Interfaz de ayuda, proporciona una breve descripcion de los\n"
               "comandos internos implementados.\n\n"
               "•exit: Salir de MyBash.\n");

    }
    else if(!strcmp(scommand_front(cmd), "exit")){
        exit(0);
    }
    
}

