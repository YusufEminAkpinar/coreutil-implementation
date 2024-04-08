FLAGS := -Wall -Wextra -pedantic -ggdb
LIBS :=
# OUTDIR := ./out/

all: file1 ls1 cat1

file1: file1.c 
	cc $(FLAGS) $(LIBS) -o $(OUTDIR)file1 file1.c

ls1: ls1.c
	cc $(FLAGS) $(LIBS) -o $(OUTDIR)ls1 ls1.c


cat1: cat1.c
	cc $(FLAGS) $(LIBS) -o $(OUTDIR)cat1 cat1.c

clean:
	test $(OUTDIR) && rm $(OUTDIR)*


