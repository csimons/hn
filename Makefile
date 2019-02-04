PROGRAM_NAME=hn
OBJ_DIR=obj
BIN_DIR=bin

OS=$(shell uname -s | tr A-Z a-z)
ifeq ($(OS),linux)
STATIC_LIBDIR=/usr/lib/x86_64-linux-gnu
else
STATIC_LIBDIR=/usr/local/lib
endif

LIB=\
	-lcurl \
	-lxml2 \
	-ldl \
	-lpthread \
	-lssl \
	-lcrypto

INC=\
	-I/usr/include/libxml2

CCFLAGS=-std=c11 -g -Wall -Wextra -Werror -D_GNU_SOURCE

#$(STATIC_LIBDIR)/libxml2.a
STATIC_LIBS=\
	$(STATIC_LIBDIR)/libssl.a \
	$(STATIC_LIBDIR)/libcrypto.a

OBJ=\
	$(OBJ_DIR)/http.o \
	$(OBJ_DIR)/main.o \
	$(OBJ_DIR)/strfind.o \
	$(OBJ_DIR)/substring.o \
	$(OBJ_DIR)/types.o

HEADERS=\
	src/http.h \
	src/strfind.h \
	src/substring.h \
	src/types.h

$(OBJ_DIR)/%.o: src/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CCFLAGS) $(INC) -c $^ -o $@

$(BIN_DIR)/$(PROGRAM_NAME): $(OBJ) $(HEADERS)
	mkdir -p $(BIN_DIR)
	$(CC) $(CCFLAGS) $(STATIC_LIBS) $(INC) $(OBJ) -o $@ $(LIB)

.PHONY: index
index:
	rm -f tags && ctags -R

.PHONY: format
format:
	clang-format \
		-style='{IndentWidth: 8, TabWidth: 8, UseTab: Always}' \
		-i src/*.c src/*.h

.PHONY: install
install: $(BIN_DIR)/$(PROGRAM_NAME)
	cp $(BIN_DIR)/$(PROGRAM_NAME) /usr/local/bin
	chmod 0755 /usr/local/bin/$(PROGRAM_NAME)
	mkdir -p /usr/local/share/man/man1/
	cp man/$(PROGRAM_NAME).1 /usr/local/share/man/man1/
	chmod 0644 /usr/local/share/man/man1/$(PROGRAM_NAME).1

.PHONY: uninstall
uninstall:
	rm -f /usr/local/bin/$(PROGRAM_NAME)
	rm -f /usr/local/share/man/man1/$(PROGRAM_NAME).1

.PHONY: clean
clean:
	rm -rf $(BIN_DIR)
	rm -rf $(OBJ_DIR)
