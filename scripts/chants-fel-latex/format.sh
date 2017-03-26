#!/bin/sh

original_folder=$(pwd)
include_file=$original_folder/$2/include.tex

echo "{ \huge" > $include_file

cd $1
for file in $(find -type f -name '*.txt')
do
  output_file="$(echo $file | sed 's/.txt$/.tex/;s/.\///')"
  $original_folder/format.py $file > $original_folder/$2/$output_file
  echo "\input{$original_folder/$2/$output_file}" >> $include_file
done

echo "}" >> $include_file

cd $original_folder/$2

pdflatex $original_folder/main.tex

cd $original_folder
