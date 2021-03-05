typedef enum {
	EXPR_VAR,
	EXPR_VAL,
	EXPR_OP,
} ExprType;

typedef enum {
	OP_EQ,
	OP_NE,
	OP_LT,
	OP_LE,
	OP_GT,
	OP_GE,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_MOD,
	OP_NOT,
	OP_NEG,
	OP_AND,
	OP_OR,
} ExprOp;

typedef union Expr {
	ExprType type;

	struct {
		ExprType type;
		char *ident;
	} var;

	struct {
		ExprType type;
		int i;
	} val;

	struct {
		ExprType type;
		ExprOp op;
		union Expr *args;
	} op;
} Expr;

typedef enum {
	STAT_EMPTY,
	STAT_ASSIGN,
	STAT_BLOCK,
	STAT_IF,
	STAT_WHILE,
	STAT_PRINT,
	STAT_READ,
} StatType;

typedef union Stat {
	StatType type;

	struct {
		StatType type;
		char *ident;
		Expr expr;
	} assign;

	struct {
		StatType type;
		int nstats;
		union Stat *stats;
	} block; 

	struct {
		StatType type;
		Expr cond;
		union Stat *a, *b;
	} _if;

	struct {
		StatType type;
		Expr cond;
		union Stat *body;
	} _while;

	struct {
		StatType type;
		Expr expr;
	} print;

	struct {
		StatType type;
		char *ident;
	} read;
} Stat;

int expr_op_is_binary(const ExprOp op);

void expr_free(Expr *e);
void stat_free(Stat *s);
