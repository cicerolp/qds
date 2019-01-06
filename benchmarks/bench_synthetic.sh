#!/bin/bash

########################################################################################################################
### SETUP THIS VARIABLES

# get from date +%Y%m%d-%H%M%S
expId="exp-synthetic-20180719-095432"

HOMEDIR=$(pwd)
DATADIR=$(pwd)
BUILDIR=$(pwd)/build-release
NDSDATADIR=/home/cicerolp/Git/nds-data

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
# test raw             #
########################

# setup benchmark
rm bench_queries
cd $BUILDIR
rm -rf *
cmake -DCMAKE_BUILD_TYPE="Release" -DNDS_OPTIMIZE_LEAF="OFF" -DENABLE_METRICS="ON" -DNDS_ENABLE_PAYLOAD="ON" -DENABLE_RAW="ON" -DENABLE_PDIGEST="OFF" -DENABLE_GAUSSIAN="OFF" -DNDS_ENABLE_CRS_SIMPLE="ON" ../../
cmake --build ./ --target bench_queries -- -j 8
chmod +x ./benchmarks/bench_queries
cp ./benchmarks/bench_queries ../
cd $HOMEDIR

# run test
NDS_DATA=${NDSDATADIR} ./bench_queries -i /home/cicerolp/Git/nds-data/logs-nds/uniform-lhs-region.log -x /home/cicerolp/Git/nds-data/uniform_50M.xml -d 9 > ${TMPDIR}/raw_${EXECID}_uniform_50M.csv

########################
# test p-digest        #
########################

# setup benchmark
rm bench_queries
cd $BUILDIR
rm -rf *
cmake -DCMAKE_BUILD_TYPE="Release" -DNDS_OPTIMIZE_LEAF="OFF" -DENABLE_METRICS="ON" -DNDS_ENABLE_PAYLOAD="ON" -DENABLE_RAW="OFF" -DENABLE_PDIGEST="ON" -DENABLE_GAUSSIAN="OFF" -DNDS_ENABLE_CRS_SIMPLE="ON" ../../
cmake --build ./ --target bench_queries -- -j 8
chmod +x ./benchmarks/bench_queries
cp ./benchmarks/bench_queries ../
cd $HOMEDIR

# run test
NDS_DATA=${NDSDATADIR} ./bench_queries -i /home/cicerolp/Git/nds-data/logs-nds/uniform-lhs-region.log -x /home/cicerolp/Git/nds-data/uniform_50M.xml -d 9 > ${TMPDIR}/pdigest_${EXECID}_uniform_50M.csv

push_to_git