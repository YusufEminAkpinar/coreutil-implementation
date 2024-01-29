FLAGS=-Wall -Wextra -pedantic -ggdb
LIBS=
OUTDIR=./out/

ls1: ls1.c
	cc $(FLAGS) $(LIBS) -o $(OUTDIR)ls1 ls1.c
clean:
	test -n $(OUTDIR) && rm $(OUTDIR)*
