#!/bin/bash

mkdir -p output

for file in *gaussian*.csv; do
    echo "$(basename "$file")"
    tail -n +19 "$(basename "$file")" > ./output/"$(basename "$file")"
done
