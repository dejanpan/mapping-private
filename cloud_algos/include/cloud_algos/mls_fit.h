#ifndef CLOUD_ALGOS_PLANAR_ESTIMATION_H
#define CLOUD_ALGOS_PLANAR_ESTIMATION_H
#include <cloud_algos/cloud_algos.h>

// For extra Eigen functions
#include <Eigen/Array>
// if needed:
//#include <Eigen/LU> // matrix inversion
//#include <Eigen/Geometry> // cross product

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

// Sample Consensus
#include <point_cloud_mapping/sample_consensus/sac.h>
#include <point_cloud_mapping/sample_consensus/msac.h>
#include <point_cloud_mapping/sample_consensus/ransac.h>

// Moving Least Squares implementation
class MovingLeastSquares : public CloudAlgo
{
 public:

  // Output type
  typedef sensor_msgs::PointCloud OutputType;

  // Options
  enum GenerationMethod {COPY, FILLING, CLOSE, SURROUNDING, ADAPTIVE, FIXED_K};
  GenerationMethod point_generation_ = COPY; // method for generating the points to be fit to the underlying surface
  enum TangentMethod {PCA, WPCA, IWPCA, SAC};
  TangentMethod approximating_tangent_ = PCA; // method for getting the approximate tangent plane of the underlying surface
  double radius_ = 0.02;         // search radius for getting the nearest neighbors
  double max_nn_ = 30;           // maximum number of nearest neighbors to consider
  double order_ = 3;             // order of the polynomial to be fit
  bool   filter_points_ = false; // should the generated/provided points be filtered based on proximity to the underlying surface?
  bool  polynomial_fit_ = true;  // should the surface+normal be approximated using a polynomial, or do only the tangent estimation?
  bool compute_moments_ = false; // switch to compute moment invariants (j1, j2, j3)
  geometry_msgs::PointStamped viewpoint_cloud_; // viewpoint towards which the normals have to point

  // Topic name
  std::string default_topic_name ()
    {return std::string ("cloud_mls");}

  // Node name
  std::string default_node_name ()
    {return std::string ("mls_fit_node");}

  // Algorithm methods
  void init (ros::NodeHandle&);
  std::vector<std::string> pre  ();
  std::vector<std::string> post ();
  std::string process (sensor_msgs::PointCloudConstPtr);
  OutputType output ();

  // Setter functions - TODO: first test, and if useful request setter for epsion_ !!!
  void setKdTree (cloud_kdtree::KdTree *kdtree)
    {kdtree_ = kdtree;}
  void setCloud2Fit (sensor_msgs::PointCloud *cloud_fit)
    {cloud_fit_ = cloud_fit;}

  // Clearing previously set data
  void clear ()
  {
    kdtree_ = NULL;
    cloud_fit_ = NULL;
  }

  // Constructor-Destructor
  MovingLeastSquares () : CloudAlgo ()
      {clear ();}
  ~MovingLeastSquares ()
  {
    points_indices_.clear ();
    points_sqr_distances_.clear ();
    if (kdtree_ != NULL)
      delete kdtree_;
    if (cloud_fit_ != NULL)
      delete cloud_fit_;
  }

 private:

  // ROS stuff
  ros::NodeHandle nh_;
  ros::Publisher pub_;

  // ROS messages
  sensor_msgs::PointCloud *cloud_fit_;

  // Kd-tree stuff
  cloud_kdtree::KdTree *kdtree_;
  vector<vector<int> > points_indices_;
  vector<vector<float> > points_sqr_distances_;

  // number of coefficients, to be computed from the provided order_
  int nr_coeff_;

  // diagonal weight matrix
  MatrixXf weight_;
  // matrix for the powers of the polynomial representation evaluated for the neighborhood
  MatrixXf P_;
  // vector for storing function values
  VectorXf f_vec_;
  // vector for storing coefficients
  VectorXf c_vec_;
  // matrices for intermediary results
  MatrixXf P_weight_;
  MatrixXf P_weight_Pt_;
  MatrixXf inv_P_weight_Pt_;
  MatrixXf inv_P_weight_Pt_P_weight_;
};

#endif

