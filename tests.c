#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "builtins.h"
#include "eval.h"
#include "parse.h"
#include "stream.h"
#include "tokenize.h"
#include "tests.h"

void stream_repl(char*, size_t, Stream*);
void tokenize_repl(char*);
void parse_repl(char*);
void eval_repl(char*, scamenv*);

enum { REPL_EVAL, REPL_PARSE, REPL_TOKENIZE, REPL_STREAM };
int main(int argc, char** argv) {
    // Run tests
    if (argc == 1 || strcmp(argv[1], "-r") != 0) {
        stream_tests();
        tokenize_tests();
        parse_tests();
        eval_tests();
        return 0;
    }
    // Run the REPL
    int mode = REPL_EVAL;
    char* buffer = NULL;
    size_t s_len = 0;
    Stream strm;
    stream_from_str(&strm, "");
    scamenv* env = scamenv_init(NULL);
    register_builtins(env);
    for (;;) {
        switch (mode) {
            case REPL_STREAM: printf("stream"); break;
            case REPL_TOKENIZE: printf("tokenize"); break;
            case REPL_PARSE: printf("parse"); break;
            case REPL_EVAL: printf("eval"); break;
            default: printf("unknown mode"); break;
        }
        printf(">>> ");
        int end = getline(&buffer, &s_len, stdin);
        // remove trailing newline
        if (end > 0) {
            buffer[--end] = '\0';
        }
        if (strcmp(buffer, "quit") == 0) {
            break;
        } else if (strcmp(buffer, "!eval") == 0) {
            mode = REPL_EVAL;
            continue;
        } else if (strcmp(buffer, "!parse") == 0) {
            mode = REPL_PARSE;
            continue;
        } else if (strcmp(buffer, "!tokenize") == 0) {
            mode = REPL_TOKENIZE;
            continue;
        } else if (strcmp(buffer, "!stream") == 0) {
            mode = REPL_STREAM;
            continue;
        }
        if (mode == REPL_STREAM) {
            stream_repl(buffer, end, &strm);
        } else if (mode == REPL_TOKENIZE) {
            tokenize_repl(buffer);
        } else if (mode == REPL_PARSE) {
            parse_repl(buffer);
        } else if (mode == REPL_EVAL) {
            eval_repl(buffer, env);
        }
    }
    if (buffer) free(buffer);
    stream_close(&strm);
    scamenv_free(env);
    return 0;
}

void print_stream(Stream* strm);

void stream_repl(char* command, size_t s_len, Stream* strm) {
    if (strstr(command, "open") == command && s_len >= 6) {
        char* fp = command + 5;
        stream_close(strm);
        stream_from_file(strm, fp);
    } else if (strstr(command, "feed") == command && s_len >= 6) {
        char* line = command + 5;
        stream_close(strm);
        stream_from_str(strm, line);
    } else {
        if (!strm) {
            printf("Error: stream not yet opened\n");
            return;
        }
        if (strcmp(command, "getc") == 0) {
            char c = stream_getc(strm);
            printf("'%c' (%d)\n", c, c);
        } else if (strcmp(command, "mark") == 0) {
            stream_mark(strm);
            printf("Set mem_flag to %d\n", strm->mem_flag);
        } else if (strcmp(command, "recall") == 0) {
            char* s = stream_recall(strm);
            printf("\"%s\"\n", s);
            free(s);
        } else if (strstr(command, "ungetc") == command && s_len == 8) {
            stream_ungetc(strm, command[7]);
        } else if (strcmp(command, "status") == 0) {
            print_stream(strm);
        } else {
            printf("unknown command\n");
        }
    }
}

void tokenize_repl(char* command) {
    Tokenizer tz;
    if (strstr(command, "open") == command && strlen(command) >= 6) {
        tokenizer_from_file(&tz, command + 5);
    } else {
        tokenizer_from_str(&tz, command);
    }
    print_all_tokens(&tz);
    tokenizer_close(&tz);
}

void parse_repl(char* command) {
    if (strstr(command, "open") == command && strlen(command) >= 6) {
        scamval* ast = parse_file(command + 5);
        scamval_print_ast(ast, 0);
        scamval_free(ast);
    } else {
        scamval* ast = parse_str(command);
        scamval_print_ast(ast, 0);
        scamval_free(ast);
    }
}

void eval_repl(char* command, scamenv* env) {
    scamval* v = eval_str(command, env);
    scamval_println(v);
    scamval_free(v);
}

void print_stream(Stream* strm) {
    printf("line=%d, col=%d, good=%d, ", strm->line, strm->col, strm->good);
    printf("mem_flag=%d, mem_line=%d, ", strm->mem_flag, strm->mem_line);
    printf("mem_col=%d, last_pos=%d, ", strm->mem_col, strm->last_pos);
    printf("mem_len=%d\n", strm->mem_len);
}
