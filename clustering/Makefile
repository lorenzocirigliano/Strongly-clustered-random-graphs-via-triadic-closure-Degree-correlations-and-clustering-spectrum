# Makefile for STC full simulation

CC = gcc
CFLAGS = -O3 -Wall -Wextra -std=c99
LDFLAGS = -lm

SRCDIR = src
OBJDIR = obj

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

TARGET = stc

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

# Dependencies
$(OBJDIR)/main.o: $(SRCDIR)/rng.h $(SRCDIR)/graph.h $(SRCDIR)/config_model.h \
                  $(SRCDIR)/triadic_closure.h $(SRCDIR)/measurements.h
$(OBJDIR)/graph.o: $(SRCDIR)/graph.h
$(OBJDIR)/config_model.o: $(SRCDIR)/config_model.h $(SRCDIR)/graph.h $(SRCDIR)/rng.h
$(OBJDIR)/triadic_closure.o: $(SRCDIR)/triadic_closure.h $(SRCDIR)/graph.h $(SRCDIR)/rng.h
$(OBJDIR)/measurements.o: $(SRCDIR)/measurements.h $(SRCDIR)/graph.h
$(OBJDIR)/rng.o: $(SRCDIR)/rng.h
