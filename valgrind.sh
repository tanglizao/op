#/bin/sh
make clean
make
rm memcheck.log.*
valgrind --leak-check=full --show-reachable=yes --log-file=./memcheck.log `head -n 1 ./run.sh`
cat memcheck.log.*
