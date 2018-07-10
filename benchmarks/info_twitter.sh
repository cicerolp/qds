#!/bin/bash

## USAGE
# ./script.sh <dataset> <min_year> <max_year> <min_depth> <max_depth> <leaf_size>

########################################################################################################################
### SETUP THIS VARIABLES

# get from date +%Y%m%d-%H%M%S
expId="exp-info-20180709-142751"

HOMEDIR=$(pwd)
DATADIR=$(pwd)
BUILDIR=$(pwd)/build-release
NDSDATADIR=/home/cicerolp/git/nds-data

TMPDIR=/tmp/$expId

# [index] -> offset
declare -A payload_map

MIN_DEPTH=$4
MAX_DEPTH=$5

MIN_YEAR=$2
MAX_YEAR=$3

NDS_FILE=$1

LEAF_SIZE=$6

NDS_SCHEMA_MEMORY="<categorical>
			<index>app</index>
			<bin>4</bin>
			<offset>0</offset>
		</categorical>
		<categorical>
			<index>device</index>
			<bin>5</bin>
			<offset>2</offset>
		</categorical>
		<categorical>
			<index>language</index>
			<bin>15</bin>
			<offset>4</offset>
		</categorical>
		<temporal>
			<index>time</index>
			<bin>14400</bin>
			<offset>6</offset>
		</temporal>
		<spatial>
			<index>coord</index>
			<bin>${LEAF_SIZE}</bin>
			<offset>10</offset>
		</spatial>"

NDS_SCHEMA_ACCURACY="<spatial>
			<index>coord</index>
			<bin>1</bin>
			<offset>10</offset>
		</spatial>"

NDS_SCHEMA_ACCURACY_GROUP="coord"

########################################################################################################################

NUM_PAYLOAD=$((${#payload_map[@]} - 1))

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
    FILELIST=$(find ./*${EXECID}*.json)

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
# MEMORY TEST
########################################################################################################################

##################################################
# test WITHOUT payload and WITHOUT pivot sharing #
##################################################

# setup benchmark
cd $BUILDIR
rm * -rf
cmake -DCMAKE_BUILD_TYPE="Release" -DENABLE_METRICS="ON" -DNDS_ENABLE_PAYLOAD="OFF" -DNDS_SHARE_PIVOT="OFF" ../../
cmake --build ./ --target bench_info -- -j 8
chmod +x ./benchmarks/bench_info
cp ./benchmarks/bench_info ../
cd $HOMEDIR

for depth in $(seq $MAX_DEPTH $MAX_DEPTH); do
    # write WITHOUT payload
    echo "" > input.xml

    echo '<?xml version="1.0" encoding="utf-8"?>
        <config>
        <name>'${NDS_FILE}'</name>' >> input.xml

    echo '<files>' >> input.xml

    echo '<file>'${NDS_FILE}'.nds</file>' >> input.xml

    echo '</files>
        <schema>' $NDS_SCHEMA_MEMORY >> input.xml

    echo '</schema></config>' >> input.xml

    # run test
    NDS_DATA=${NDSDATADIR} ./bench_info --no-log -x input.xml -d ${depth} > ${TMPDIR}/${NDS_FILE}_${EXECID}_-1_${depth}_-1_-1_${LEAF_SIZE}_MEMORY.json
done

###############################################
# test WITHOUT payload and WITH pivot sharing #
###############################################

# setup benchmark
cd $BUILDIR
rm * -rf
cmake -DCMAKE_BUILD_TYPE="Release" -DENABLE_METRICS="ON" -DNDS_ENABLE_PAYLOAD="OFF" -DNDS_SHARE_PIVOT="ON" ../../
cmake --build ./ --target bench_info -- -j 8
chmod +x ./benchmarks/bench_info
cp ./benchmarks/bench_info ../
cd $HOMEDIR

for depth in $(seq $MAX_DEPTH $MAX_DEPTH); do
    # write WITHOUT payload
    echo "" > input.xml

    echo '<?xml version="1.0" encoding="utf-8"?>
        <config>
        <name>'${NDS_FILE}'</name>' >> input.xml

    echo '<files>' >> input.xml

    echo '<file>'${NDS_FILE}'.nds</file>' >> input.xml

    echo '</files>
        <schema>' $NDS_SCHEMA_MEMORY >> input.xml

    echo '</schema></config>' >> input.xml

    # run test
    NDS_DATA=${NDSDATADIR} ./bench_info --no-log -x input.xml -d ${depth} > ${TMPDIR}/${NDS_FILE}_${EXECID}_-1_${depth}_-1_-1_${LEAF_SIZE}_SHARING_MEMORY.json
done

push_to_git