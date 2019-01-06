#!/bin/bash

# QDS and Naive
git checkout exp-synthetic-20180719-095432

./bench_synthetic.sh

# Databases
git checkout exp-queries-db-20181207-142310

./bench_queries_db.sh