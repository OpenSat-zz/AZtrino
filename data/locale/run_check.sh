#!/bin/sh
for i in *.locale; do
echo "Updating $i"
./check.sh $i
done
