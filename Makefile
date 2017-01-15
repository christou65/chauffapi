# Chauffapi makefile
# CLU 2016

EXEC=Chauffapi
SRC= $(wildcard *.c)
OBJ= $(SRC:.c=.o)
LIBS=-lpthread
LDFLAGS=
CFLAGS= -c -Wall -lpthread
CC=gcc


all: $(EXEC)

Chauffapi: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: all clean cleanall install


cleanall:
	-@rm -rf *.o
	-@rm -f Chauffapi
	-@rm -rf /usr/share/Chauffapi

clean:
	-@rm -rf *.o
	-@rm -f Chauffapi

install:

	@echo Install files in INTROOT
	-@mkdir -p /usr/share/Chauffapi/script/
	-@cp -f ./script/*.sh /usr/share/Chauffapi/script
	-@cp -f ./Chauffapi /usr/share/Chauffapi
	-@chown root /usr/share/Chauffapi/Chauffapi
	-@chmod 4755 /usr/share/Chauffapi/Chauffapi

