enum token_type {
	TOKEN_EOF     = 256,

	TOKEN_LE,
	TOKEN_GE,
	TOKEN_EQ,
	TOKEN_NE,
	TOKEN_AND,
	TOKEN_OR,
	TOKEN_WHILE,
	TOKEN_IF,
	TOKEN_ELSE,
	TOKEN_PRINT,
	TOKEN_READ,
	TOKEN_IDENT,
	TOKEN_NUM,
	TOKEN_COMMENT,
};

typedef struct {
	int type, line, col, i;

	union {
		int num;
		char *ident;
	} arg;
} Token;

typedef struct {
	void *data;
	int line, col, i;
	int c;
	int (*getchar)(void *);
} Lexer;

void lexer_init(Lexer *lexer, int (*getchar)(void *), void *data);
int  lexer_next(Lexer *lexer, Token *token);
