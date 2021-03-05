#include <stdlib.h>

#include "stat.h"

void
optimize_expr(Expr *e)
{
	int val;
	Expr tmp;

	switch (e->type) {
	case EXPR_OP:
		optimize_expr(e->op.args);
		if (expr_op_is_binary(e->op.op))
			optimize_expr(e->op.args + 1);
		switch (e->op.op) {
		case OP_ADD:
			if (e->op.args[0].type == EXPR_VAL && e->op.args[1].type == EXPR_VAL) {
				val = e->op.args[0].val.i + e->op.args[1].val.i;
				expr_free(&e->op.args[0]);
				expr_free(&e->op.args[1]);
				free(e->op.args);

				e->type = EXPR_VAL;
				e->val.i = val;
			} else if (e->op.args[0].type == EXPR_VAL && e->op.args[0].val.i == 0) {
				tmp = e->op.args[1];
				expr_free(&e->op.args[0]);
				free(e->op.args);
				*e = tmp;
			} else if (e->op.args[1].type == EXPR_VAL && e->op.args[1].val.i == 0) {
				tmp = e->op.args[0];
				expr_free(&e->op.args[1]);
				free(e->op.args);
				*e = tmp;
			}
		default:
			break;
		}
	default:
		break;
	}
}

void
optimize(Stat *s)
{
	Stat tmp;
	int i;

	if (!s)
		return;
	switch (s->type) {
	case STAT_ASSIGN:
		optimize_expr(&s->assign.expr);
		break;

	case STAT_PRINT:
		optimize_expr(&s->print.expr);
		break;

	case STAT_BLOCK:
		for (i = 0; i < s->block.nstats; i++)
			optimize(s->block.stats + i);
		break;

	case STAT_IF:
		optimize_expr(&s->_if.cond);
		optimize(s->_if.a);
		optimize(s->_if.b);

		if (s->_if.cond.type == EXPR_VAL) {
			if (s->_if.cond.val.i) {
				tmp = *s->_if.b;
				stat_free(s->_if.a);
				free(s->_if.a);

				*s = tmp;
			} else {
				tmp = *s->_if.a;
				stat_free(s->_if.b);
				free(s->_if.a);

				*s = tmp;
			}
		}
		break;

	case STAT_WHILE:
		optimize_expr(&s->_while.cond);
		optimize(s->_while.body);

		if (s->_while.cond.type == EXPR_VAL && s->_while.cond.val.i == 0) {
			expr_free(&s->_while.cond);
			stat_free(s->_while.body);
			free(s->_while.body);
			s->type = STAT_EMPTY;
		}

		break;

	default:
		break;
	}
}
