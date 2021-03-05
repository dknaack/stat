#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "util.h"

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

static const struct {
	char *str;
	int type;
} keywords[] = {
	/* keyword     type       */
	{ "while",    TOKEN_WHILE, },
	{ "if",       TOKEN_IF,    },
	{ "else",     TOKEN_ELSE,  },
	{ "print",    TOKEN_PRINT, },
	{ "read",     TOKEN_READ,  },
	{ "and",      TOKEN_AND,   },
	{ "or",       TOKEN_OR,    },
};

static int 
keyword(const char *str)
{
	int i;

	for (i = 0; i < LENGTH(keywords); i++)
		if (strcmp(keywords[i].str, str) == 0)
			return keywords[i].type;
	return TOKEN_IDENT;
}

void
lexer_init(Lexer *l, int (*getchar)(void *), void *data)
{
	l->getchar = getchar;
	l->data = data;
	l->c = (*getchar)(data);
	l->i = l->col = l->line = 1;
}

int
lexer_next(Lexer *l, Token *token)
{
	int c, d, i;
	size_t size = 1024;

	l->col--;
	if (l->c == '\n')
		l->line--;

	token->type = TOKEN_EOF;
	for (c = l->c; c != EOF; c = l->c = l->getchar(l->data)) {
		l->col++;

		if (c == '\n') {
			l->i += l->col;
			l->col = 0;
			l->line++;
		}

		switch (token->type) {
		case TOKEN_EOF:
			if (isspace(c))
				continue;
			token->i = l->i;
			token->col = l->col;
			token->line = l->line + 1;

			if (isalpha(c) || c == '_') {
				token->type = TOKEN_IDENT;
				token->arg.ident = ecalloc(1, size);
				token->arg.ident[0] = c;
				i = 1;
			} else if (isdigit(c)) {
				token->type = TOKEN_NUM;
				token->arg.num = c - '0';
			} else {
				token->type = c;
			} 

			break;

		case '/':
			if (c != '*')
				return 1;

			token->type = TOKEN_COMMENT;
			i = 1;
			break;

		case TOKEN_COMMENT:

			if (d == '/' && c == '*')
				i++;
			if (d == '*' && c == '/')
				i--;
			if (i == 0)
				token->type = TOKEN_EOF;
			d = c;
			break;

		case TOKEN_IDENT:
			if (!isalnum(c) && c != '_') {
				token->arg.ident[i] = '\0';
				if ((token->type = keyword(token->arg.ident)) == TOKEN_IDENT)
					return 1;

				free(token->arg.ident);
				return 1;
			}

			if (i + 1 > size) {
				size *= 2;
				if (!(token->arg.ident = realloc(token->arg.ident, size)))
					die("realloc:");
			}

			token->arg.ident[i++] = c;
			break;

		case TOKEN_NUM:
			if (!isdigit(c))
				return 1;
			token->arg.num *= 10;
			token->arg.num += c - '0';
			break;

		default:
			if (c == '=') {
				switch (token->type) {
				case '<': token->type = TOKEN_LE; break;
				case '>': token->type = TOKEN_GE; break;
				case '!': token->type = TOKEN_NE; break;
				case '=': token->type = TOKEN_EQ; break;
				default:  return 1;
				}
			} else {
				return 1;
			}
		}
	}

	return 0;
}
