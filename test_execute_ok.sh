#! /bin/sh

# UCLA CS 111 Lab 1 - Test that valid syntax is processed correctly.

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >test.sh <<'EOF'
true

#Test short circuiting
true  || echo test1
false || echo test2
true  && echo test3
false && echo test4

# Test andor return values
true  && false || echo test5
true  && true  || echo test6
false && true  || echo test7
false && false || echo test8
true  || false || echo test9
true  || true  || echo test10
false || true  || echo test11
false || false || echo test12

# Test andor printouts
echo test13.1 && echo test13.2
echo test14.1 || echo test14.2

# Test pipeline return values
true  | true  || echo test15
true  | false || echo test16
false | true  || echo test17
false | false || echo test18

# Test pipelines
echo test19 | wc -c
echo test20 | tr a-z A-Z
echo test21 | tr a-z A-Z | wc -c

# Test the return value of subshells
#(true; true)   || echo test22
(true; true)
(true; false)  || echo test23
(false; true)  || echo test24
(false; false) || echo test25

# Test output of subshells
(echo test26_1; echo test26_2) | tr a-z A-Z
(echo test26.3 && echo test26.4) | tr a-z A-Z
( (echo test26.5;false) || echo test26.6) | tr a-z A-Z

# Test input of subshells
echo test27 | (tr a-z A-Z)
echo test28 | (tr a-z A-Z | cat -b)

# Test redirections
echo test29 > a
cat -b < a
cat -e < a > b
cat b

# Test redirections and pipelines
echo test30 | tr a-z A-Z > a
cat -b a
cat -e < a | tr A-Z a-z > b
cat b

# Test redirections from subshells
(echo test31) > a
(cat -b) < a
(cat -e) < a > b
cat b

# Test andors with pipelines
echo test32.1 && echo test32.2 | tr a-z A-Z
EOF

chmod 777 test.sh
./test.sh > test.exp 

../timetrash -t test.sh >test.out 2>test.err || (echo "timetrash not running" && false) || exit

diff -u test.exp test.out || (echo "diff unsuccessful" && false) || exit

test ! -s test.err || {
  cat test.err
  exit 1
}

) || exit

rm -fr "$tmp"
