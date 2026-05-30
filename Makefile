CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -std=c11
ALL_SRC = $(wildcard src/*.c)
# Exclude test files in src/ (those starting with test or containing 'test')
SRC = $(filter-out src/test%.c src/*test*.c, $(ALL_SRC))
OBJ = $(SRC:.c=.o)
MODULE_OBJ = $(filter-out src/main.o, $(OBJ))
TARGET = bin/morse
TEST_TARGET = bin/test_modules
TEST_SRC = tests/test_modules.c

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_TARGET): $(MODULE_OBJ) $(TEST_SRC)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^ 

clean:
	rm -rf bin/*.o bin/morse bin/test_modules src/*.o tests/*.o

test: all $(TEST_TARGET)
	@echo "Running basic smoke test"
	@printf "0\n" | ./$(TARGET)
	@echo "Running module tests"
	@./$(TEST_TARGET)

.PHONY: all clean test

.PHONY: all clean test
