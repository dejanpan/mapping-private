/*
 * Copyright (c) 2009 Technische Universitaet Muenchen
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: cluster_voxelization.cpp 17089 2009-06-15 18:52:12Z veedee $
 *
 */

/**
@mainpage

@htmlinclude manifest.html

 **/

// ROS core
#include <ros/node.h>
#include <ros/node_handle.h>
// ROS messages
#include <robot_msgs/PointCloud.h>
#include <robot_msgs/PolygonalMap.h>

// Sample Consensus
#include <point_cloud_mapping/sample_consensus/sac.h>
#include <point_cloud_mapping/sample_consensus/msac.h>
#include <point_cloud_mapping/sample_consensus/ransac.h>
#include <point_cloud_mapping/sample_consensus/sac_model_plane.h>

#include <angles/angles.h>

// Kd Tree
#include <point_cloud_mapping/kdtree/kdtree_ann.h>

// Cloud geometry
#include <point_cloud_mapping/geometry/angles.h>
#include <point_cloud_mapping/geometry/areas.h>
#include <point_cloud_mapping/geometry/point.h>
#include <point_cloud_mapping/geometry/distances.h>
#include <point_cloud_mapping/geometry/nearest.h>
#include <point_cloud_mapping/geometry/transforms.h>
#include <point_cloud_mapping/geometry/statistics.h>

#include <sys/time.h>

#include <perception_srvs/ClustersVoxels.h>

using namespace std;
using namespace ros;
using namespace std_msgs;
using namespace robot_msgs;
using namespace perception_srvs;

class ClusterVoxelization
{
  protected:
    ros::NodeHandle nh_;
  public:

    // ROS messages
    PointCloudConstPtr cloud_in_;

    Point32 axis_;
    bool need_cloud_data_;

    // Parameters
    Point leaf_width_;
    string input_cloud_topic_;
    int k_;
    int clusters_min_pts_;

    int object_cluster_min_pts_;
    double object_cluster_tolerance_;
    double sac_distance_threshold_, eps_angle_;

    Subscriber cloud_sub_;
    int downsample_factor_;
    ServiceServer clusters_service_;
//     Publisher cloud_table_pub_, cloud_clusters_pub_, pmap_pub_;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ClusterVoxelization ()
    {
      axis_.x = 0; axis_.y = 0; axis_.z = 1;
      nh_.param ("~downsample_leaf_width_x", leaf_width_.x, 0.03);          // 3cm radius by default
      nh_.param ("~downsample_leaf_width_y", leaf_width_.y, 0.03);          // 3cm radius by default
      nh_.param ("~downsample_leaf_width_z", leaf_width_.z, 0.03);          // 3cm radius by default
      nh_.param ("~search_k_closest", k_, 10);                 // 10 k-neighbors by default
      nh_.param ("~normal_eps_angle", eps_angle_, 15.0);       // 15 degrees
      eps_angle_ = angles::from_degrees (eps_angle_);          // convert to radians

      nh_.param ("~object_cluster_tolerance", object_cluster_tolerance_, 0.04);   // 4cm between two objects
      nh_.param ("~object_cluster_min_pts", object_cluster_min_pts_, 30);         // 30 points per object cluster

      nh_.param ("~input_cloud_topic", input_cloud_topic_, string ("/cloud_pcd"));
      clusters_service_ = nh_.advertiseService ("clusters_service", &ClusterVoxelization::cluster_voxelization_service, this);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   void
      updateParametersFromServer ()
    {
      nh_.getParam ("~input_cloud_topic", input_cloud_topic_);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool
      cluster_voxelization_service (ClustersVoxels::Request &req, ClustersVoxels::Response &resp)
    {
      ROS_INFO ("Service request initiated.");
      updateParametersFromServer ();

      // Subscribe to a point cloud topic
//       need_cloud_data_ = true;
//       cloud_sub_ = nh_.subscribe (input_cloud_topic_, 1, &ClusterVoxelization::cloud_cb, this);

      // Wait until the scan is ready, sleep for 100ms
      ros::Duration tictoc (0, 10000000);
      while (need_cloud_data_)
      {
        //tictoc.sleep ();
        ros::spinOnce ();
      }

//       detectTable (*cloud_in_, resp);

      ROS_INFO ("Service request terminated.");
      return (true);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PointCloud message callback
    void
      cloud_cb (const PointCloudConstPtr& cloud)
    {
      if (!need_cloud_data_)
        return;

      ROS_INFO ("PointCloud message received on %s", input_cloud_topic_.c_str ());
      cloud_in_ = cloud;
      need_cloud_data_ = false;
    }
};

/* ---[ */
int
  main (int argc, char** argv)
{
  ros::init (argc, argv, "cluster_voxelization");

  ClusterVoxelization p;
  ros::spin ();

  return (0);
}
/* ]--- */
