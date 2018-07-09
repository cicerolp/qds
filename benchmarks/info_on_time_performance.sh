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

payload_map[arr_delay]=46
payload_map[dep_delay]=38

MIN_DEPTH=$4
MAX_DEPTH=$5

MIN_YEAR=$2
MAX_YEAR=$3

NDS_FILE=$1

LEAF_SIZE=$6

NDS_SCHEMA_MEMORY="<categorical>
			<index>dep_on_time</index>
			<bin>9</bin>
			<offset>2</offset>
		</categorical>		
		<categorical>
			<index>unique_carrier</index>
			<bin>1639</bin>
			<offset>8</offset>
		</categorical>
		<temporal>
			<index>crs_dep_time</index>
			<bin>14400</bin>
			<offset>14</offset>
		</temporal>
		<spatial>
			<index>origin_airport</index>
			<bin>${LEAF_SIZE}</bin>
			<offset>22</offset>
		</spatial>"

NDS_SCHEMA_ACCURACY='<spatial>
            <index>origin_airport</index>
            <bin>1</bin>
            <offset>22</offset>
        </spatial>'

NDS_SCHEMA_ACCURACY_GROUP="origin_airport"

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
    tar -cvzf log_$EXECID.tgz *_$EXECID_*.json

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
    for year_index in $(seq $MAX_YEAR $MAX_YEAR); do
        # collect years
        years=()
        for year in $(seq $MIN_YEAR $year_index); do
            years+=("$year")
        done

        # write WITHOUT payload
        echo "" > input.xml

        echo '<?xml version="1.0" encoding="utf-8"?>
            <config>
            <name>'${NDS_FILE}'</name>' >> input.xml

        echo '<files>' >> input.xml

        for var in "${years[@]}"; do
            echo '<file>'${NDS_FILE}_"${var}"'.nds</file>' >> input.xml
        done

        echo '</files>
            <schema>' $NDS_SCHEMA_MEMORY >> input.xml
        echo '</schema></config>' >> input.xml

        # run test
        NDS_DATA=${NDSDATADIR} ./bench_info --no-log -x input.xml -d ${depth} > ${TMPDIR}/${NDS_FILE}_-1_${depth}_${year_index}_-1_${LEAF_SIZE}_${EXECID}_MEMORY.json        
    done
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
    for year_index in $(seq $MAX_YEAR $MAX_YEAR); do
        # collect years
        years=()
        for year in $(seq $MIN_YEAR $year_index); do
            years+=("$year")
        done

        # write WITHOUT payload
        echo "" > input.xml

        echo '<?xml version="1.0" encoding="utf-8"?>
            <config>
            <name>'${NDS_FILE}'</name>' >> input.xml

        echo '<files>' >> input.xml

        for var in "${years[@]}"; do
            echo '<file>'${NDS_FILE}_"${var}"'.nds</file>' >> input.xml
        done

        echo '</files>
            <schema>' $NDS_SCHEMA_MEMORY >> input.xml
        echo '</schema></config>' >> input.xml

        # run test
        NDS_DATA=${NDSDATADIR} ./bench_info --no-log -x input.xml -d ${depth} > ${TMPDIR}/${NDS_FILE}_-1_${depth}_${year_index}_-1_${LEAF_SIZE}_${EXECID}_SHARING_MEMORY.json
    done
done

########################################################################################################################
# PAYLOAD TEST
########################################################################################################################

#################
# test WITH raw #
#################

for compression in 0; do
    # setup benchmark
    cd $BUILDIR
    rm * -rf
    cmake -DCMAKE_BUILD_TYPE="Release" -DENABLE_METRICS="ON" -DNDS_ENABLE_PAYLOAD="ON" -DENABLE_PDIGEST="OFF" -DENABLE_RAW="ON" ../../
    cmake --build ./ --target bench_info -- -j 8
    chmod +x ./benchmarks/bench_info
    cp ./benchmarks/bench_info ../
    cd $HOMEDIR

    for depth in $(seq $MAX_DEPTH $MAX_DEPTH); do

        for year_index in $(seq $MAX_YEAR $MAX_YEAR); do
            # collect years
            years=()
            for year in $(seq $MIN_YEAR $year_index); do
                years+=("$year")
            done

            # write WITH payload
            for payload_index in $(seq 0 $NUM_PAYLOAD); do
                curr_payload=0
                args=()
                for i in "${!payload_map[@]}"; do
                    args+=("$i")
                    if [ $curr_payload == $payload_index ]; then
                        break
                    fi
                    curr_payload=$(($curr_payload + 1))
                done

                # write xml
                echo "" > input.xml

                echo '<?xml version="1.0" encoding="utf-8"?>
                    <config>
                    <name>'${NDS_FILE}'</name>' >> input.xml

                echo '<files>' >> input.xml

                for var in "${years[@]}"; do
                  echo '<file>'${NDS_FILE}_"${var}"'.nds</file>' >> input.xml
                done

                echo '</files>
                    <schema>' $NDS_SCHEMA_MEMORY >> input.xml

                for var in "${args[@]}"; do
                  echo '<payload>
                    <index>'"$var"'</index>
                    <bin>0</bin>
                    <offset>'${payload_map[$var]}'</offset>
                    </payload>' >> input.xml
                done
                echo '</schema></config>' >> input.xml

                # run test
                NDS_DATA=${NDSDATADIR} ./bench_info --no-log -x input.xml -d ${depth} > ${TMPDIR}/${NDS_FILE}_${compression}_${depth}_${year_index}_${payload_index}_${LEAF_SIZE}_${EXECID}_PDIGEST_PAYLOAD.json
            done
        done
    done
done

#####################
# test WITH pdigest #
#####################

for compression in 25 50 100; do
    # setup benchmark
    cd $BUILDIR
    rm * -rf
    cmake -DCMAKE_BUILD_TYPE="Release" -DENABLE_METRICS="ON" -DNDS_ENABLE_PAYLOAD="ON" -DENABLE_PDIGEST="ON" -DPDIGEST_COMPRESSION="${compression}.f" ../../
    cmake --build ./ --target bench_info -- -j 8
    chmod +x ./benchmarks/bench_info
    cp ./benchmarks/bench_info ../
    cd $HOMEDIR

    for depth in $(seq $MAX_DEPTH $MAX_DEPTH); do

        for year_index in $(seq $MAX_YEAR $MAX_YEAR); do
            # collect years
            years=()
            for year in $(seq $MIN_YEAR $year_index); do
                years+=("$year")
            done

            # write WITH payload
            for payload_index in $(seq 0 $NUM_PAYLOAD); do
                curr_payload=0
                args=()
                for i in "${!payload_map[@]}"; do
                    args+=("$i")
                    if [ $curr_payload == $payload_index ]; then
                        break
                    fi
                    curr_payload=$(($curr_payload + 1))
                done

                # write xml
                echo "" > input.xml

                echo '<?xml version="1.0" encoding="utf-8"?>
                    <config>
                    <name>'${NDS_FILE}'</name>' >> input.xml

                echo '<files>' >> input.xml

                for var in "${years[@]}"; do
                  echo '<file>'${NDS_FILE}_"${var}"'.nds</file>' >> input.xml
                done

                echo '</files>
                    <schema>' $NDS_SCHEMA_MEMORY >> input.xml

                for var in "${args[@]}"; do
                  echo '<payload>
                    <index>'"$var"'</index>
                    <bin>0</bin>
                    <offset>'${payload_map[$var]}'</offset>
                    </payload>' >> input.xml
                done
                echo '</schema></config>' >> input.xml

                # run test
                NDS_DATA=${NDSDATADIR} ./bench_info --no-log -x input.xml -d ${depth} > ${TMPDIR}/${NDS_FILE}_${compression}_${depth}_${year_index}_${payload_index}_${LEAF_SIZE}_${EXECID}_PDIGEST_PAYLOAD.json
            done
        done
    done
done

push_to_git