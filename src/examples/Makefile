SOURCES=$(wildcard *.c)
TARGETS=$(SOURCES:.c=)

LDFLAGS=-lkinetic-c-client.0.12.0 -lpthread -lssl -lcrypto -ljson-c
CFLAGS=-std=c99

all : $(TARGETS)
	$(ECHO) $(TARGETS)

% : %.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

clean :
	$(RM) $(TARGETS)

.PHONY : clean
