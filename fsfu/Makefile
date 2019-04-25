EXE=idump itype dblockvalid1 dblockvalid2 roote dot parent bdump binode1 binode2 dupaddr1 dupaddr2 iref1 iref2 refc dlink false control fcorrupt1 fcorrupt2
CC=gcc
INC=-Wall -Wextra


all: $(EXE)


.c: $(SOURCES) $(INC) $(HEADER)
	$(CC) $(DEFS) $(INC) $(CFLAGS) $< -o $@


clean:
	rm -f $(EXE)

restore:
	cp fs.img.bak fs.img

m:
	mv fs.img ..
