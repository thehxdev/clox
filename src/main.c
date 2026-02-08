static sth_arena_t *arena = NULL;

static void usage(const char *program_name) {
    fprintf(stderr, "Usage: %s <script-file>\n", program_name);
}

int main(int argc, char *argv[]) {
    int err = 64;
    if (argc > 2) {
        usage(argv[0]);
    } else if (argc == 2) {
        arena = sth_arena_new(STH_ARENA_DEFAULT_CONFIG());
        err = interpreter_run_file(arena, argv[1]);
    } else {
        arena = sth_arena_new(STH_ARENA_DEFAULT_CONFIG());
        err = interpreter_run_prompt(arena);
    }
    if (arena) {
        sth_arena_destroy(arena);
    }
    return err;
}
