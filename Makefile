LDFLAGS = 
CFLAGS  = -g -std=c99 -Wall -pedantic
SRC     = main.c lexer.c hash.c util.c parser.c eval.c stat.c optimize.c
OBJ     = $(SRC:.c=.o)

stat: $(OBJ)
	cc -o $@ $(OBJ) $(LDFLAGS)

.c.o:
	cc $(CFLAGS) -c -o $@ $<

clean:
	rm -rf main *.o

.PHONY: clean
