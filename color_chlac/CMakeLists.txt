cmake_minimum_required(VERSION 2.4.6)
include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)

# Set the build type.  Options are:
#  Coverage       : w/ debug symbols, w/o optimization, w/ code-coverage
#  Debug          : w/ debug symbols, w/o optimization
#  Release        : w/o debug symbols, w/ optimization
#  RelWithDebInfo : w/ debug symbols, w/ optimization
#  MinSizeRel     : w/o debug symbols, w/ optimization, stripped binaries
#set(ROS_BUILD_TYPE RelWithDebInfo)

rosbuild_init()

#set the default path for built executables to the "bin" directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#set the default path for built libraries to the "lib" directory
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#uncomment if you have defined messages
#rosbuild_genmsg()
#uncomment if you have defined services
#rosbuild_gensrv()

#common commands for building c++ executables and libraries
#rosbuild_add_boost_directories()
#rosbuild_link_boost(${PROJECT_NAME} thread)
rosbuild_add_executable(test_color_chlac test/example_colorCHLAC.cpp)
rosbuild_add_executable(test_color_chlac_rand test/example_colorCHLAC_rand.cpp)
rosbuild_add_executable(exampleGRSD test/exampleGRSD.cpp)
#rosbuild_add_executable(exampleRSD test/exampleRSD.cpp)
rosbuild_add_executable(exampleVOSCH test/exampleVOSCH.cpp)
rosbuild_add_executable(test_GRSD_CCHLAC test/example_GRSD_CCHLAC.cpp)
rosbuild_add_executable(test_GRSD_CCHLAC_rand test/example_GRSD_CCHLAC_rand.cpp)
rosbuild_add_executable(convertOld2NewPCD test/convertOld2NewPCD.cpp)
#rosbuild_add_executable(hoge gomi/hoge.cpp)
