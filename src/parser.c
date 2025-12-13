static struct {
    sth_arena_t *arena;
    token_t *tokens;
    long current;
} parser = { 0 };

static inline token_t *parser_previous(void) {
    return &parser.tokens[parser.current - 1];
}

static inline token_t *parser_peek(void) {
    return &parser.tokens[parser.current];
}

static inline bool parser_is_at_end(void) {
    return (parser_peek()->type) == TOKEN_EOF;
}

static inline token_t *parser_advance(void) {
    if (!parser_is_at_end())
        parser.current += 1;
    return parser_previous();
}

static inline bool parser_check(int token_type) {
    if (parser_is_at_end())
        return false;
    return (parser_peek()->type) == token_type;
}

static inline bool parser_match(const int token_types[], long count) {
    for (long i = 0; i < count; i++) {
        if (parser_check(token_types[i])) {
            (void)parser_advance();
            return true;
        }
    }
    return false;
}

expr_t *parser_expression(void);

expr_t *parser_primary(void) {
    expr_t *expr = sth_arena_alloc(parser.arena, sizeof(*expr));
    if (parser_match((int[]){ TOKEN_LEFT_PAREN }, 1)) {
        *expr = (expr_t){
            .type = EXPR_GROUPING,
            .grouping = parser_expression(),
        };
        parser_advance();
        return expr;
    }

    expr->type = EXPR_LITERAL;
    if (parser_match((int[]){ TOKEN_TRUE }, 1)) {
        expr->literal.type = TOKEN_TRUE;
        expr->literal.data = sth_arena_alloc(parser.arena, sizeof(bool));
        *(bool*)expr->literal.data = true;
    } else if (parser_match((int[]){ TOKEN_FALSE }, 1)) {
        expr->literal.type = TOKEN_FALSE;
        expr->literal.data = sth_arena_alloc(parser.arena, sizeof(bool));
        *(bool*)expr->literal.data = false;
    } else if (parser_match((int[]){ TOKEN_NIL }, 1)) {
        expr->literal.type = TOKEN_NIL;
        expr->literal.data = NULL;
    } else if (parser_match((int[]){ TOKEN_STRING, TOKEN_NUMBER }, 2)) {
        token_t *token = parser_previous();
        expr->literal.type = token->type;
        expr->literal.data = token->literal;
    }
    return expr;
}

expr_t *parser_unary(void) {
    const int unary_matches[] = { TOKEN_BANG, TOKEN_MINUS, };
    if (parser_match(unary_matches, STH_ARRAY_LEN(unary_matches))) {
        token_t *operator = parser_previous();
        expr_t *right = parser_unary();
        expr_t *expr = sth_arena_alloc(parser.arena, sizeof(*expr));
        *expr = (expr_t){
            .type = EXPR_UNARY,
            .unary = {
                .right = right,
            },
        };
        memcpy(&expr->unary.operator, operator, sizeof(*operator));
        return expr;
    }
    return parser_primary();
}

expr_t *parser_factor(void) {
    const int matches[] = { TOKEN_SLASH, TOKEN_STAR, };
    expr_t *expr = parser_unary();
    while (parser_match(matches, STH_ARRAY_LEN(matches))) {
        token_t *operator = parser_previous();
        expr_t *right = parser_unary();
        expr_t *left = expr;
        expr = expr_new_binary(parser.arena, operator, left, right);
    }
    return expr;
}

expr_t *parser_term(void) {
    const int matches[] = { TOKEN_MINUS, TOKEN_PLUS, };
    expr_t *expr = parser_factor();
    while (parser_match(matches, STH_ARRAY_LEN(matches))) {
        token_t *operator = parser_previous();
        expr_t *right = parser_factor();
        expr_t *left = expr;
        expr = expr_new_binary(parser.arena, operator, left, right);
    }
    return expr;
}

expr_t *parser_comparison(void) {
    const int matches[] = {
        TOKEN_GREATER,
        TOKEN_GREATER_EQUAL,
        TOKEN_LESS,
        TOKEN_LESS_EQUAL,
    };
    expr_t *expr = parser_term();
    while (parser_match(matches, STH_ARRAY_LEN(matches))) {
        token_t *operator = parser_previous();
        expr_t *right = parser_term();
        expr_t *left = expr;
        expr = expr_new_binary(parser.arena, operator, left, right);
    }
    return expr;
}

expr_t *parser_equality(void) {
    const int matches[] = { TOKEN_BANG_EQUAL, TOKEN_EQUAL_EQUAL, };
    expr_t *expr = parser_comparison();
    while (parser_match(matches, STH_ARRAY_LEN(matches))) {
        token_t *operator = parser_previous();
        expr_t *right = parser_comparison();
        expr_t *left = expr;
        expr = expr_new_binary(parser.arena, operator, left, right);
    }
    return expr;
}

expr_t *parser_expression(void) {
    return parser_equality();
}

expr_t *parser_parse(sth_arena_t *arena, token_t *tokens) {
    parser.arena = arena;
    parser.tokens = tokens;
    parser.current = 0;
    return parser_expression();
}
