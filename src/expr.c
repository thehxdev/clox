enum {
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_GROUPING,
    EXPR_LITERAL,
};

typedef struct expr {
    int type;

    union {
        struct {
            token_t operator;
            struct expr *left, *right;
        } binary;

        struct {
            token_t operator;
            struct expr *right;
        } unary;

        struct expr *grouping;

        struct {
            int type;
            void *data;
        } literal;
    };
} expr_t;

expr_t *expr_new_binary(sth_arena_t *arena, token_t *operator, expr_t *left, expr_t *right) {
    expr_t *expr = sth_arena_alloc(arena, sizeof(*expr));
    *expr = (expr_t){
        .type = EXPR_BINARY,
        .binary = {
            .left = left,
            .right = right,
        },
    };
    memcpy(&expr->binary.operator, operator, sizeof(*operator));
    return expr;
}

void expr_pretty_print(expr_t *expr) {
    if (!expr) {
        printf("NULL expression! ");
        return;
    }

    switch (expr->type) {
        case EXPR_BINARY:
            {
                printf("( %s ", expr->binary.operator.lexeme);
                expr_pretty_print(expr->binary.left);
                expr_pretty_print(expr->binary.right);
                printf(") ");
            }
            break;

        case EXPR_UNARY:
            {
                printf("( %s ", expr->unary.operator.lexeme);
                expr_pretty_print(expr->unary.right);
                printf(") ");
            }
            break;

        case EXPR_GROUPING:
            {
                printf("( grouping ");
                expr_pretty_print(expr->grouping);
                printf(") ");
            }
            break;

        case EXPR_LITERAL:
            {
                void *data = expr->literal.data;
                if (!data) {
                    printf("nil ");
                } else {
                    switch (expr->literal.type) {
                        case TOKEN_NUMBER:
                            printf("%.3lf ", *(double*)data);
                            break;
                        case TOKEN_STRING:
                            printf("%s ", (char*)data);
                            break;
                    }
                }
            }
            break;
    }
}

void expr_eval(expr_t *expr) {
    switch (expr->type) {
        case EXPR_BINARY:
            break;

        case EXPR_UNARY:
            break;

        case EXPR_GROUPING:
            break;

        case EXPR_LITERAL:
            break;
    }
}
