#include <stdlib.h>

#include "stat.h"

int
expr_op_is_binary(const ExprOp op)
{
	switch (op) {
	case OP_ADD:
	case OP_SUB:
	case OP_MUL:
	case OP_DIV:
	case OP_MOD:
	case OP_AND:
	case OP_OR:
	case OP_LE:
	case OP_LT:
	case OP_GE:
	case OP_GT:
	case OP_EQ:
	case OP_NE:
		return 1;
	default:
		return 0;
	}
}

void 
expr_free(Expr *e)
{
	switch (e->type) {
	case EXPR_VAR:
		free(e->var.ident);
		break;
	case EXPR_OP:
		expr_free(&e->op.args[0]);

		switch (e->op.op) {
		case OP_EQ:
		case OP_NE:
		case OP_LT:
		case OP_LE:
		case OP_GT:
		case OP_GE:
		case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_DIV:
		case OP_MOD:
			expr_free(&e->op.args[1]);
		default:
			break;
		}

		free(e->op.args);
	default: 
		break;
	}
}

void
stat_free(Stat *s)
{
	int i;

	if (!s)
		return;

	switch (s->type) {
	case STAT_ASSIGN:
		free(s->assign.ident);
		expr_free(&s->assign.expr);
		break;

	case STAT_IF:
		expr_free(&s->_if.cond);
		stat_free(s->_if.b);
		stat_free(s->_if.a);
		free(s->_if.a);
		break;

	case STAT_WHILE:
		expr_free(&s->_while.cond);
		stat_free(s->_while.body);
		break;

	case STAT_PRINT:
		expr_free(&s->print.expr);
		break;

	case STAT_READ:
		free(s->read.ident);
		break;

	case STAT_BLOCK:
		for (i = 0; i < s->block.nstats; i++)
			stat_free(&s->block.stats[i]);
		free(s->block.stats);
		break;

	default:
		break;
	}
}
