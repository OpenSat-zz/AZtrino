#!/bin/sh

mkdir new
mkdir missing
mkdir double
rm $1.missing
rm $1.new
rm missing/$1.missing
rm new/$1
rm double/$1

cat $1 | sort | uniq > tmp

X=0
while read LINE
do
  
  if grep -q "^[ ]*$LINE " tmp
  then
    check=`grep -c "^[ ]*$LINE " tmp`
    if [ "$check" != "1" ]
	then echo $check
	grep "^[ ]*$LINE " tmp >> double/$1
    fi
    grep "^[ ]*$LINE " tmp >> new/$1
  else
    echo $LINE >> missing/$1.missing
  fi
done < neutrino.locales.needed

rm tmp



#"^[ ]*EPGPlus.reset_settings "