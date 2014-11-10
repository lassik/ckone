CC=gcc -Wall -Wextra -Wno-unused-parameter -pedantic -std=c99 -g -O -c
LD=gcc -g -o

OBJ=ckone.o disasm.o sim.o insn.o mem.o parser.o reg.o size.o sym.o die.o

ckone: $(OBJ)
	$(LD) ckone $(OBJ)

ckone.o: ckone.c
	$(CC) ckone.c

disasm.o: disasm.c
	$(CC) disasm.c

sim.o: sim.c
	$(CC) sim.c

insn.o: insn.c
	$(CC) insn.c

mem.o: mem.c
	$(CC) mem.c

parser.o: parser.c
	$(CC) parser.c

reg.o: reg.c
	$(CC) reg.c

size.o: size.c
	$(CC) size.c

sym.o: sym.c
	$(CC) sym.c

die.o: die.c
	$(CC) die.c

clean:
	rm -f ckone $(OBJ)
