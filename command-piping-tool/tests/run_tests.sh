#!/usr/bin/env bash
# Runs each pipeline through ./pipetool and through Bash, and compares the output.
set -u
cd "$(dirname "$0")/.."
[ -x ./pipetool ] || { echo "Build first: make"; exit 1; }

pass=0; fail=0
run_case() {   # id, cmd1, cmd2
    local id="$1" c1="$2" c2="$3"
    local tool bash_out
    tool=$(printf '%s\n%s\n' "$c1" "$c2" | ./pipetool 2>/dev/null \
           | sed -n '/^Final output:/,/^\[parent\]/p' | sed '1d;/^\[parent\]/d' | sed '/^$/d')
    bash_out=$(bash -c "$c1 | $c2" 2>/dev/null | sed '/^$/d')
    if [ "$tool" == "$bash_out" ]; then
        echo "PASS  $id  $c1 | $c2"; pass=$((pass+1))
    else
        echo "FAIL  $id  $c1 | $c2"; fail=$((fail+1))
    fi
}

run_case T1  'ls -l'                       'grep .c'
run_case T2  'cat /etc/passwd'             'wc -l'
run_case T3  'echo "hello operating system"' 'tr a-z A-Z'
run_case T4  'lsx -l'                      'wc -l'
run_case T5  'ls'                          'sort -r'
run_case T6  'seq 1 100000'                'tail -n 3'
run_case T7  'head -c 1048576 /dev/zero'   'wc -c'
run_case T8  'printf "banana\napple\ncherry\napple\n"' 'sort -u'
run_case T9  'ls -l'                       'grepx foo'
run_case T10 'seq 1 1000000'               'head -n 2'

echo "----"
echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
