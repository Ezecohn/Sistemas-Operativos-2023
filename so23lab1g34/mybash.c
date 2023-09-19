#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <glib.h>
#include <unistd.h>

#include "command.h"
#include "execute.h"
#include "parser.h"
#include "parsing.h"
#include "builtin.h"

struct scommand_s
{
    GQueue *list; // lista de string(comandos escritos por stdin)
    char *out;    // redireccionamiento out
    char *in;     // redireccionamiento in
};
struct pipeline_s
{
    GQueue *scommand_list; // lista de scommand
    bool is_fore;          // se ejecuta de manera foreground o background?
};

static void mostrarDirectorioActual(void)
{
    char *directorioActual = getcwd(NULL, 0); // Obtener el directorio actual
    if (directorioActual != NULL)
    {
        printf("%s> ", directorioActual);
        free(directorioActual); // Liberar la memoria asignada por getcwd
    }
    else
    {
        perror("Error al obtener el directorio actual");
    }
}

static void show_prompt(void)
{
    printf("mybash ");
    mostrarDirectorioActual();
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    pipeline pipe;
    Parser input;
    bool quit = false;

    while (!quit)
    {
        input = parser_new(stdin);
        show_prompt();

        pipe = parse_pipeline(input);

        if (pipe != NULL)
        {
            execute_pipeline(pipe);
            quit = parser_at_eof(input);
        }

        /* Hay que salir luego de ejecutar? */
    }
    parser_destroy(input);
    input = NULL;
    return EXIT_SUCCESS;
}
