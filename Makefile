CC := gcc

CFLAGS := -std=c17 -Wall -Wextra -Wpedantic -Werror -Iinclude

SRC := src/main.c src/args.c src/proc_info.c src/output.c
OBJ := build/main.o build/args.o build/proc_info.o build/output.o

TARGET := bin/procview
.PHONY: all debug release run clean

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p bin
	$(CC) $(OBJ) -o $(TARGET)

build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

debug: CFLAGS += -g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer
debug: clean all

release: CFLAGS += -O2
release: clean all

run: all
	./$(TARGET) $$

clean:
	rm -rf build bin

-include $(OBJ:.o=.d)

.PHONY: all debug release run clean