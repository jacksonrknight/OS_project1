SRC := src
OBJ := obj
BIN := bin
EXECUTABLE:= shell

SRCS := $(wildcard $(SRC)/*.c)
OBJS := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))
INCS := -Iinclude/
DIRS := $(OBJ)/ $(BIN)/
EXEC := $(BIN)/$(EXECUTABLE)

CC := gcc
CFLAGS := -g -Wall -std=c99 $(INCS)
LDFLAGS :=

all: $(EXEC)

# Link all objects (lexer.o is included, but its main is renamed at compile time)
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC) $(LDFLAGS)

# Generic compile rule
$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Special rule for lexer.o: rename main() to avoid conflict, without changing the file.
$(OBJ)/lexer.o: $(SRC)/lexer.c
	$(CC) $(CFLAGS) -Dmain=lexer_demo_main -c $< -o $@

run: $(EXEC)
	$(EXEC)

clean:
	rm -f $(OBJ)/*.o $(EXEC)

$(shell mkdir -p $(DIRS))

.PHONY: run clean all