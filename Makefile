PROGRAM = grep
SOURCES = src/grep.c

CFLAGS = -Wall -Wextra -Werror

all: $(PROGRAM)

$(PROGRAM): $(SOURCES)
	gcc $(CFLAGS) -o $(PROGRAM) $(SOURCES)

test: $(PROGRAM)
	tests/grep_tests.sh

clean:
	rm -f $(PROGRAM) *.o system_grep.txt my_grep.txt system_err.txt my_err.txt