#define LEXER_INITIAL_TOKENS_CAP 1024

enum {
    // Single-character tokens.
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS, TOKEN_SEMICOLON,
    TOKEN_SLASH, TOKEN_STAR,

    // One or two character tokens.
    TOKEN_BANG, TOKEN_BANG_EQUAL, TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL, TOKEN_LESS, TOKEN_LESS_EQUAL,

    // Literals.
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

    // Keywords.
    TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE, TOKEN_FUN, TOKEN_FOR,
    TOKEN_IF, TOKEN_NIL, TOKEN_OR, TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER,
    TOKEN_THIS, TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

    TOKEN_EOF,
};

typedef struct token {
    int type, line;
    char *lexeme;
    void *literal;
} token_t;

static struct lexer {
    sth_arena_t *arena;
    const char *source;
    size_t source_size, start, current, line;
} lexer = { 0 };

static struct {
    char *key;
    int value;
} *lexer_keywords = NULL;


static inline bool lexer_is_at_end() {
    return lexer.current >= lexer.source_size;
}

static inline char lexer_advance() {
    return lexer.source[lexer.current++];
}

static inline bool lexer_match(char c) {
    if (lexer_is_at_end() || lexer.source[lexer.current] != c)
        return false;

    lexer.current++;
    return true;
}

static inline char lexer_peek(void) {
    if (lexer_is_at_end())
        return '\0';
    return lexer.source[lexer.current];
}

static inline char lexer_peek_next(void) {
    if (lexer.current + 1 >= lexer.source_size)
        return '\0';
    return lexer.source[lexer.current + 1];
}

static inline bool lexer_is_digit(char c) {
    return (c >= '0' && c <= '9');
}

static inline bool lexer_is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

static inline bool lexer_is_alpha_numeric(char c) {
    return lexer_is_alpha(c) || lexer_is_digit(c);
}

static void lexer_token_add(token_t *tokens, int type, void *literal) {
    token_t token = ((token_t){
        .type = type,
        .line = lexer.line,
        .literal = literal,
    });
    token.lexeme = sth_arena_strndup(lexer.arena,
                                     &lexer.source[lexer.start],
                                     lexer.current - lexer.start);
    arrput(tokens, token);
}

static void lexer_scan_string(token_t *tokens) {
    while (lexer_peek() != '\"' && !lexer_is_at_end()) {
        if (lexer_peek() == '\n')
            lexer.line++;
        lexer_advance();
    }

    if (lexer_is_at_end()) {
        error(lexer.line, "unterminated string");
        return;
    }

    char *literal = sth_arena_strndup(lexer.arena,
                                      &lexer.source[lexer.start+1],
                                      lexer.current - lexer.start - 1);
    lexer_advance();

    lexer_token_add(tokens, TOKEN_STRING, literal);
}

static void lexer_scan_identifier(token_t *tokens) {
    while (lexer_is_alpha_numeric(lexer_peek()))
        lexer_advance();

    size_t substring_size = lexer.current - lexer.start;
    char *substring = sth_arena_strndup(lexer.arena,
                                        &lexer.source[lexer.start],
                                        substring_size);

    int type = shget(lexer_keywords, substring);
    sth_arena_pop(lexer.arena, substring_size+1);

    lexer_token_add(tokens, type, NULL);
}

static void lexer_scan_number(token_t *tokens) {
    while (lexer_is_digit(lexer_peek()))
        lexer_advance();

    if (lexer_peek() == '.' && lexer_is_digit(lexer_peek_next())) {
        lexer_advance();
        while (lexer_is_digit(lexer_peek()))
            lexer_advance();
    }

    double *d = sth_arena_alloc(lexer.arena, sizeof(*d));
    *d = atof(&lexer.source[lexer.start]);
    lexer_token_add(tokens, TOKEN_NUMBER, d);
}

void lexer_init(void) {
    hmdefault(lexer_keywords, TOKEN_IDENTIFIER);
    shput(lexer_keywords, "and", TOKEN_AND);
    shput(lexer_keywords, "class", TOKEN_CLASS);
    shput(lexer_keywords, "else", TOKEN_ELSE);
    shput(lexer_keywords, "false", TOKEN_FALSE);
    shput(lexer_keywords, "for", TOKEN_FOR);
    shput(lexer_keywords, "fun", TOKEN_FUN);
    shput(lexer_keywords, "if", TOKEN_IF);
    shput(lexer_keywords, "nil", TOKEN_NIL);
    shput(lexer_keywords, "or", TOKEN_OR);
    shput(lexer_keywords, "print", TOKEN_PRINT);
    shput(lexer_keywords, "return", TOKEN_RETURN);
    shput(lexer_keywords, "super", TOKEN_SUPER);
    shput(lexer_keywords, "this", TOKEN_THIS);
    shput(lexer_keywords, "true", TOKEN_TRUE);
    shput(lexer_keywords, "while", TOKEN_WHILE);
    shput(lexer_keywords, "var", TOKEN_VAR);
}

token_t *lexer_scan_tokens(sth_arena_t *arena, const char *source, size_t size) {
    token_t *tokens = NULL;
    arrsetcap(tokens, LEXER_INITIAL_TOKENS_CAP);

    lexer.arena = arena;
    lexer.start = 0;
    lexer.current = 0;
    lexer.line = 1;
    lexer.source = source;
    lexer.source_size = size;

    while (!lexer_is_at_end()) {
        char c = lexer_advance();

        switch (c) {
            case '(':
                lexer_token_add(tokens, TOKEN_LEFT_PAREN, NULL);
                break;
            case ')':
                lexer_token_add(tokens, TOKEN_RIGHT_PAREN, NULL);
                break;
            case '{':
                lexer_token_add(tokens, TOKEN_RIGHT_BRACE, NULL);
                break;
            case '}':
                lexer_token_add(tokens, TOKEN_LEFT_BRACE, NULL);
                break;
            case ',':
                lexer_token_add(tokens, TOKEN_COMMA, NULL);
                break;
            case '.':
                lexer_token_add(tokens, TOKEN_DOT, NULL);
                break;
            case '-':
                lexer_token_add(tokens, TOKEN_MINUS, NULL);
                break;
            case '+':
                lexer_token_add(tokens, TOKEN_PLUS, NULL);
                break;
            case ';':
                lexer_token_add(tokens, TOKEN_SEMICOLON, NULL);
                break;
            case '*':
                lexer_token_add(tokens, TOKEN_STAR, NULL);
                break;

            case '\"':
                lexer_scan_string(tokens);
                break;

            case '!':
                lexer_token_add(tokens, lexer_match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG, NULL);
                break;
            case '=':
                lexer_token_add(tokens, lexer_match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL, NULL);
                break;
            case '<':
                lexer_token_add(tokens, lexer_match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS, NULL);
                break;
            case '>':
                lexer_token_add(tokens, lexer_match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER, NULL);
                break;

            case '/':
                {
                    if (lexer_match('/')) {
                        while (lexer_peek() != '\n' && !lexer_is_at_end())
                            lexer_advance();
                    } else if (lexer_match('*')) {
                        // TODO: support for block comments (/* */) with nesting support
                    } else {
                        lexer_token_add(tokens, TOKEN_SLASH, NULL);
                    }
                }
                break;

            case '\n':
                lexer.line++;
            case '\t':
            case '\r':
            case ' ':
                break;

            default:
                {
                    if (lexer_is_digit(c)) {
                        lexer_scan_number(tokens);
                    } else if (lexer_is_alpha(c)) {
                        lexer_scan_identifier(tokens);
                    } else {
                        error(lexer.line, "unexpected character");
                    }
                }
                break;
        } // end switch
        lexer.start = lexer.current;
    } // end while

    lexer_token_add(tokens, TOKEN_EOF, NULL);
    return tokens;
}

void lexer_destroy(void) {
    hmfree(lexer_keywords);
    lexer_keywords = NULL;
    memset(&lexer, 0, sizeof(lexer));
}
