#include <sys/time.h>
#include <ros/ros.h>
#include <pcl/point_types.h>
#include <pcl/features/feature.h>
#include <pcl/io/pcd_io.h>
#include "pcl/filters/voxel_grid.h"
#include "pcl/features/rsd.h"
#include <algorithm>
#include <pcl_cloud_algos/pcl_cloud_algos_point_types.h>

typedef pcl::KdTree<pcl::PointNormal>::Ptr KdTreePtr;

//* time
double t1,t2;
double my_clock()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

#define NR_CLASS 5 
#define NOISE 0 
#define PLANE 1 
#define CYLINDER 2
#define CIRCLE 3  
#define EDGE 4 
#define EMPTY 5 

double min_radius_plane_;
double min_radius_edge_;
double min_radius_noise_, max_radius_noise_;
double max_min_radius_diff_;

int get_type (float min_radius, float max_radius)
{
  if (min_radius > min_radius_plane_) // 0.066
    return PLANE; // plane
  else if ((min_radius < min_radius_noise_) && (max_radius < max_radius_noise_))
    return NOISE; // noise/corner
  else if (max_radius - min_radius < max_min_radius_diff_) // 0.0075
    return CIRCLE; // circle (corner?)
  else if (min_radius < min_radius_edge_) /// considering small cylinders to be edges
    return EDGE; // edge
  else
    return CYLINDER; // cylinder (rim)
}

int main( int argc, char** argv ){
  if( argc != 2 ){
    ROS_ERROR ("Need one parameter! Syntax is: %s {input_pointcloud_filename.pcd}\n", argv[0]);
    return(-1);
  }
  min_radius_plane_ = 0.066;
  min_radius_noise_ = 0.030, max_radius_noise_ = 0.050;
  max_min_radius_diff_ = 0.01;
  min_radius_edge_ = 0.030;


  double fixed_radius_search = 0.03;
  // read input cloud
  pcl::PointCloud<pcl::PointNormal> input_cloud;
  pcl::PCDReader reader;
  pcl::PCDWriter writer;
  reader.read ( argv[1], input_cloud);

  // Create the voxel grid
  pcl::PointCloud<pcl::PointNormal>::ConstPtr cloud = boost::make_shared<const pcl::PointCloud<pcl::PointNormal> > (input_cloud);
  pcl::PointCloud<pcl::PointNormal> cloud_downsampled;
  pcl::VoxelGrid<pcl::PointNormal> grid;
  double downsample_leaf = 0.01;                          // 1cm voxel size by default
  grid.setLeafSize (downsample_leaf, downsample_leaf, downsample_leaf);
  grid.setInputCloud (cloud);
  grid.setSaveLeafLayout(true);
  grid.filter (cloud_downsampled);
  pcl::PointCloud<pcl::PointNormal>::ConstPtr cloud_downsampled_ptr;
  cloud_downsampled_ptr.reset (new pcl::PointCloud<pcl::PointNormal> (cloud_downsampled));
  

  // Compute RSD
  pcl::RadiusSurfaceDescriptor <pcl::PointNormal, pcl::PointNormal, pcl::PrincipalRadiiRSD> rsd;
  rsd.setInputCloud(cloud_downsampled_ptr);
  rsd.setSearchSurface(cloud);
  rsd.setInputNormals(cloud);
  ROS_INFO("radius search: %f", std::max(fixed_radius_search, downsample_leaf/2 * sqrt(3)));
  rsd.setRadiusSearch(std::max(fixed_radius_search, downsample_leaf/2 * sqrt(3)));
  pcl::KdTree<pcl::PointNormal>::Ptr tree2 = boost::make_shared<pcl::KdTreeANN<pcl::PointNormal> > ();
  tree2->setInputCloud (cloud);
  rsd.setSearchMethod(tree2);
  pcl::PointCloud<pcl::PrincipalRadiiRSD> radii;
  rsd.compute(radii);
  writer.write("radii.pcd", radii, false);
  
  // Get rmin/rmax for adjacent 27 voxel
  Eigen3::MatrixXi relative_coordinates (3, 13);

  Eigen3::MatrixXi transition_matrix =  Eigen3::MatrixXi::Zero(6, 6);

  int idx = 0;
  
  // 0 - 8
  for( int i=-1; i<2; i++ )
  {
    for( int j=-1; j<2; j++ )
    {
      relative_coordinates( 0, idx ) = i;
      relative_coordinates( 1, idx ) = j;
      relative_coordinates( 2, idx ) = -1;
      idx++;
    }
  }
  // 9 - 11
  for( int i=-1; i<2; i++ )
  {
    relative_coordinates( 0, idx ) = i;
    relative_coordinates( 1, idx ) = -1;
    relative_coordinates( 2, idx ) = 0;
    idx++;
  }
  // 12
  relative_coordinates( 0, idx ) = -1;
  relative_coordinates( 1, idx ) = 0;
  relative_coordinates( 2, idx ) = 0;

  Eigen3::MatrixXi relative_coordinates_all (3, 26);
  relative_coordinates_all.block<3, 13>(0, 0) = relative_coordinates;
  relative_coordinates_all.block<3, 13>(0, 13) = -relative_coordinates;
  
  // Get transition matrix
  std::vector<int> types (radii.points.size());
 
  for (size_t idx = 0; idx < radii.points.size (); ++idx)
    types[idx] = get_type(radii.points[idx].r_min, radii.points[idx].r_max);
  
  for (size_t idx = 0; idx < cloud_downsampled_ptr->points.size (); ++idx)
  {
    int source_type = types[idx];
    std::vector<int> neighbors = grid.getNeighborCentroidIndices (cloud_downsampled_ptr->points[idx], relative_coordinates);
    for (unsigned id_n = 0; id_n < neighbors.size(); id_n++)
    {
      int neighbor_type;
      if (neighbors[id_n] == -1)
        neighbor_type = EMPTY;
      else
        neighbor_type = types[neighbors[id_n]];

      transition_matrix(source_type, neighbor_type)++;
    }
  }

  int nrf = 0;
  pcl::PointCloud<pcl::GRSDSignature21> cloud_grsd;
  cloud_grsd.points.resize(1);
  
  for (int i=0; i<NR_CLASS+1; i++)
  {
    for (int j=i; j<NR_CLASS+1; j++)
    {
      cloud_grsd.points[0].histogram[nrf++] = transition_matrix(i, j); //@TODO: resize point cloud
    }
  }
  writer.write("grsd.pcd", cloud_grsd, false);
  std::cerr << "transition matrix" << std::endl << transition_matrix << std::endl;
  return(0);
}
