CC		=	gcc
CFLAGS	=	-g -Wall -Wextra -Werror -std=gnu89

LDLIBS	=	$(shell pkg-config --silence-errors --libs libkmod)
LDLIBS	+=	$(shell pkg-config --silence-errors --libs ncurses)
OBJS	=	../common/mem_ops.o ../common/deserialize.o ../common/string_utils.o netlink.o utils.o ui.o monks-log.o
SOURCES	=	monks-logs.c
DESTS	=	../bin/monks-logs

KMOD	=	$(shell pkg-config --exists libkmod && echo "1" || echo "0")

new: clean all

clean:
	@rm -rf *.o */*.o ../common/*.o $(DESTS)

all: _check_kmod $(DESTS)

_check_kmod:
ifeq ($(KMOD), 0)
CFLAGS += -D__NO_KMOD__
else
OBJS += lkm.o
endif

../bin/monks-logs: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
