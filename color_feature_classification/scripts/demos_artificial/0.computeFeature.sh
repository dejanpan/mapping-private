#!/bin/bash
# Example directory containing .pcd files
DATA=`rospack find color_feature_classification`/demos_artificial
n=0

for i in `find $DATA/data -type f \( -iname "obj_*.pcd" ! -iname "*GRSD_CCHLAC.pcd" ! -iname "*colorCHLAC.pcd" \) | sort -d`
do
    echo "Processing $i"
    num=$(printf "%03d" $n)
    rosrun color_feature_classification computeFeature $i c -rotate 3 -subdiv 7 -offset 2 $DATA/features_c/$num.pcd	
    rosrun color_feature_classification computeFeature $i d -rotate 3 -subdiv 7 -offset 2 $DATA/features_d/$num.pcd	
    rosrun color_feature_classification computeFeature $i g -rotate 3 -subdiv 7 -offset 2 $DATA/features_g/$num.pcd	
    rosrun color_feature_classification computeFeature $i r -rotate 3 -subdiv 7 -offset 2 $DATA/features_r/$num.pcd	
    n=`expr $n + 1`	
done