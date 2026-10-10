CXX ?= clang++
CXXFLAGS ?= -std=c++20 -O3 -Wall -Wextra -fPIC -Iinclude
AR ?= ar

SRC = src/value.cpp src/verbrt.cpp
OBJ = $(SRC:.cpp=.o)

all: libverbrt.a libverbrt.so test_runtime

libverbrt.a: $(OBJ)
	$(AR) rcs $@ $^

libverbrt.so: $(OBJ)
	$(CXX) -shared -o $@ $^ -ldl

test_runtime: tests/test_runtime.cpp libverbrt.a
	$(CXX) $(CXXFLAGS) tests/test_runtime.cpp libverbrt.a -ldl -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: test_runtime
	./test_runtime

clean:
	rm -f $(OBJ) libverbrt.a libverbrt.so test_runtime tests/*.o

.PHONY: all test clean
