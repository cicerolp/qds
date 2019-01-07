#!/bin/bash

########################################################################################################################
### SETUP THIS VARIABLES

# get from date +%Y%m%d-%H%M%S
expId="exp-queries-db-20181207-142310"

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
    git add -u
    git commit -m "Finish execution $EXECID - Info"

    git push -u origin $expId

    echo $EXECID
}

########################################################################################################################

#############
# raw       #
#############

rm bench_queries_db
cd $BUILDIR
rm * -rf
cmake -DCMAKE_BUILD_TYPE="Release" -DENABLE_METRICS=TRUE ../../
cmake --build ./ --target bench_queries_db -- -j 8
chmod +x ./benchmarks/bench_queries_db
cp ./benchmarks/bench_queries_db ../
cd $HOMEDIR

## brightkite
# ./bench_queries_db -s snap -i /home/cicerolp/Git/nds-data/logs-nds/brightkite-count-region.log -d /home/cicerolp/data/brightkite/csv/data.csv > ${TMPDIR}/db_${EXECID}_brightkite-count-region.csv

## gowalla
# ./bench_queries_db -s snap -i /home/cicerolp/Git/nds-data/logs-nds/gowalla-count-region.log -d /home/cicerolp/data/gowalla/csv/data.csv > ${TMPDIR}/db_${EXECID}_gowalla-count-region.csv

## on-time
# ./bench_queries_db -s on-time -i /home/cicerolp/Git/nds-data/logs-nds/on_time_performance-count-region.log -d /home/cicerolp/data/flights/csv/data.csv > ${TMPDIR}/db_${EXECID}_on_time_performance-count-region.csv

## twitter
./bench_queries_db -s twitter -i /home/cicerolp/Git/nds-data/logs-nds/twitter-small-count-region.log -d /home/cicerolp/data/twitter/csv/data.csv > ${TMPDIR}/db_${EXECID}_twitter-small-count-region.csv

## percitile gaussian
#./bench_queries_db -s gaussian -i /home/cicerolp/Git/nds-data/logs-nds/uniform-lhs-region.log -d /run/media/cicerolp/Misc/Data/uniform_50M/csv/data.csv > ${TMPDIR}/db_${EXECID}_uniform-quantile-0_500-region.csv

push_to_git
