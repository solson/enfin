# Based on a Makefile by Curtis McEnroe (@programble)

CC := clang
OUTPUT := enfin
CFLAGS := -std=gnu99 -D_GNU_SOURCE -MMD -Iinclude/ -Wall -Wextra
LDFLAGS :=

# Find sources
SOURCES := $(shell find src/ -name '*.c')
OBJECTS := $(SOURCES:%.c=%.o)
DFILES  := $(SOURCES:%.c=%.d)

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	@echo -e " [\033[32;1mLD\033[0m] $@"
	@$(CC) $(LDFLAGS) $^ -o $@

# Dependency files from -MMD
-include $(DFILES)

%.o: %.c
	@echo -e " [\033[34;1mCC\033[0m] $<"
	@$(CC) -c $(CFLAGS) $< -o $@

clean:
	@echo -e " [\033[31;1mRM\033[0m] $(OBJECTS)"
	@rm -f $(OBJECTS)
	@echo -e " [\033[31;1mRM\033[0m] $(DFILES)"
	@rm -f $(DFILES)
	@echo -e " [\033[31;1mRM\033[0m] $(OUTPUT)"
	@rm -f $(OUTPUT)

.PHONY: all clean
