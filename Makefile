PROGRAM_NAME=hn
OBJ_DIR=obj
BIN_DIR=bin

OS=$(shell uname -s | tr A-Z a-z)
ifeq ($(OS),linux)
LIBDIR=/usr/lib/x86_64-linux-gnu
INC=-I/usr/include/libxml2
else
LIBDIR=$(shell brew --prefix openssl@3)/lib
INC=-I$(shell xcrun --show-sdk-path)/usr/include/libxml2
endif

LIB=\
	-lcurl \
	-lxml2 \
	-ldl \
	-lpthread \
	-lssl \
	-lcrypto

CFLAGS=-std=c11 -g -Wall -Wextra -Werror -D_GNU_SOURCE
LDFLAGS=-L$(LIBDIR)

DEBUG_OBJ_DIR=$(OBJ_DIR)/debug
DEBUG_BIN_DIR=$(BIN_DIR)/debug
SANFLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer

OBJ=\
	$(OBJ_DIR)/http.o \
	$(OBJ_DIR)/main.o \
	$(OBJ_DIR)/types.o

HEADERS=\
	src/http.h \
	src/types.h

DEBUG_OBJ=$(patsubst $(OBJ_DIR)/%.o,$(DEBUG_OBJ_DIR)/%.o,$(OBJ))

$(OBJ_DIR)/%.o: src/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INC) -c $^ -o $@

$(DEBUG_OBJ_DIR)/%.o: src/%.c
	mkdir -p $(DEBUG_OBJ_DIR)
	$(CC) $(CFLAGS) $(SANFLAGS) $(INC) -c $^ -o $@

$(BIN_DIR)/$(PROGRAM_NAME): $(OBJ) $(HEADERS)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(INC) $(OBJ) -o $@ $(LIB)

.PHONY: debug
debug: $(DEBUG_OBJ) $(HEADERS)
	mkdir -p $(DEBUG_BIN_DIR)
	$(CC) $(CFLAGS) $(SANFLAGS) $(LDFLAGS) $(INC) $(DEBUG_OBJ) -o $(DEBUG_BIN_DIR)/$(PROGRAM_NAME) $(LIB)

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
