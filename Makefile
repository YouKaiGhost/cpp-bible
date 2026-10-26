CXX ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -g
LDFLAGS ?=

SINGLE_PROGRAMS := \
	00-hello-world/hello \
	01-types/types \
	02-expressions/expressions \
	03-control-flow/control \
	04-functions/functions \
	04-functions/args \
	05-arrays-pointers/arrays \
	05-arrays-pointers/pointers \
	06-strings/strings \
	07-classes/classes \
	09-templates/templates \
	10-undefined/portable \
	11-capstone/calc

MULTI_PROGRAMS := \
	08-storage/storage

PROGRAMS := $(SINGLE_PROGRAMS) $(MULTI_PROGRAMS)
RUN_PROGRAMS := $(sort $(PROGRAMS))

.PHONY: all run check clean

all: $(PROGRAMS)

$(SINGLE_PROGRAMS): %: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

# 多文件程序在这里显式链接。
08-storage/storage: 08-storage/storage.cpp 08-storage/counter.cpp 08-storage/counter.h
	$(CXX) $(CXXFLAGS) 08-storage/storage.cpp 08-storage/counter.cpp -o $@ $(LDFLAGS)

run: all
	@set -e; for program in $(RUN_PROGRAMS); do \
		printf '\n===== %s =====\n' "$$program"; \
		./"$$program"; \
	done

check:
	$(MAKE) clean
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -Werror" all

clean:
	rm -f $(PROGRAMS)
