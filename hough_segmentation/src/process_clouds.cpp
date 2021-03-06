/*
 * Copyright (c) 2011, Lucian Cosmin Goron <goron@cs.tum.edu>
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
 *     * Neither the name of Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
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
 */

// ------------------------------------------------------------------------- //
// -------------------- Specify all needed dependencies -------------------- //
// ------------------------------------------------------------------------- //

// ros dependencies
#include "ros/ros.h"

// terminal tools dependecies
#include "terminal_tools/parse.h"

// pcl dependencies
#include "pcl/io/pcd_io.h"
#include "pcl/features/normal_3d.h"
#include "pcl/filters/passthrough.h"
#include "pcl/filters/extract_indices.h"
#include "pcl/filters/project_inliers.h"
#include "pcl/filters/statistical_outlier_removal.h"
#include "pcl/sample_consensus/method_types.h"
#include "pcl/sample_consensus/sac_model_circle.h"
#include "pcl/surface/convex_hull.h"
#include "pcl/segmentation/sac_segmentation.h"
#include "pcl/segmentation/extract_clusters.h"
#include "pcl/segmentation/extract_polygonal_prism_data.h"

// pcl visualization dependencies
#include "pcl_visualization/pcl_visualizer.h"

// ---------------------------------------------------------------------- //
// -------------------- Declare definitions of types -------------------- //
// ---------------------------------------------------------------------- //

typedef pcl::PointXYZINormal PointT;

// --------------------------------------------------------------------------- //
// -------------------- Declare the processing parameters -------------------- //
// --------------------------------------------------------------------------- //

// Options for processing
bool pass = false; 
bool table = false; 
bool filter = false; 
bool normal = false; 
bool refine = false; 
bool project = false; 

// Pass through filter's parameters
double height = 0.500; /// [meters]

// Plane fitting's parameters
int weight = 0.100; /// [meters]
double epsilon = 0.100; /// [radians]
int iterations = 1000; /// [iterations]
double threshold = 0.100; /// [meters]

// Visualization's parameters
bool verbose = false;
int size_of_points = 3;



////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief Main routine of the method. Processing point clouds. Pass through filtering.
 * Objects segmentation of the table. Statistical noise removal. Computation of normals in 2D.
 * Refinement of normals in 2D. Projection of points in 2D.
 */
int main (int argc, char** argv)
{

  // ------------------------------------------------------------------------- //
  // -------------------- Check and parse arguments -------------------- //
  // ------------------------------------------------------------------------- //

  // Argument check and info about
  if (argc < 2)
  {
    ROS_INFO (" ");
    ROS_INFO ("Syntax is: %s <input>.pcd <options>", argv[0]);
    ROS_INFO ("  where <options> are:");
    ROS_INFO (" ");
    ROS_INFO ("    -pass B                = Yes/No to passing through filter");
    ROS_INFO ("    -table B               = Yes/No to segmenting the objects on the table");
    ROS_INFO ("    -filter B              = Yes/No to filtering by statistical outlier removal");
    ROS_INFO ("    -normal B              = Yes/No to computing the normals of points");
    ROS_INFO ("    -refine B              = Yes/No to refinement of normals in 2D");
    ROS_INFO ("    -project B             = Yes/No to projecting points in xOy plane");
    ROS_INFO (" ");
    ROS_INFO ("    -height X              = The height of pass through filter");
    ROS_INFO (" ");
    ROS_INFO ("    -weight X              = The weight of angular distance between point normals and the normal of plane");
    ROS_INFO ("    -epsilon X             = The maximum allowed difference between the plane normal and the given axis");
    ROS_INFO ("    -threshold X           = The distance to the fitted plane model");
    ROS_INFO ("    -iterations X          = The maximum number of interations for fitting the plane model");
    ROS_INFO (" ");
    ROS_INFO ("    -verbose B             = Yes/No to printing of useful messages");
    ROS_INFO ("    -size_of_points D      = The size of points in 3D viewer");
    ROS_INFO (" ");
    return (-1);
  }

  // Take only the first .pcd file into account
  std::vector<int> pFileIndicesPCD = terminal_tools::parse_file_extension_argument (argc, argv, ".pcd");
  if (pFileIndicesPCD.size () == 0)
  {
    ROS_INFO ("No .pcd file given as input!");
    return (-1);
  }

  // Options for processing
  terminal_tools::parse_argument (argc, argv, "-pass", pass);
  terminal_tools::parse_argument (argc, argv, "-table", table);
  terminal_tools::parse_argument (argc, argv, "-filter", filter);
  terminal_tools::parse_argument (argc, argv, "-normal", normal);
  terminal_tools::parse_argument (argc, argv, "-refine", refine);
  terminal_tools::parse_argument (argc, argv, "-project", project);

  // Pass through filter's parameters
  terminal_tools::parse_argument (argc, argv, "-height", height);

  // Plane fitting's parameters
  terminal_tools::parse_argument (argc, argv, "-weight", weight);
  terminal_tools::parse_argument (argc, argv, "-epsilon", epsilon);
  terminal_tools::parse_argument (argc, argv, "-threshold", threshold);
  terminal_tools::parse_argument (argc, argv, "-iterations", iterations);

  // Visualization's parameters
  terminal_tools::parse_argument (argc, argv, "-verbose", verbose);
  terminal_tools::parse_argument (argc, argv, "-size_of_points", size_of_points);

  // --------------------------------------------------------- //
  // -------------------- Initializations -------------------- //
  // --------------------------------------------------------- //

  // Initialize random number generator
  srand (time(0));

  // Initialize ros time
  ros::Time::init();

  // Declare the timer
  terminal_tools::TicToc tt;

  // Starting timer
  tt.tic ();

  if ( verbose )
  {
    // Displaying when the timer starts
    ROS_WARN ("Timer started !");
  }

  // ------------------------------------------------------------------------- //
  // -------------------- Load point cloud data -------------------- //
  // ------------------------------------------------------------------------- //

  // Input point cloud data
  pcl::PointCloud<PointT>::Ptr input_cloud (new pcl::PointCloud<PointT> ());

  // Load point cloud data
  if (pcl::io::loadPCDFile (argv [pFileIndicesPCD [0]], *input_cloud) == -1)
  {
    ROS_ERROR ("Couldn't read file %s", argv [pFileIndicesPCD [0]]);
    return (-1);
  }

  if ( verbose )
  {
    ROS_INFO ("Loaded %d data points from %s with the following fields: %s", (int) (input_cloud->points.size ()), argv[pFileIndicesPCD[0]], pcl::getFieldsList (*input_cloud).c_str ());
  }

  // Working point cloud data 
  pcl::PointCloud<PointT>::Ptr working_cloud (new pcl::PointCloud<PointT> ());

  // Update working point cloud
  *working_cloud = *input_cloud;

  // -------------------------------------------------------------------- //
  // -------------------- Visualize point cloud data -------------------- //
  // -------------------------------------------------------------------- //

  // Open a 3D viewer
  pcl_visualization::PCLVisualizer viewer ("3D VIEWER");
  // Set the background of viewer
  viewer.setBackgroundColor (0.0, 0.0, 0.0);
  // Add system coordiante to viewer
  viewer.addCoordinateSystem (1.0f);
  // Parse the camera settings and update the internal camera
  viewer.getCameraParameters (argc, argv);
  // Update camera parameters and render
  viewer.updateCamera ();

  // Add the point cloud data
  viewer.addPointCloud (*working_cloud, "INPUT");
  // Set the size of points for cloud
  viewer.setPointCloudRenderingProperties (pcl_visualization::PCL_VISUALIZER_POINT_SIZE, size_of_points, "INPUT"); 
  // And wait until Q key is pressed
  viewer.spin ();

  // -------------------------------------------------------------- //
  // -------------------- Pass Through Filter  -------------------- //
  // -------------------------------------------------------------- //

  if ( pass )
  {
    // The minimum and maximum height of point cloud
    PointT minimum, maximum;
    pcl::getMinMax3D (*working_cloud, minimum, maximum);

    if ( verbose )
    {
      ROS_INFO ("The point with minimum height is (%6.3f,%6.3f,%6.3f)", minimum.x, minimum.y, minimum.z);
      ROS_INFO ("The point with maximum height is (%6.3f,%6.3f,%6.3f)", maximum.x, maximum.y, maximum.z);
      ROS_INFO ("The height of point cloud is %5.3f meters", maximum.z - minimum.z);
    } 

    // Create the filtering object
    pcl::PassThrough<pcl::PointXYZINormal> pass;
    pcl::PointCloud<PointT>::Ptr through (new pcl::PointCloud<PointT> ());

    // Set parameters for filtering
    pass.setFilterFieldName ("z");
    pass.setFilterLimits (0.0, height);
    pass.setInputCloud (working_cloud);

    // Call the filtering function
    pass.setFilterLimitsNegative (false);
    pass.filter (*through);
    pass.setFilterLimitsNegative (true);
    pass.filter (*working_cloud);

    if ( verbose )
    {
      ROS_INFO ("Filtered cloud has %d points !", through->points.size ());
      ROS_INFO ("Remaining cloud has %d points !", working_cloud->points.size ());
    }

    // Create ID for visualization
    std::stringstream id;
    id << "THROUGH_" << ros::Time::now();

    // Add point cloud to viewer
    viewer.addPointCloud (*through, id.str());
    // Set the size of points for cloud
    viewer.setPointCloudRenderingProperties (pcl_visualization::PCL_VISUALIZER_POINT_SIZE, size_of_points, id.str()); 
    // And wait until Q key is pressed
    viewer.spin ();
  }

  // ---------------------------------------------------------------------------- //
  // -------------------- Segmenting Objects from the Table  -------------------- //
  // ---------------------------------------------------------------------------- //

  if ( table )
  {
    // Point cloud of normals
    pcl::PointCloud<pcl::Normal>::Ptr normals (new pcl::PointCloud<pcl::Normal> ());
    // Build kd-tree structure for normals
    pcl::KdTreeFLANN<PointT>::Ptr tree (new pcl::KdTreeFLANN<PointT> ());

    // Create object for normal estimation
    pcl::NormalEstimation<PointT, pcl::Normal> estimation;
    // Provide pointer to the search method
    estimation.setSearchMethod (tree);
    // Set for which point cloud to compute the normals
    estimation.setInputCloud (working_cloud);
    // Set number of k nearest neighbors to use
    estimation.setKSearch (50);

    // Estimate the normals
    estimation.compute (*normals);

    if ( verbose )
    {
      ROS_INFO ("Normal Estimation ! Returned: %d normals", (int) normals->points.size ());
    }

    // Create the segmentation object and declare variables
    pcl::SACSegmentationFromNormals<PointT, pcl::Normal> segmentation;
    Eigen::Vector3f axis = Eigen::Vector3f (0.0, 0.0, 1.0); 
    pcl::PointIndices::Ptr inliers (new pcl::PointIndices ());
    pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients ());

    // Set all the parameters for segmenting 
    segmentation.setAxis (axis);
    segmentation.setEpsAngle (epsilon);
    segmentation.setInputNormals (normals);
    segmentation.setInputCloud (working_cloud);
    segmentation.setMaxIterations (iterations);
    segmentation.setOptimizeCoefficients (true);
    segmentation.setMethodType (pcl::SAC_RANSAC);
    segmentation.setDistanceThreshold (threshold);
    segmentation.setNormalDistanceWeight (weight);
    segmentation.setModelType (pcl::SACMODEL_NORMAL_PLANE);

    // Obtain the plane inliers and coefficients
    segmentation.segment (*inliers, *coefficients);

    if ( verbose )
    {
      ROS_INFO ("Plane has %5d inliers with parameters A = %f B = %f C = %f and D = %f found in maximum %d iterations", (int) inliers->indices.size (),
                coefficients->values [0], coefficients->values [1], coefficients->values [2], coefficients->values [3], iterations);
    }

    // Extract the inliers from the working cloud
    pcl::ExtractIndices<PointT> extraction;
    pcl::PointCloud<PointT>::Ptr cloud (new pcl::PointCloud<PointT> ());

    // Set point cloud from where to extract
    extraction.setInputCloud (working_cloud);
    // Set which indices to extract
    extraction.setIndices (inliers);

    // Return the points which represent the inliers
    extraction.setNegative (false);
    // Call the extraction function
    extraction.filter (*cloud);
    // Return the remaining points of inliers
    extraction.setNegative (true);
    // Call the extraction function
    extraction.filter (*working_cloud);

    // Create ID for visualization
    std::stringstream id;
    id << "INLIERS_" << ros::Time::now();

    // Add point cloud to viewer
    viewer.addPointCloud (*cloud, id.str());
    // Set the size of points for cloud
    viewer.setPointCloudRenderingProperties (pcl_visualization::PCL_VISUALIZER_POINT_SIZE, size_of_points, id.str ()); 
    // And wait until Q key is pressed
    viewer.spin ();





    pcl::PointIndices::Ptr over (new pcl::PointIndices ());
    pcl::PointIndices::Ptr under (new pcl::PointIndices ());

    float a = coefficients->values [0];
    float b = coefficients->values [1];
    float c = coefficients->values [2];
    float d = coefficients->values [3];



    for (int idx = 0; idx < (int) working_cloud->points.size (); idx++)
    {



      float x = working_cloud->points [idx].x;
      float y = working_cloud->points [idx].y;
      float z = working_cloud->points [idx].z;



      float s = (a*x) + (b*y) + (c*z) + d;


      //cerr << " s = " << s << endl ;

      if ( s < 0 ) 
      {
        //cerr << " idx " << idx <<  " bla " << endl ;
        over->indices.push_back (idx);
      }
      else
      {
        //cerr << " idx " << idx << " alb " << endl ;
        under->indices.push_back (idx);
      }

      //cerr << " s = " << s << endl ;
    }






    // Extract the inliers from the working cloud
    pcl::ExtractIndices<PointT> extraction_;
    pcl::PointCloud<PointT>::Ptr cloud_ (new pcl::PointCloud<PointT> ());

    // Set point cloud from where to extract
    extraction_.setInputCloud (working_cloud);
    // Set which indices to extract
    extraction_.setIndices (over);

    // Return the points which represent the inliers
    extraction_.setNegative (false);
    // Call the extraction function
    extraction_.filter (*cloud_);
    // Return the remaining points of inliers
    extraction_.setNegative (true);
    // Call the extraction function
    extraction_.filter (*working_cloud);

    // Create ID for visualization
    std::stringstream id_;
    id_ << "INLIERS_" << ros::Time::now();

    // Add point cloud to viewer
    viewer.addPointCloud (*cloud_, id_.str());
    // Set the size of points for cloud
    viewer.setPointCloudRenderingProperties (pcl_visualization::PCL_VISUALIZER_POINT_SIZE, size_of_points, id_.str ()); 
    // And wait until Q key is pressed
    viewer.spin ();






    // Get position of dot in path of file 
    std::string file = argv [pFileIndicesPCD[0]];
    size_t dot = file.find (".");

    // Create file name for saving
    std::string name = argv [pFileIndicesPCD [0]];
    name.insert (dot, "-objects");

    // Save these points to disk
    pcl::io::savePCDFile (name, *cloud_);

    if ( verbose )
    {
      // Show the objects' number of points
      ROS_INFO ("Message %d points !", (int) cloud_->points.size ());
    }

















/*
    // Project the table inliers using the planar model coefficients    
    pcl::ProjectInliers<PointT> projecting;   
    pcl::PointCloud<PointT>::Ptr projected_cloud (new pcl::PointCloud<PointT> ()); 

    // Set all the parameters for projecting 
    projecting.setModelType (pcl::SACMODEL_NORMAL_PLANE);
    projecting.setInputCloud (cloud);
    projecting.setModelCoefficients (coefficients);
    projecting.filter (*projected_cloud);



    // Remove the point cloud data
    viewer.removePointCloud (id.str ());
    // And wait until Q key is pressed
    viewer.spin ();



    // Visualize Projecte Poitns
    std::stringstream id_of_proj;
    id_of_proj << "PROJ_" << ros::Time::now();

    // Add point cloud to viewer
    viewer.addPointCloud (*projected_cloud, id_of_proj.str());
    // Set the size of points for cloud
    viewer.setPointCloudRenderingProperties (pcl_visualization::PCL_VISUALIZER_POINT_SIZE, size_of_points * 2, id_of_proj.str());
    // And wait until Q key is pressed
    viewer.spin ();



    // Create a Convex Hull representation of the projected inliers
    pcl::ConvexHull<PointT> chull_;  
    pcl::PointCloud<PointT> cloud_hull;

    chull_.setInputCloud (projected_cloud);
    ROS_ERROR (" BEFORE IT WORKS ");
    chull_.reconstruct (cloud_hull);      
    ROS_ERROR (" AFTERWARDS NOT ");

    ROS_INFO ("Convex hull has %d data points.", (int) cloud_hull.points.size ());



    // Visualize Convex Hulls
    std::stringstream id_of_hull;
    id_of_hull << "HULL_" << ros::Time::now();

    // Visualize hull of cloud 
    viewer.addPointCloud (cloud_hull, id_of_hull.str());
    // Set the size of points for cloud
    viewer.setPointCloudRenderingProperties (pcl_visualization::PCL_VISUALIZER_POINT_SIZE, size_of_points, id_of_hull.str());     
    // And wait until Q key is pressed
    viewer.spin ();



    pcl::PointIndices::Ptr cloud_object_indices (new pcl::PointIndices ());

    // ---[ Get the objects on top of the table
    pcl::ExtractPolygonalPrismData<PointT> prism_;
    prism_.setHeightLimits (0.000, 0.500);
    prism_.setInputCloud (input_cloud);
    prism_.setInputPlanarHull (cloud_hull.makeShared());
    prism_.setViewPoint (-1.0, -1.0, 1.0);
    prism_.segment (*cloud_object_indices);

    if ( (int) cloud_object_indices->indices.size () != 0 )
    {
      // Extract objects
      //pcl::PointCloud<PointT> object_cloud;
      pcl::PointCloud<PointT>::Ptr object_cloud (new pcl::PointCloud<PointT> ()); 
      pcl::ExtractIndices<PointT> extraction_of_object;
      // Set point cloud from where to extract
      extraction_of_object.setInputCloud (input_cloud);
      // Set which indices to extract
      extraction_of_object.setIndices (cloud_object_indices);
      // Return the points which represent the inliers
      extraction_of_object.setNegative (false);
      // Call the extraction function
      extraction_of_object.filter (*object_cloud);



      // Visualize objects
      std::stringstream id_of_object;
      id_of_object << "OBJECTS_" << ros::Time::now();

      // Visualize object
      viewer.addPointCloud (*object_cloud, id_of_object.str());
      // Set the size of points for cloud
      viewer.setPointCloudRenderingProperties (pcl_visualization::PCL_VISUALIZER_POINT_SIZE, size_of_points * 3, id_of_object.str());     
      // And wait until Q key is pressed
      viewer.spin ();
    }
*/
  }

















  // ------------------------------------------------------------------- //
  // -------------------- Filtering of Point Cloud  -------------------- //
  // ------------------------------------------------------------------- //

  if ( filter )
  {
    // Filtered point cloud
    pcl::PointCloud<PointT>::Ptr filtered_cloud (new pcl::PointCloud<PointT> ());

    // Create the filtering object
    pcl::StatisticalOutlierRemoval<PointT> sor;
    // Set which point cloud to filter
    sor.setInputCloud (working_cloud);
    // Set number of points for mean distance estimation
    sor.setMeanK (25);
    // Set the standard deviation multiplier threshold
    sor.setStddevMulThresh (1.0);
    // Call the filtering method
    sor.filter (*working_cloud);

    if ( verbose )
    {
      ROS_INFO ("Statistical Outlier Removal ! Before: %d points | After: %d points | Filtered: %d points",
                (int) input_cloud->points.size (),  (int) working_cloud->points.size (), (int) input_cloud->points.size () - (int) working_cloud->points.size ());
    }

    // Add the point cloud data
    viewer.addPointCloud (*working_cloud, "FILTERED");
    // Set the size of points for cloud
    viewer.setPointCloudRenderingProperties (pcl_visualization::PCL_VISUALIZER_POINT_SIZE, size_of_points, "FILTERED"); 
    // And wait until Q key is pressed
    viewer.spin ();
  }

  // -------------------------------------------------------------------- //
  // -------------------- Projecting of Point Clouds -------------------- //
  // -------------------------------------------------------------------- //

  if ( project )
  {
    for (int idx = 0; idx < (int) working_cloud->points.size (); idx++)
      working_cloud->points[idx].z = 0.0;  

    // Add the point cloud data
    viewer.addPointCloud (*working_cloud, "PROJECTED");
    // Set the size of points for cloud
    viewer.setPointCloudRenderingProperties (pcl_visualization::PCL_VISUALIZER_POINT_SIZE, size_of_points, "PROJECTED"); 
    // And wait until Q key is pressed
    viewer.spin ();
  }

  // ---------------------------------------------------------------------- //
  // -------------------- Estimating Normals of Points -------------------- //
  // ---------------------------------------------------------------------- //

  if ( normal )
  {
    // Point cloud of normals
    pcl::PointCloud<pcl::Normal>::Ptr normals_cloud (new pcl::PointCloud<pcl::Normal> ());
    // Build kd-tree structure for normals
    pcl::KdTreeFLANN<PointT>::Ptr normals_tree (new pcl::KdTreeFLANN<PointT> ());

    // Create object for normal estimation
    pcl::NormalEstimation<PointT, pcl::Normal> ne;
    // Provide pointer to the search method
    ne.setSearchMethod (normals_tree);
    // Set for which point cloud to compute the normals
    ne.setInputCloud (working_cloud);
    // Set number of k nearest neighbors to use
    ne.setKSearch (50);
    // Estimate the normals
    ne.compute (*normals_cloud);

    if ( verbose )
    {
      ROS_INFO ("Normal Estimation ! Returned: %d normals", (int) normals_cloud->points.size ());
    }

    int level = 1;
    double scale = 0.025;
    // Add the normals
    viewer.addPointCloudNormals (*working_cloud, *normals_cloud, level, scale, "NORMALS 3D");
    // Color the normals with red
    viewer.setPointCloudRenderingProperties (pcl_visualization::PCL_VISUALIZER_COLOR, 1.0, 0.0, 0.0, "NORMALS 3D"); 
    // And wait until Q key is pressed 
    viewer.spin ();

    // Remove the point cloud data
    viewer.removePointCloud ("NORMALS 3D");
    // And wait until Q key is pressed
    viewer.spin ();



    cerr << normals_cloud->points[10].normal_x << " " << normals_cloud->points[10].normal_y << " " << normals_cloud->points[10].normal_z << " "<<   endl ;



    // ------------------------------------------------------------------------- //
    // -------------------- Refinement of Normals of Points -------------------- //
    // ------------------------------------------------------------------------- //

    if ( refine )
    {
      for (int idx = 0; idx < (int) normals_cloud->points.size (); idx++)
        normals_cloud->points[idx].normal_z = 0.0;  

      // Add the normals
      viewer.addPointCloudNormals (*working_cloud, *normals_cloud, level, scale, "NORMALS 2D");
      // Color the normals with red
      viewer.setPointCloudRenderingProperties (pcl_visualization::PCL_VISUALIZER_COLOR, 0.0, 1.0, 0.0, "NORMALS 2D"); 
      // And wait until Q key is pressed 
      viewer.spin ();

      // Remove the point cloud data
      viewer.removePointCloud ("NORMALS 2D");
      // And wait until Q key is pressed
      viewer.spin ();



      cerr << normals_cloud->points[10].normal_x << " " << normals_cloud->points[10].normal_y << " " << normals_cloud->points[10].normal_z << " "<<   endl ;



    }

    if ( refine )
    {
      for (int idx = 0; idx < (int) normals_cloud->points.size (); idx++)
      {
        double nx = normals_cloud->points[idx].normal_x;
        double ny = normals_cloud->points[idx].normal_y;
        double nz = normals_cloud->points[idx].normal_z;

        double nl = sqrt (nx*nx + ny*ny + nz*nz);
        nx = nx / nl;
        ny = ny / nl;
        nz = nz / nl;

        normals_cloud->points[idx].normal_x = nx;
        normals_cloud->points[idx].normal_y = ny;
        normals_cloud->points[idx].normal_z = nz;

      }
    
      // Add the normals
      viewer.addPointCloudNormals (*working_cloud, *normals_cloud, level, scale, "NORMALS");
      // Color the normals with red
      viewer.setPointCloudRenderingProperties (pcl_visualization::PCL_VISUALIZER_COLOR, 0.0, 1.0, 0.0, "NORMALS"); 
      // And wait until Q key is pressed 
      viewer.spin ();

      // Remove the point cloud data
      viewer.removePointCloud ("NORMALS");
      // And wait until Q key is pressed
      viewer.spin ();



      cerr << normals_cloud->points[10].normal_x << " " << normals_cloud->points[10].normal_y << " " << normals_cloud->points[10].normal_z << " "<<   endl ;



    }



    cerr << normals_cloud->points[10].normal_x << " " << normals_cloud->points[10].normal_y << " " << normals_cloud->points[10].normal_z << " "<<   endl ;



  }



















 //// Get position of dot in path of file 
  //std::string file = argv [pFileIndicesPCD[0]];
  //size_t dot = file.find (".");

  //// Create file name for saving
  //std::string circle_space_filename = argv [pFileIndicesPCD [0]];
  //circle_space_filename.insert (dot, "-circles");

  //// Save these points to disk
  //pcl::io::savePCDFile (circle_space_filename, *those_points);

  //if ( verbose )
  //{
    //// Show the floor's number of points
    //ROS_INFO ("The space which is represented by the circles models has %d points and was saved !", (int) those_points->points.size ());
  //}


















  if ( verbose )
  {
    // Displaying the overall time
    ROS_WARN ("Finished in %5.3g [s] !", tt.toc ());
  }

  // And wait until Q key is pressed
  viewer.spin ();

return (0);
}
