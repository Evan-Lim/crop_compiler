# ============================================================
# CROP Compiler Makefile (Complete)
# ============================================================

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./src
LDFLAGS = -lm -lfl

LEX = flex
YACC = bison -d -v

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = .
EXAMPLES_DIR = examples

# All source files
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/ast.c \
       $(SRC_DIR)/symbol.c \
       $(SRC_DIR)/typecheck.c \
       $(SRC_DIR)/codegen.c \
       $(SRC_DIR)/lexer.c \
       $(SRC_DIR)/parser.c \
       $(SRC_DIR)/options.c

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

TARGET = crop-compiler

.PHONY: all clean test install help

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo ""
	@echo "✓ CROP Compiler built successfully!"
	@echo "  Run: ./$(TARGET) examples/hello.crop"
	@echo ""

# Generate lexer and parser
$(SRC_DIR)/lexer.c: $(SRC_DIR)/crop.l
	$(LEX) -o $@ $<

$(SRC_DIR)/parser.c $(SRC_DIR)/parser.h: $(SRC_DIR)/crop.y
	$(YACC) -o $(SRC_DIR)/parser.c $<

# Compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	rm -f $(SRC_DIR)/lexer.c
	rm -f $(SRC_DIR)/parser.c $(SRC_DIR)/parser.h
	rm -f $(SRC_DIR)/crop.output
	rm -f *.crop.c *.crop.elf *.crop
	rm -f examples/*.c examples/*.elf examples/*.crop
	@echo "✓ Cleaned all generated files"

test: $(TARGET)
	./$(TARGET) $(EXAMPLES_DIR)/hello.crop --target=native
	@echo ""
	@echo "✓ Test complete!"

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

help:
	@echo "CROP Compiler Makefile"
	@echo ""
	@echo "  make          - Build the compiler"
	@echo "  make clean    - Remove all generated files"
	@echo "  make test     - Compile hello.crop"
	@echo "  make install  - Install to /usr/local/bin"
	@echo ""
