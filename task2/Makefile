CC ?= gcc
PYTHON ?= python3
CFLAGS ?= -Wall -Wextra -O2 -g
CPPFLAGS := -Iinclude
LDFLAGS := -lm

CORE_SOURCES := \
	src/c/byte_encoder.c \
	src/c/hashmap.c \
	src/c/merge_table.c \
	src/c/tokenizer_loader.c \
	src/c/tokenizer_splitter.c \
	src/c/tokenizer_encoder.c \
	src/c/tokenizer_decoder.c

.PHONY: all native python tests clean

all: native tests

native: build/lib/libtokenizer.so

build/lib/libtokenizer.so: $(CORE_SOURCES) | build/lib
	$(CC) $(CFLAGS) $(CPPFLAGS) -fPIC -shared $(CORE_SOURCES) -o $@ $(LDFLAGS)

python: native
	$(PYTHON) setup.py build_ext --build-lib build/python

build/bin/test_loader: src/c/hashmap.c src/c/merge_table.c src/c/tokenizer_loader.c src/c/byte_encoder.c tests/c/test.c | build/bin
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

build/bin/test_encoder: $(CORE_SOURCES) tests/c/test_encoder.c | build/bin
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

build/bin/tokenizer_test: src/c/main.c src/c/tokenizer_splitter.c src/c/byte_encoder.c | build/bin
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

tests: build/bin/test_loader build/bin/test_encoder build/bin/tokenizer_test

build/lib build/bin build/python build/obj:
	mkdir -p $@

clean:
	rm -rf build/bin build/obj build/python build/lib/libtokenizer.so
	rm -f src/python/tokenizer_wrapper.c *.so
	rm -f artifacts/c_tokens.txt artifacts/decoded_output.txt
