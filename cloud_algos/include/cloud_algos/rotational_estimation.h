#ifndef CLOUD_ALGOS_ROTATIONAL_ESTIMATION_H
#define CLOUD_ALGOS_ROTATIONAL_ESTIMATION_H
#include <cloud_algos/cloud_algos.h>
#include <mapping_msgs/PolygonalMap.h>
#include <ias_visualization_msgs/PositionStringList.h>
#include <ias_table_msgs/TriangularMesh.h>
#include <point_cloud_mapping/geometry/point.h>

namespace cloud_algos
{

class RotationalEstimation : public CloudAlgo
{
 public:
  RotationalEstimation () {};
  typedef ias_table_msgs::TriangularMesh OutputType;
  typedef sensor_msgs::PointCloud InputType;

  static std::string default_input_topic ()
    {return std::string ("cloud_pcd");}

  static std::string default_output_topic ()
    {return std::string ("cloud_normals");};

  static std::string default_node_name () 
    {return std::string ("rotational_estimation_node");};

  void init (ros::NodeHandle&);
  void pre  ();
  void post ();
  std::vector<std::string> requires ();
  std::vector<std::string> provides ();
  std::string process (const boost::shared_ptr<const InputType>);
  boost::shared_ptr<OutputType> output ();
  boost::shared_ptr<sensor_msgs::PointCloud> getInliers ();
  boost::shared_ptr<sensor_msgs::PointCloud> getOutliers ();
 
 private: 
  boost::shared_ptr<mapping_msgs::PolygonalMap> vis_pmap_;
  boost::shared_ptr<sensor_msgs::PointCloud> vis_cloud_;
  boost::shared_ptr<ias_visualization_msgs::PositionStringList> vis_text_;
  
  boost::shared_ptr<OutputType> mesh_;
  boost::shared_ptr<const sensor_msgs::PointCloud> cloud_;
  std::vector<int> inliers_;
  std::vector<int> outliers_;

  ros::NodeHandle nh_;
  ros::Publisher vis_cloud_pub_;
  ros::Publisher vis_pmap_pub_;
  ros::Publisher vis_text_pub_;
};

}
#endif

