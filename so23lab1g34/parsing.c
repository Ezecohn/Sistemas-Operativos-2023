#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <glib.h>

#include "parsing.h"
#include "parser.h"
#include "command.h"


static scommand parse_scommand(Parser p) {
    assert(p != NULL && !parser_at_eof (p));
    scommand cmd = scommand_new();

    arg_kind_t type;
    char *arg = NULL;
    parser_skip_blanks(p); // Saltaemos todos los espacios en blanco que hubisen delante del comando escrito
    bool nulo = false;

    while (!parser_at_eof(p) && !nulo) {
        
        arg = parser_next_argument(p, &type);
        if(arg == NULL){
            nulo = true; // En caso de fallar al leer el argumento, se devuelve un scommand vacio
            /* Que devuelva un scommand vacio no significa que NO se haya reservado memoria,
               asi que de todas formas es necesario liberarla */
        }
        else
        {
            if (type == ARG_NORMAL) { 
                scommand_push_back(cmd, arg);
            } else if (type == ARG_INPUT) {         // Que deberia pasar si tenemos > out.txt out2.txt
                scommand_set_redir_in(cmd,arg);
            } else if (type == ARG_OUTPUT) {
                scommand_set_redir_out(cmd,arg);
            }
        } 
        parser_skip_blanks(p);
    }
    return cmd;
}


pipeline parse_pipeline(Parser p) {
    assert(p != NULL && !parser_at_eof (p));
    pipeline result = pipeline_new();
    scommand cmd = NULL;
    bool error = false;
    bool empty;
    bool another_pipe = true;
    
    cmd = parse_scommand(p);
    empty = scommand_is_empty(cmd);

    if (empty == true || cmd == NULL) {
        // No se encontraron argumentos, retorna NULL
        pipeline_destroy(result);
        return NULL;
    }

    while (another_pipe && !error) {
        // Intenta leer un operador de pipe
        bool is_pipe;
        parser_op_pipe(p, &is_pipe);

        if (is_pipe) {
            // Parsear el siguiente scommand
            scommand next_cmd = parse_scommand(p);
            if (next_cmd == NULL) {
                // Hubo un error de parseo en el siguiente scommand
                error = true;
            } else {
                // Agregar el comando actual a la pipeline
                pipeline_push_back(result, cmd);
                cmd = next_cmd; // El siguiente scommand se convierte en el actual
            }
        } else {
            // No se encontró un operador de pipe, terminar el ciclo
            another_pipe = false;
        }
    }

    if (!error) {
        // Agregar el último comando a la pipeline
        pipeline_push_back(result, cmd);
    } else {
        // Hubo un error, liberar recursos
        pipeline_destroy(result);
        result = NULL;
    }

    // Intenta leer un operador de background
    bool is_background;
    parser_op_background(p, &is_background);
    if (is_background && result != NULL) {
        // Marcar la pipeline como ejecución en background
        pipeline_set_wait(result, false);
    }

    // Consume espacios posteriores y caracteres hasta el final de línea
    parser_skip_blanks(p);
    bool garbage;
    parser_garbage(p, &garbage);

    if (error || garbage) {
        // Hubo error o se encontró basura, limpiar y retornar NULL
        pipeline_destroy(result);
        result = NULL;
    }

    return result;
}
