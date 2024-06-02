#include <stdlib.h>
#include <assert.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tests/syscall_mock.h"
#include "execute.h"
#include "command.h"
#include "builtin.h"
#include "strextra.h"

#define READ 0
#define WRITE 1

static void execute_command(scommand command)
{
    assert(command != NULL);
    unsigned int length = scommand_length(command);
    if (builtin_is_internal(command))
    {
        builtin_run(command);
        exit(EXIT_SUCCESS);
    }
    else
    {
        char **argument_list = calloc(length + 1, sizeof(char));
        for (unsigned int i = 0u; i < length; i++)
        {
            argument_list[i] = strdup(scommand_front(command));
            scommand_pop_front(command);
        }

        argument_list[length] = NULL;

        char *dir_out = scommand_get_redir_out(command);
        char *dir_in = scommand_get_redir_in(command);
        if (dir_in != NULL)
        {
            int fd_redir_in = open(dir_in, O_RDONLY, S_IWUSR);
            close(STDIN_FILENO);
            dup2(fd_redir_in, STDIN_FILENO);
            close(fd_redir_in);
        }
        if (dir_out != NULL)
        {
            int fd_redit_out = open(dir_out, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
            close(STDOUT_FILENO);
            dup2(fd_redit_out, STDOUT_FILENO);
            close(fd_redit_out);
        }

        // free(dir_out);
        // free(dir_in);
        for (unsigned int i = 0u; i <= length; i++)
        {
            if (execvp(argument_list[i], argument_list))
        {
            printf("ERROR: Comando desconocido! \n");
        }
        }
    }
}

void execute_pipeline(pipeline apipe)
{
    assert(apipe != NULL);
    char *invalid_cmd = "invalid command";
    if (pipeline_is_empty(apipe))
    {
        write(STDOUT_FILENO, invalid_cmd, strlen(invalid_cmd));
    }
    else if (scommand_is_empty(pipeline_front(apipe)))
    {
        pipeline_pop_front(apipe); /*si viene vacio lo saco al cmd*/
    }
    else if (builtin_alone(apipe))
    { /*si el comando tiene un pipe y es un built in*/
        builtin_run(pipeline_front(apipe));
        pipeline_pop_front(apipe);
    }
    else
    {
        unsigned int length = pipeline_length(apipe); // cantidad de comandos simples
        scommand cmd = pipeline_front(apipe);         /*agarro el primer comand*/
        bool have2_wait = pipeline_get_wait(apipe);
        pid_t pid1, pid2, pid3;
        int fd1[2], fd2[2];
        int status;
        if (length >= 2)
        {
            pipe(fd1); // Comunica el primer cmd con el segundo
        }

        pid1 = fork(); /**/

        /* Hijo: ejecuta primer comando */
        if (pid1 == 0)
        {
            if (length >= 2)
            {
                /* basicamente si tengo mas de 2 comandos los voy a tener que comunicar en otro caso hay un solo comando y solo lo tengo que ejecutar */
                close(fd1[READ]);                /*al no usarlo lo cierro*/
                dup2(fd1[WRITE], STDOUT_FILENO); /*basicamente el retorno es decir hago un duplicado entre stdout la salida del comando y lo meto en fd1*/
                close(fd1[WRITE]);               /*cierro el archivo*/
            }

            /* Ejecuto el cmd */
            execute_command(cmd);
        }

        else
        {
            if (length == 1)
            {
                if (have2_wait)
                {
                    waitpid(pid1, &status, WCONTINUED); /*esperara a que termine el proceso hijo*/
                }
            }
            else if (length >= 2)
            {

                close(fd1[WRITE]);         /*como no lo uso lo cierro*/
                pipeline_pop_front(apipe); /*mato al comando ya ejecutado*/
                cmd = pipeline_front(apipe);
                have2_wait = pipeline_get_wait(apipe);
                if (have2_wait)
                {
                    waitpid(pid1, &status, WCONTINUED);
                }
                if (length >= 3)
                {
                    pipe(fd2); /*creo otro pipe*/
                }
                pid2 = fork();

                if (pid2 == 0)
                {
                    dup2(fd1[READ], STDIN_FILENO);
                    close(fd1[READ]);
                    if (length >= 3)
                    {
                        close(fd2[READ]);
                        dup2(fd2[WRITE], STDOUT_FILENO);
                        close(fd2[WRITE]);
                    }

                    execute_command(cmd);
                }
                else
                {
                    close(fd1[READ]);
                    if (length == 2 && have2_wait)
                    {
                        waitpid(pid2, &status, WCONTINUED);
                    }
                    else if (length >= 3)
                    {
                        close(fd2[WRITE]);

                        pipeline_pop_front(apipe);
                        cmd = pipeline_front(apipe);
                        have2_wait = pipeline_get_wait(apipe);
                        if (have2_wait)
                        {
                            // waitpid(pid2, &status, WCONTINUED);
                        }
                        pid3 = fork();
                        /* Hijo: ejecuta segundo comando */
                        if (pid3 == 0)
                        {
                            /* Preaparar la entrada/salida */
                            dup2(fd2[READ], STDIN_FILENO);
                            close(fd2[READ]);
                            execute_command(cmd);
                        }
                        else
                        {
                            if (have2_wait)
                            {
                                waitpid(pid3, &status, WCONTINUED);
                            }
                        }
                    }
                }
            }
        }
    }
}