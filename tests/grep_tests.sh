#!/bin/bash

function grep_test {
    OPTIONS="$1"
    PATTERN="$2"
    INPUT="$3"

    EXPECTED_CMD="grep $OPTIONS $PATTERN $INPUT"
    MY_CMD="./grep $OPTIONS $PATTERN $INPUT"

    echo "Running test: $MY_CMD"
    
    if [ "$PATTERN" = "" ]; then
        echo "OPTIONS = $OPTIONS"
        echo "PATTERNS = $PATTERN"
        echo "INPUT = $INPUT"
        cat test1.txt | GREP_COLORS='' grep $OPTIONS "$INPUT" > system_grep.txt 2> system_err.txt
        cat test1.txt | ./grep $OPTIONS "$INPUT" > my_grep.txt 2> my_err.txt
    fi

    if [ "$INPUT" = "stdin" ]; then
        echo "Who is the best programmer? It's me" | GREP_COLORS='' grep $OPTIONS $PATTERN > system_grep.txt 2> system_err.txt
        echo "Who is the best programmer? It's me" | ./grep $OPTIONS $PATTERN > my_grep.txt 2> my_err.txt
    else
        GREP_COLORS='' eval "$EXPECTED_CMD" < /dev/null > system_grep.txt 2> system_err.txt
        eval "$MY_CMD" < /dev/null > my_grep.txt 2> my_err.txt
    fi

    if diff -q my_grep.txt system_grep.txt > /dev/null && diff -q my_err.txt system_err.txt > /dev/null; then
        echo "PASS"
    else
        echo "FAIL: Outputs differ"
        echo "Stdout diff:"
        diff -u system_grep.txt my_grep.txt || true
        echo "Stderr diff:"
        diff -u system_err.txt my_err.txt || true
        exit 1
    fi
}

FLAGS=("" "-i" "-v" "-c" "-n" "-l" "-v -c" "-v -l" "-v -n -c" "-n -v" "-l -c")
TESTS=("test1.txt" "test2.txt" "test3.txt" "test4.txt" "test5.txt" "test.txt")
PATTERNS=("is" "c.r" "My" "killkill" "")

for j in "${TESTS[@]}"
do
    for p in "${PATTERNS[@]}"
    do
        for i in "${FLAGS[@]}"
        do
            grep_test "$i" "$p" "$j"
        done
    done
done

rm -f my_grep.txt system_grep.txt my_err.txt system_err.txt

echo "All integration tests passed!"