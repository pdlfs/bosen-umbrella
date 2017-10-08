#!/bin/bash -u

# Unpack data
cd ${umbrella_dir}/misc && gunzip -k newratings.dat.gz || exit 1

data_file_path=${umbrella_dir}/misc/newratings.dat
output_file_path=${umbrella_dir}/misc/newratings.out
num_partitions=1
data_format=list

GLOG_logtostderr=true \
    GLOG_v=-1 \
    GLOG_minloglevel=0 \
    GLOG_vmodule="" \
    ${bosen_dir}/apps/matrixfact/bin/data_split \
    --outputfile ${output_file_path} \
    --datafile ${data_file_path} \
    --num_partitions ${num_partitions} \
    --data_format ${data_format}
