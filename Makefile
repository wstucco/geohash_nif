CFLAGS = -fPIC -O2 -Wno-unused-parameter -std=c99 -Wall

ERLANG_PATH = $(shell erl -eval 'io:format("~s", [lists:concat([code:root_dir(), "/erts-", erlang:system_info(version), "/include"])])' -s init stop -noshell)

CFLAGS += -I$(ERLANG_PATH)
CFLAGS += -Isrc
CFLAGS += -Ic_src

ifeq ($(shell uname),Darwin)
CFLAGS += -dynamiclib -undefined dynamic_lookup
endif

LDFLAGS =

.PHONY: all clean
all: geohash

geohash: priv/geohash.so

priv/geohash.so: src/geohash_nif.c c_src/geohash.c
	$(CC) $(CFLAGS) -shared $^ -o $@

clean:
	$(RM) -r priv/geo*.so*

