#include <stdio.h>
#include <string.h>

#include "hash.h"
#include "stat.h"

static int eval_op(int op, Expr *args, HashMap *vars);
static int eval_expr(Expr e, HashMap *vars);

int
eval_op(int op, Expr *args, HashMap *vars)
{
	int arg = eval_expr(args[0], vars);

	switch (op) {
	case OP_AND: return  arg && eval_expr(args[1], vars);
	case OP_OR:  return  arg || eval_expr(args[1], vars);
	case OP_EQ:  return  arg == eval_expr(args[1], vars);
	case OP_NE:  return  arg != eval_expr(args[1], vars);
	case OP_LE:  return  arg <= eval_expr(args[1], vars);
	case OP_GE:  return  arg >= eval_expr(args[1], vars);
	case OP_LT:  return  arg <  eval_expr(args[1], vars);
	case OP_GT:  return  arg >  eval_expr(args[1], vars);
	case OP_ADD: return  arg +  eval_expr(args[1], vars);
	case OP_SUB: return  arg -  eval_expr(args[1], vars);
	case OP_MUL: return  arg *  eval_expr(args[1], vars);
	case OP_DIV: return  arg /  eval_expr(args[1], vars);
	case OP_MOD: return  arg %  eval_expr(args[1], vars);
	case OP_NOT: return !arg;
	case OP_NEG: return -arg;
	}

	return 0; /* UNREACHABLE */
}

static int
eval_expr(Expr e, HashMap *vars)
{
	long *l;
	switch (e.type) {
	case EXPR_OP:  return eval_op(e.op.op, e.op.args, vars);
	case EXPR_VAL: return e.val.i;
	case EXPR_VAR: 
		l = hashmap_get(vars, HASH_STR(e.var.ident));
		return l? *l : 0;
	}

	return 0; /* UNREACHABLE */
}

int
eval(Stat *s, HashMap *vars)
{
	char *ident;
	int i, val, ret;
	long *l;

	switch (s->type) {
	case STAT_ASSIGN:
		ident = s->assign.ident;
		val = eval_expr(s->assign.expr, vars);
		hashmap_set(vars, HASH_STR(ident), val);
		break;

	case STAT_IF:
		if (eval_expr(s->_if.cond, vars))
			return eval(s->_if.a, vars);
		else if (s->_if.b)
			return eval(s->_if.b, vars);
		break;

	case STAT_WHILE:
		while (eval_expr(s->_while.cond, vars))
			if ((ret = eval(s->_while.body, vars)))
				return ret;
		break;
	case STAT_BLOCK:
		for (i = 0; i < s->block.nstats; i++)
			if ((ret = eval(&s->block.stats[i], vars)))
				return ret;
		break;

	case STAT_READ:
		printf("%s = ", s->read.ident);
		scanf("%d", &val);
		hashmap_set(vars, HASH_STR(s->read.ident), val);
		break;

	case STAT_PRINT:
		printf("%d\n", eval_expr(s->print.expr, vars));
		break;

	default:
		break;
	}

	return 0;
}
