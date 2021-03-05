#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "stat.h"
#include "parser.h"
#include "lexer.h"
#include "util.h"

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

typedef struct {
	Lexer l;
	Token t;
	int accepted;
	
	const char *path;
	const char *src;
	unsigned int i, len;
} Parser;

static int accept(Parser *p, int type);
static int error(Parser *p, const char *msg);
static int getch(Parser *p);

static int stat(Parser *p, Stat *s);
static int stat_if(Parser *p, Stat *s);
static int stat_while(Parser *p, Stat *s);
static int stat_print(Parser *p, Stat *s);
static int stat_read(Parser *p, Stat *s);
static int stat_block(Parser *p, Stat *s);
static int stat_assign(Parser *p, Stat *s, Token t);

static int expr(Parser *p, Expr *e);
static int expr_op(Parser *p, Expr *e, int rank);
static int expr_val(Parser *p, Expr *e);
static int expr_var(Parser *p, Expr *e);
static int expr_unary(Parser *p, Expr *e);

static const int *rank[] = {
	(int []){ 1, TOKEN_OR },
	(int []){ 1, TOKEN_AND },
	(int []){ 2, TOKEN_EQ, TOKEN_NE },
	(int []){ 4, '<', TOKEN_LE, '>', TOKEN_GE },
	(int []){ 2, '+', '-' },
	(int []){ 3, '*', '/', '%' },
};

int
accept(Parser *p, int type)
{
	if (p->accepted)
		lexer_next(&p->l, &p->t);
	p->accepted = (p->t.type == type);
	return p->accepted;
}

int
error(Parser *p, const char *msg)
{
	int i;
	const char *end = strchr(p->src + p->t.i - 1, '\n');

	if (!end)
		end = p->src + p->len + 1;
	i = end - p->src + p->t.i;

	fprintf(stderr, "%s:%d:%d: %s\n", p->path, p->t.line, p->t.col, msg);
	fprintf(stderr, "|\n"
					"| %.*s"
					"| ", i, p->src + p->t.i - 1);
	fprintf(stderr, "%*.s^ %s\n", p->t.col - 1, "", msg);
	return -1;
}

int
getch(Parser *p)
{
	if (p->i < p->len)
		return p->src[p->i++];
	else 
		return -1;
}

int
stat(Parser *p, Stat *s)
{
	int i;
	Token t;

	if ((i = stat_if(p, s))) {
		s->type = STAT_IF;
	} else if ((i = stat_while(p, s))) {
		s->type = STAT_WHILE;
	} else if ((i = stat_print(p, s))) {
		s->type = STAT_PRINT;
	} else if ((i = stat_read(p, s))) {
		s->type = STAT_READ;
	} else if ((i = stat_block(p, s))) {
		s->type = STAT_BLOCK;
	} else if (accept(p, TOKEN_IDENT)) {
		t = p->t;
		if ((i = stat_assign(p, s, t))) {
			s->type = STAT_ASSIGN;
		} else {
			p->t = t;
			free(t.arg.ident);
			return error(p, "Unexpected identifier");
		}
	} else if (accept(p, ';')) {
		s->type = STAT_EMPTY;
	} else {
		return error(p, "Not a statement");
	}
	return i;
}

int
stat_if(Parser *p, Stat *s)
{
	int ret;

	if (!accept(p, TOKEN_IF))
		return 0;
	accept(p, TOKEN_IF);
	s->type = STAT_IF;

	if (expr(p, &s->_if.cond) <= 0)
		return -1;

	s->_if.a = ecalloc(1, sizeof(Stat));
	if ((ret = stat_block(p, s->_if.a)) == 0)
		return error(p, "Expected '{'");
	else if (ret < 0)
		return -1;
	if (accept(p, TOKEN_ELSE)) {
		s->_if.b = ecalloc(1, sizeof(Stat));
		if ((ret = stat_block(p, s->_if.b)) == 0)
			return error(p, "Expected '{'");
		else if (ret < 0)
			return -1;
	}

	return 1;
}

int
stat_while(Parser *p, Stat *s)
{
	int err;

	if (!accept(p, TOKEN_WHILE))
		return 0;
	s->type = STAT_WHILE;

	if (expr(p, &s->_while.cond) <= 0)
		return -1;

	s->_while.body = ecalloc(1, sizeof(Stat));
	if (stat_block(p, s->_while.body) <= 0)
		return -1;

	return 1;
}

int
stat_print(Parser *p, Stat *s)
{
	int ret;
	if (!accept(p, TOKEN_PRINT))
		return 0;
	s->type = STAT_PRINT;

	if ((ret = expr(p, &s->print.expr)) <= 0)
		return error(p, "Expected expression");

	if (!accept(p, ';'))
		return error(p, "Expected ';'");
	return 1;
}

int
stat_read(Parser *p, Stat *s)
{
	if (!accept(p, TOKEN_READ))
		return 0;
	s->type = STAT_READ;

	if (!accept(p, TOKEN_IDENT))
		return error(p, "Expected identifier");
	s->read.ident = p->t.arg.ident;

	if (!accept(p, ';'))
		return error(p, "Expected ';'");
	return 1;
}

int
stat_block(Parser *p, Stat *s)
{
	size_t size = 256 * sizeof(Stat);

	if (!accept(p, '{'))
		return 0;
	s->type = STAT_BLOCK;
	s->block.nstats = 0;
	s->block.stats = ecalloc(1, size);

	while (!accept(p, '}')) {
		if (stat(p, &s->block.stats[s->block.nstats++]) <= 0)
			return -1;
	}

	return 1;
}

int
stat_assign(Parser *p, Stat *s, Token t)
{
	if (!accept(p, '='))
		return 0;
	s->type = STAT_ASSIGN;
	s->assign.ident = t.arg.ident;

	if (expr(p, &s->assign.expr) < 0)
		return error(p, "Invalid expression");
	if (!accept(p, ';'))
		return error(p, "Expected ';'");
	return 1;
}

int
expr(Parser *p, Expr *e)
{
	return expr_op(p, e, 0);
}

int
expr_unary(Parser *p, Expr *e)
{
	int ret = 1;

	if (accept(p, '-')) {
		e->type = EXPR_OP;
		e->op.op = OP_NEG;
		e->op.args = ecalloc(1, sizeof(Expr));
		if (expr_unary(p, &e->op.args[0]) <= 0)
			return -1;
	} else if (accept(p, '!')) {
		e->type = EXPR_OP;
		e->op.op = OP_NOT;
		e->op.args = ecalloc(1, sizeof(Expr));
		if (expr_unary(p, &e->op.args[0]) <= 0)
			return -1;
	} else if (accept(p, '+')) {
		if (expr_unary(p, e) < 0)
			return -1;
	} else if ((ret = expr_var(p, e))) {
		e->type = EXPR_VAR;
	} else if ((ret = expr_val(p, e))) {
		e->type = EXPR_VAL;
	} else if (accept(p, '(')) {
		if (expr(p, e) <= 0)
			return error(p, "Error parsing expression");
		if (!accept(p, ')'))
			return error(p, "Expected: ')'");
	} else {
		return 0;
	}

	return ret;
}

int
expr_val(Parser *p, Expr *e)
{
	if (accept(p, TOKEN_NUM)) {
		e->type = EXPR_VAL;
		e->val.i = p->t.arg.num;
		return 1;
	}

	return 0;
}

int
expr_var(Parser *p, Expr *e)
{
	if (accept(p, TOKEN_IDENT)) {
		e->type = EXPR_VAR;
		e->var.ident = p->t.arg.ident;
		return 1;
	}

	return 0;
}

int
expr_op(Parser *p, Expr *e, int r)
{
	int i, n, ret;
	Expr left, right;

	if (r >= LENGTH(rank)) {
		ret = expr_unary(p, e);
		return ret == 0? -1 : ret;
	}

	if (expr_op(p, e, r + 1) < 0)
		return -1;
	n = rank[r][0];
	for (left = *e;; left = *e) {
		for (i = 1; i <= n; i++)
			if (accept(p, rank[r][i]))
				break;
		if (i > n)
			return 1;

		if (expr_op(p, &right, r + 1) <= 0)
			return error(p, "Expected expression");

		e->type = EXPR_OP;
		e->op.args = ecalloc(2, sizeof(Expr));
		e->op.args[0] = left;
		e->op.args[1] = right;
		switch (rank[r][i]) {
		case TOKEN_AND: e->op.op = OP_AND; break;
		case TOKEN_OR:  e->op.op = OP_OR;  break;
		case TOKEN_EQ:  e->op.op = OP_EQ;  break;
		case TOKEN_NE:  e->op.op = OP_NE;  break;
		case TOKEN_LE:  e->op.op = OP_LE;  break;
		case TOKEN_GE:  e->op.op = OP_GE;  break;
		case '+':       e->op.op = OP_ADD; break;
		case '-':       e->op.op = OP_SUB; break;
		case '*':       e->op.op = OP_MUL; break;
		case '/':       e->op.op = OP_DIV; break;
		case '%':       e->op.op = OP_MOD; break;
		case '<':       e->op.op = OP_LT;  break;
		case '>':       e->op.op = OP_GT;  break;
		default:						   break;
		}
	}

	return 1;
}

Stat *
parse(const char *path, const char *src, unsigned int len)
{
	size_t size = 1024 * sizeof(Stat);
	int ret;
	Parser p = {0};
	Stat *s;

	if (!src || !len)
		return NULL;

	p.accepted = 1;
	p.path = path;
	p.src = src;
	p.len = len;
	p.i = 0;
	lexer_init(&p.l, (int (*)(void *))getch, &p);

	s = ecalloc(1, sizeof(Stat));
	s->type = STAT_BLOCK;
	s->block.nstats = 0;
	s->block.stats = ecalloc(1, size);
	while (!accept(&p, TOKEN_EOF)) {
		if ((ret = stat(&p, &s->block.stats[s->block.nstats++])) <= 0)
			return NULL;
		if (s->block.nstats + 1 >= size) {
			size *= 2;
			if (!(s = realloc(s, size)))
				die("realloc:");
		}
	}

	if (ret < 0) {
		stat_free(s);
		free(s);
		return NULL;
	} else {
		return s;
	}
}
