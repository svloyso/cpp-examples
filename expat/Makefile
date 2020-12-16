CC = gcc
CFLAGS = -Iinclude -O0 -Wall -Werror -std=c99 -g
LDFLAGS = -lexpat

EXE = lab06
SRCDIR = src
BINDIR = bin

OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BINDIR)/%.o,$(wildcard $(SRCDIR)/*.c))

all: $(EXE)

$(EXE): $(BINDIR) $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXE) $(LDFLAGS)
	
$(BINDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -MMD -o $@ $<

include $(wildcard $(BINDIR)/*.d)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(BINDIR) $(EXE)

.PHONY: clean all
