#!/bin/bash

# ./script.sh <dataset> <min_year> <max_year> <min_depth> <max_depth> <leaf_size>

# brightkite
./info_checkins.sh brightkite 1 1 22 22 1
./info_checkins.sh brightkite 1 1 22 22 32

# gowalla
./info_checkins.sh gowalla 1 1 22 22 1
./info_checkins.sh gowalla 1 1 22 22 32

# on_time_performance
./info_on_time_performance.sh  on_time_performance 1987 2008 15 15 1
./info_on_time_performance.sh  on_time_performance 1987 2008 15 15 32

# twitter
./info_twitter.sh twitter 1 1 22 22 1
./info_twitter.sh twitter 1 1 22 22 64

# twitter-small
./info_twitter-small.sh twitter 1 1 22 22 1
./info_twitter-small.sh twitter 1 1 22 22 64

# tripdata
./info_tripdata.sh green_tripdata 2013 2016 22 22 1
./info_tripdata.sh green_tripdata 2013 2016 22 22 64

#./info_tripdata.sh yellow_tripdata 2010 2014 22 22 1
#./info_tripdata.sh yellow_tripdata 2010 2014 22 22 64

# tripdata-small
./info_tripdata_small.sh green_tripdata 2013 2016 22 22 1
./info_tripdata_small.sh green_tripdata 2013 2016 22 22 64

#./info_tripdata_small.sh yellow_tripdata 2010 2014 22 22 1
#./info_tripdata_small.sh yellow_tripdata 2010 2014 22 22 64