#! /bin/sh

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat > test.sh << 'EOF'
echo a
(echo b) && false || echo c
echo d | cat
echo e | cat ; echo f; echo g    
echo j && echo k || echo l ; echo m

mv fileA fileB
pwd | sort | cat > v    

echo write this to file > testfile.txt
cat testfile.txt
EOF

cat > test.exp <<'EOF'
a
b
c
d
e
f
g
j
k
m
write this to file
EOF

../timetrash test.sh> test.out 2>test.err || exit

diff -u test.exp test.out || exit
test ! -s test.err || {
    cat test.err
    exit 1
}

) || exit

rm -fr "$tmp"
