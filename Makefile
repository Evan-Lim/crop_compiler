# ============================================================
# CROP Compiler Makefile
# ============================================================

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./src
LDFLAGS = -lm

LEX = flex
YACC = bison -d -v

SRC_DIR = src
BUILD_DIR = build
EXAMPLES_DIR = examples

SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/ast.c \
       $(SRC_DIR)/symbol.c \
       $(SRC_DIR)/typecheck.c \
       $(SRC_DIR)/codegen.c \
       $(SRC_DIR)/lexer.c \
       $(SRC_DIR)/parser.c \
       $(SRC_DIR)/options.c

GENERATED = $(SRC_DIR)/lexer.c $(SRC_DIR)/parser.c $(SRC_DIR)/parser.h

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

TARGET = crop-compiler

.PHONY: all clean test install help

all: $(GENERATED) $(TARGET)

$(SRC_DIR)/lexer.c: $(SRC_DIR)/crop.l
	@echo "  [FLEX]   $<"
	$(LEX) -o $@ $<

$(SRC_DIR)/parser.c $(SRC_DIR)/parser.h: $(SRC_DIR)/crop.y
	@echo "  [BISON]  $<"
	$(YACC) -o $(SRC_DIR)/parser.c $<

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR) $(GENERATED)
	@echo "  [CC]     $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
	@echo "  [LD]     $@"
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo ""
	@echo "✓ CROP Compiler built successfully!"
	@echo "  Run: ./$(TARGET) examples/hello.crop"
	@echo ""

clean:
	@echo "  [CLEAN]  Removing generated files..."
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	rm -f $(SRC_DIR)/lexer.c
	rm -f $(SRC_DIR)/parser.c $(SRC_DIR)/parser.h
	rm -f $(SRC_DIR)/crop.output
	# Keep checked-in examples/simulate.c and demo executables intact.
	rm -f $(EXAMPLES_DIR)/hello.c $(EXAMPLES_DIR)/blink.c \
	      $(EXAMPLES_DIR)/minimal.c $(EXAMPLES_DIR)/greenhouse.c $(EXAMPLES_DIR)/*.elf
	rm -f *.c *.elf *.crop
	@echo "✓ Cleaned all generated files (source .crop files preserved)"

test: $(TARGET)
	@echo ""
	@echo "  [TEST]   Compiling examples/hello.crop"
	@tmp=$$(mktemp -d); \
	  cp $(EXAMPLES_DIR)/hello.crop $$tmp/hello.crop; \
	  ./$(TARGET) $$tmp/hello.crop --target=native && \
	  grep -q 'output_led_set(true);' $$tmp/hello.c && \
	  grep -q '__attribute__((weak)) bool sensor_button_read_opt()' $$tmp/hello.c && \
	  grep -q 'output_led_commit();' $$tmp/hello.c && \
	  grep -q 'every_0();' $$tmp/hello.c && \
	  ! grep -q 'output_led_set(false);' $$tmp/hello.c; \
	  result=$$?; rm -rf $$tmp; exit $$result
	@echo ""
	@echo "✓ Test complete!"

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

help:
	@echo "CROP Compiler Makefile"
	@echo ""
	@echo "  make          - Build the compiler"
	@echo "  make clean    - Remove generated files (preserves .crop sources)"
	@echo "  make test     - Compile hello.crop"
	@echo "  make install  - Install to /usr/local/bin"
	@echo ""
