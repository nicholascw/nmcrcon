CFLAGS=-std=gnu99 -I. -O3 -Wall -Wextra -Wno-missing-field-initializers -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIE -pie
LIBS=
SRC=$(wildcard *.c)
HDR=$(wildcard *.h)
OBJS=$(patsubst %.c, obj/%.o, $(SRC))

.PHONY: all clean

all: format obj nmcrcon

format:
	$(foreach n, $(SRC), clang-format -i $(n); )
	$(foreach n, $(HDR), clang-format -i $(n); )

nmcrcon: obj $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LIBS)

clean:
	$(RM) -r obj nmcrcon vgcore.* callgrind.out.*

obj/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

obj:
	mkdir -p obj
