#!/bin/bash
# Example directory containing _vfh.pcd files
DATA=`rospack find color_feature_classification`/demos
#n=0

# compute a subspace
dirNum=`ls $DATA/features_c/* -d | wc -l`
for((i=0;i<$dirNum;i++))
do
    num=$(printf "%03d" $i)
    dir_name=$(printf "obj%03d" $i)
    echo $dir_name
    files=`find $DATA/features_c/$dir_name -type f -iname "*.pcd" | sort -d`
    rosrun color_feature_classification computeSubspace_from_file $files -dim 100 -comp $DATA/pca_result_c/compress_axis $DATA/pca_result_c/$num
done
#
dirNum=`ls $DATA/features_d/* -d | wc -l`
for((i=0;i<$dirNum;i++))
do
    num=$(printf "%03d" $i)
    dir_name=$(printf "obj%03d" $i)
    echo $dir_name
    files=`find $DATA/features_d/$dir_name -type f -iname "*.pcd" | sort -d`
    rosrun color_feature_classification computeSubspace_from_file $files -dim 100 -comp $DATA/pca_result_d/compress_axis $DATA/pca_result_d/$num
done
#
dirNum=`ls $DATA/features_g/* -d | wc -l`
for((i=0;i<$dirNum;i++))
do
    num=$(printf "%03d" $i)
    dir_name=$(printf "obj%03d" $i)
    echo $dir_name
    files=`find $DATA/features_g/$dir_name -type f -iname "*.pcd" | sort -d`
    rosrun color_feature_classification computeSubspace_from_file $files $DATA/pca_result_g/$num
done
#
dirNum=`ls $DATA/features_r/* -d | wc -l`
for((i=0;i<$dirNum;i++))
do
    num=$(printf "%03d" $i)
    dir_name=$(printf "obj%03d" $i)
    echo $dir_name
    files=`find $DATA/features_r/$dir_name -type f -iname "*.pcd" | sort -d`
    rosrun color_feature_classification computeSubspace_from_file $files -dim 80 -comp $DATA/pca_result_r/compress_axis $DATA/pca_result_r/$num
done

##############################################
# # compute a subspace
# dirNum=`ls data/* -d | wc -l`
# for((i=0;i<$dirNum;i++))
# do
#     num=$(printf "%03d" $i)
#     dir_name=$(printf "obj%03d" $i)
#     #echo $dir_name
#     files=`find $DATA/$dir_name -type f \( -iname "*.pcd" ! -iname "*vfh*" ! -iname "*colorCHLAC*" \) | sort -R`
#     rosrun color_feature_classification computeSubspace_with_rotate c $files -dim 800 -comp pca_result_c/compress_axis -rotate 1 -subdiv 10 -offset 5 pca_result_c/$num
#     rosrun color_feature_classification computeSubspace_with_rotate d $files -dim 800 -comp pca_result_d/compress_axis -rotate 1 -subdiv 10 -offset 5 pca_result_d/$num
#     rosrun color_feature_classification computeSubspace g $files -subdiv 10 -offset 5 pca_result_g/$num
#     rosrun color_feature_classification computeSubspace r $files -subdiv 10 -offset 5 pca_result_r/$num
#     ### rosrun color_feature_classification computeSubspace r $files -dim 80 -comp pca_result_r/compress_axis pca_result_r/$num
# done

###############################################
# for i in `find $DATA -type d -name "*"`
# do
#     if [ $DATA != $i ]
#     then
# 	echo $i
# 	num=$(printf "%03d" $n)
# 	files=`find $i -type f \( -iname "*.pcd" ! -iname "*vfh*" ! -iname "*colorCHLAC*" \) | sort -R`

# 	rosrun color_feature_classification computeSubspace_with_rotate c $files -dim 800 -comp pca_result_c/compress_axis -rotate 1 pca_result_c/$num
# 	rosrun color_feature_classification computeSubspace_with_rotate d $files -dim 800 -comp pca_result_d/compress_axis -rotate 1 pca_result_d/$num
# 	rosrun color_feature_classification computeSubspace g $files pca_result_g/$num
# 	rosrun color_feature_classification computeSubspace r $files pca_result_r/$num
# 	#rosrun color_feature_classification computeSubspace r $files -dim 50 -comp pca_result_r/compress_axis pca_result_r/$num
	
# 	n=`expr $n + 1`
#     fi
# done