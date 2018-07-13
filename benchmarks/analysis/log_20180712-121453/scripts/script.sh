#!/bin/bash

mkdir -p output

for file in *_on_time_performance-*.csv; do
    echo "$(basename "$file")"
    tail -n +49 "$(basename "$file")" > ./output/"$(basename "$file")"
done
