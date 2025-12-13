#define INTERPRETER_PROMPT_BUFFER_SIZE 1024

static void interpreter_run(sth_arena_t *arena, const char *source, size_t size) {
    token_t *tokens = lexer_scan_tokens(arena, source, size);

    expr_t *expr = parser_parse(arena, tokens);
    expr_pretty_print(expr);
    putchar('\n');

    arrfree(tokens);
}

int interpreter_run_file(sth_arena_t *arena, const char *path) {
    size_t source_size;
    char *source = io_file_read(arena, path, &source_size);
    if (!source) {
        fprintf(stderr, "[ERROR] failed to read \'%s\' file: ", path);
        perror(NULL);
        return errno;
    }

    lexer_init();
    interpreter_run(arena, source, source_size);
    lexer_destroy();

    return (had_error) ? 65 : 0;
}

int interpreter_run_prompt(sth_arena_t *arena) {
    char source[INTERPRETER_PROMPT_BUFFER_SIZE];

    lexer_init();
    for (;;) {
        printf("> ");
        fflush(stdout);

        if (!fgets(source, INTERPRETER_PROMPT_BUFFER_SIZE, stdin)) {
            puts("Bye!");
            break;
        }

        interpreter_run(arena, source, strlen(source));
        had_error = false;
    }
    lexer_destroy();

    return 0;
}
