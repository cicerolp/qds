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
    FILELIST=$(find ./*${EXECID}*.csv)

    tar -cvzf log_$EXECID.tgz $FILELIST

    cd $DATADIR
    cp $TMPDIR/log_$EXECID.tgz .

    git add log_$EXECID.tgz
    git commit -m "Finish execution $EXECID - Output"

    git add info.org
    # git add -u
    git commit -m "Finish execution $EXECID - Info"

    git push -u origin $expId

    echo $EXECID
}

########################################################################################################################

########################
# test without payload #
########################

# setup benchmark
rm bench_queries
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
rm bench_queries
cd $BUILDIR
rm * -rf
cmake -DCMAKE_BUILD_TYPE="Release" -DENABLE_TIMMING="ON" -DNDS_ENABLE_PAYLOAD="ON" -DENABLE_RAW="OFF" -DENABLE_PDIGEST="ON" -DENABLE_GAUSSIAN="OFF" ../../
cmake --build ./ --target bench_queries -- -j 8
chmod +x ./benchmarks/bench_queries
cp ./benchmarks/bench_queries ../
cd $HOMEDIR

# run test
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_001-tile.log -x ../xml/nc_on_time_performance.xml -d 15 > 
${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_001-tile.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_010-tile.log -x ../xml/nc_on_time_performance.xml -d 15 > 
${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_010-tile.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_100-tile.log -x ../xml/nc_on_time_performance.xml -d 15 > 
${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_100-tile.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_500-tile.log -x ../xml/nc_on_time_performance.xml -d 15 > ${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_500-tile.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_900-tile.log -x ../xml/nc_on_time_performance.xml -d 15 > 
${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_900-tile.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_990-tile.log -x ../xml/nc_on_time_performance.xml -d 15 > 
${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_990-tile.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_999-tile.log -x ../xml/nc_on_time_performance.xml -d 15 > 
${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_999-tile.csv

#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_001-region.log -x ../xml/nc_on_time_performance.xml -d 15 > 
${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_001-region.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_010-region.log -x ../xml/nc_on_time_performance.xml -d 15 > 
${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_010-region.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_100-region.log -x ../xml/nc_on_time_performance.xml -d 15 > 
${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_100-region.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_500-region.log -x ../xml/nc_on_time_performance.xml -d 15 > ${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_500-region.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_900-region.log -x ../xml/nc_on_time_performance.xml -d 15 > 
${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_900-region.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_990-region.log -x ../xml/nc_on_time_performance.xml -d 15 > 
${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_990-region.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_999-region.log -x ../xml/nc_on_time_performance.xml -d 15 > 
${TMPDIR}/pdigest_${EXECID}_nc_on_time_performance-quantile-0_999-region.csv

########################
# test raw             #
########################

# setup benchmark
rm bench_queries
cd $BUILDIR
rm * -rf
cmake -DCMAKE_BUILD_TYPE="Release" -DENABLE_TIMMING="ON" -DNDS_ENABLE_PAYLOAD="ON" -DENABLE_RAW="ON" -DENABLE_PDIGEST="OFF" -DENABLE_GAUSSIAN="OFF" ../../
cmake --build ./ --target bench_queries -- -j 8
chmod +x ./benchmarks/bench_queries
cp ./benchmarks/bench_queries ../
cd $HOMEDIR

# run test
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_001-tile.log -x ../xml/nc_on_time_performance_raw.xml -d 15 > 
${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_001-tile.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_010-tile.log -x ../xml/nc_on_time_performance_raw.xml -d 15 > 
${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_010-tile.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_100-tile.log -x ../xml/nc_on_time_performance_raw.xml -d 15 > 
${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_100-tile.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_500-tile.log -x ../xml/nc_on_time_performance_raw.xml -d 15 > ${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_500-tile.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_900-tile.log -x ../xml/nc_on_time_performance_raw.xml -d 15 > 
${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_900-tile.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_990-tile.log -x ../xml/nc_on_time_performance_raw.xml -d 15 > 
${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_990-tile.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_999-tile.log -x ../xml/nc_on_time_performance_raw.xml -d 15 > 
${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_999-tile.csv

#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_001-region.log -x ../xml/nc_on_time_performance_raw.xml -d 15 
> ${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_001-region.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_010-region.log -x ../xml/nc_on_time_performance_raw.xml -d 15 
> ${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_010-region.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_100-region.log -x ../xml/nc_on_time_performance_raw.xml -d 15 
> ${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_100-region.csv
NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_500-region.log -x ../xml/nc_on_time_performance_raw.xml -d 15 > ${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_500-region.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_900-region.log -x ../xml/nc_on_time_performance_raw.xml -d 15 
> ${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_900-region.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_990-region.log -x ../xml/nc_on_time_performance_raw.xml -d 15 
> ${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_990-region.csv
#NDS_DATA=${NDSDATADIR} ./bench_queries -i ./logs/on_time_performance-quantile-0_999-region.log -x ../xml/nc_on_time_performance_raw.xml -d 15 
> ${TMPDIR}/raw_${EXECID}_nc_on_time_performance-quantile-0_999-region.csv

push_to_git
