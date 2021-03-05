#include <stdio.h>
#include <stdlib.h>

#include "hash.h"
#include "stat.h"
#include "parser.h"
#include "optimize.h"
#include "eval.h"
#include "util.h"

#define NVARS 1024

static int
read_file(FILE *fp, char **src, unsigned int *len)
{
	size_t s, size = 2 * BUFSIZ;

	if (!fp)
		return -1;

	*len = 0;
	*src = ecalloc(size, 1);
	while ((s = fread((*src) + (*len), 1, BUFSIZ, fp)) > 0) {
		*len += s;
		if ((*len) + BUFSIZ + 1 > size) {
			size += BUFSIZ;
			if (!(*src = realloc(*src, size)))
				return -1;
		}
	}

	(*src)[*len] = '\0';

	return 0;
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	char *src, *name;
	unsigned int len;
	long keys[NVARS] = {0}, vals[NVARS] = {0};
	HashMap vars = { keys, vals, NVARS };
	Stat *stat;

	if (argc <= 1)
		die("not enough arguments");
	fp = fopen(argv[1], "r");
	if (read_file(fp, &src, &len) == -1)
		die("%s:", argv[0]);

	if (!(stat = parse(argv[1], src, len)))
		die("Parsing error");

	optimize(stat);

	if (eval(stat, &vars) < 0)
		die("failed to evaluate statement");

	stat_free(stat);
	free(src);
	return 0;
}
