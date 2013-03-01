# Based on a Makefile by Curtis McEnroe (@programble)

CC := clang
OUTPUT := enfin
CFLAGS := -std=gnu99 -MMD -Iinclude/ -Wall -Wextra \
	$(shell pkg-config --cflags talloc)
LDFLAGS := $(shell pkg-config --libs talloc)

# Find sources
SOURCES := $(shell find src/ -name '*.c')
OBJECTS := $(SOURCES:%.c=%.o)
DFILES  := $(SOURCES:%.c=%.d)

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

# Dependency files from -MMD
-include $(DFILES)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(DFILES) $(OUTPUT)

.PHONY: all clean
