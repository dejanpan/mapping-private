#!/bin/bash

# USAGE: bash scripts/detect_test.sh <target obj number> <similarity threshold>
#   e.g. bash scripts/detect_test.sh 19 0.3
DIR=`pwd` #`rospack find color_voxel_recognition`/demos_VOSCH

rank_num=1 #5
exist_voxel_num_threshold=80 #50
r_dim=15 #5
pca=$(echo $DIR/models/$(printf "%03d" $1)/pca_result)
detection_box_size=0.20 # unit: meter
distance_th=1.5

rosrun color_voxel_recognition detectObj $rank_num $exist_voxel_num_threshold $pca $r_dim $detection_box_size $detection_box_size $detection_box_size $2 $distance_th /input:=/camera/depth/points2_throttle
#rosrun color_voxel_recognition detectObj $rank_num $exist_voxel_num_threshold $pca $r_dim $detection_box_size $detection_box_size $detection_box_size $2 $distance_th /input:=/cloud_pcd