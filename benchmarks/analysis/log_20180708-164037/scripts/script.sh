#!/bin/bash

mkdir -p output

for file in *_on_time_performance-*.csv; do
    echo "$(basename "$file")"
    tail -n +46 "$(basename "$file")" > ./output/"$(basename "$file")"
done

for file in *brightkite*.csv; do
    echo "$(basename "$file")"
    tail -n +25 "$(basename "$file")" > ./output/"$(basename "$file")"
done

for file in *gowalla*.csv; do
    echo "$(basename "$file")"
    tail -n +25 "$(basename "$file")" > ./output/"$(basename "$file")"
done

for file in *twitter-small*.csv; do
    echo "$(basename "$file")"
    tail -n +22 "$(basename "$file")" > ./output/"$(basename "$file")"
done