.PHONY: all clean re mrproper run todo fixme xxx quirks

bin := bin/linux64
build := build/linux64
src := src

dotexe := .app
exe := $(bin)/dracosky$(dotexe)
cc := gcc
include:=include
cflags := \
	-Wall -Wextra -Wpedantic \
	-std=gnu11 -fms-extensions \
	-D_POSIX_C_SOURCE=199309L -D_GNU_SOURCE \
	-I$(include) -I/usr/include/freetype2 \
	-g \
	-m64
ldlibs := -lX11 -lGL -lasound -lpthread -lm -lrt -lfreetype

cfiles := $(wildcard \
	$(src)/*.c \
	$(src)/*/*.c \
	$(src)/*/*/*.c \
	$(src)/*/*/*/*.c \
	$(src)/*/*/*/*/*.c \
)
ofiles := $(patsubst $(src)/%.c,$(build)/%.o,$(cfiles))

all: $(exe)

$(build)/%.o: $(src)/%.c
	@mkdir -p $(@D)
	$(cc) $(cflags) -c $< -o $@

$(exe): $(ofiles)
	@mkdir -p $(@D)
	$(cc) $(cflags) $^ -o $@ $(ldlibs)

clean:
	rm -rf $(wildcard $(build)/*)
re: clean all
mrproper: re

run: all
	./$(exe)

debug: all
	gdb -q ./$(exe) -ex run

greplines:=3

todo:
	-@grep -n -R -A $(greplines) TODO $(src) $(include)
xxx:
	-@grep -n -R -A $(greplines) XXX $(src) $(include)
fixme:
	-@grep -n -R -A $(greplines) FIXME $(src) $(include)
quirks: todo fixme xxx
