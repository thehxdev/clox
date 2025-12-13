volatile bool had_error = false;

void error_report(int line, const char *where, const char *message) {
    fprintf(stderr, "[line %d] Error%s: %s\n", line, where, message);
    had_error = true;
}

void error(int line, const char *message) {
    error_report(line, "", message);
}
