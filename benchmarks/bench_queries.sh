#!/bin/bash

########################################################################################################################
### SETUP THIS VARIABLES

# get from date +%Y%m%d-%H%M%S
expId="exp-queries-20180708-180023"

HOMEDIR=$(pwd)
DATADIR=$(pwd)
BUILDIR=$(pwd)/build-release
NDSDATADIR=/home/cicerolp/git/nds-data

TMPDIR=/tmp/$expId

########################################################################################################################

set +e

mkdir -p $TMPDIR

# get machine configuration
echo "" > $DATADIR/info.org
./g5k_get_info.sh $DATADIR/info.org

# EXECUTE BENCHMARK
set +e

# cd to home dir
cd $HOMEDIR

# make build dir
mkdir -p $BUILDIR

# generate output name
EXECID=$(date +%Y%m%d-%H%M%S)

########################################################################################################################

function push_to_git {
    cd $TMPDIR
    tar -cvzf log_$EXECID.tgz *_$EXECID_*.csv

    cd $DATADIR
    cp $TMPDIR/log_$EXECID.tgz .

    git add log_$EXECID.tgz
    git commit -m "Finish execution $EXECID - Output"

    git add info.org run.sh
    git add -u
    git commit -m "Finish execution $EXECID - Info"

    git push -u origin $expId

    echo $EXECID
}

########################################################################################################################

########################
# test without payload #
########################

# setup benchmark
cd $BUILDIR
rm * -rf
cmake -DCMAKE_BUILD_TYPE="Release" -DENABLE_TIMMING="ON" -DNDS_ENABLE_PAYLOAD="OFF" ../../
cmake --build ./ --target bench_queries -- -j 8
chmod +x ./benchmarks/bench_queries
cp ./benchmarks/bench_queries ../
cd $HOMEDIR

# run test
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/gowalla-count.log                      -x ../xml/nc_gowalla.xml             -d 22 > ${TMPDIR}/disabled_${EXECID}_nc_gowalla-count.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/brightkite-count.log                   -x ../xml/nc_brightkite.xml          -d 22 > ${TMPDIR}/disabled_${EXECID}_nc_brightkite-count.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/twitter-small-count.log                -x ../xml/nc_twitter-small.xml       -d 22 > ${TMPDIR}/disabled_${EXECID}_nc_twitter-small-count.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-count.log          -x ../xml/nc_on_time_performance.xml -d 15 > ${TMPDIR}/disabled_${EXECID}_nc_on_time_performance-count.csv


########################
# test p-digest        #
########################

# setup benchmark
cd $BUILDIR
rm * -rf
cmake -DCMAKE_BUILD_TYPE="Release" -DENABLE_TIMMING="ON" -DNDS_ENABLE_PAYLOAD="ON" -DENABLE_RAW="OFF" -DENABLE_PDIGEST="ON" -DENABLE_GAUSSIAN="OFF" ../../
cmake --build ./ --target bench_queries -- -j 8
chmod +x ./benchmarks/bench_queries
cp ./benchmarks/bench_queries ../
cd $HOMEDIR

# run test
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_001.log -x ../xml/nc_on_time_performance.xml -d 15 > ${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_001.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_010.log -x ../xml/nc_on_time_performance.xml -d 15 > ${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_010.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_100.log -x ../xml/nc_on_time_performance.xml -d 15 > ${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_100.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_500.log -x ../xml/nc_on_time_performance.xml -d 15 > ${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_500.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_900.log -x ../xml/nc_on_time_performance.xml -d 15 > ${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_900.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_990.log -x ../xml/nc_on_time_performance.xml -d 15 > ${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_990.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_999.log -x ../xml/nc_on_time_performance.xml -d 15 > ${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_999.csv

########################
# test raw             #
########################

# setup benchmark
cd $BUILDIR
rm * -rf
cmake -DCMAKE_BUILD_TYPE="Release" -DENABLE_TIMMING="ON" -DNDS_ENABLE_PAYLOAD="ON" -DENABLE_RAW="ON" -DENABLE_PDIGEST="OFF" -DENABLE_GAUSSIAN="OFF" ../../
cmake --build ./ --target bench_queries -- -j 8
chmod +x ./benchmarks/bench_queries
cp ./benchmarks/bench_queries ../
cd $HOMEDIR

# run test
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_001.log -x ../xml/nc_on_time_performance.xml -d 15 > ${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_001.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_010.log -x ../xml/nc_on_time_performance.xml -d 15 > ${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_010.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_100.log -x ../xml/nc_on_time_performance.xml -d 15 > ${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_100.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_500.log -x ../xml/nc_on_time_performance.xml -d 15 > ${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_500.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_900.log -x ../xml/nc_on_time_performance.xml -d 15 > ${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_900.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_990.log -x ../xml/nc_on_time_performance.xml -d 15 > ${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_990.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_999.log -x ../xml/nc_on_time_performance.xml -d 15 > ${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_999.csv

push_to_git
