#include <string>
#include <iostream>

#include <unistd.h> //for sleep()

#include <ros/ros.h>
#include <json_prolog/prolog.h>

using namespace std;
using namespace json_prolog;

int main(int argc, char *argv[])
{
  ros::init(argc, argv, "best_object_location");

  Prolog pl;

  //classifiers: best_location_maxMaxWup and best_location_dtree
  //best_location_maxMaxWup may return multiple solutions (if they
  //have equal similarities)

  //query: get location as string
  {
    string object = "knowrob:'Spoon'";
    string classifier = "best_location_maxMaxWup"; //best_location_dtree

    string query = classifier + "(" + object + ", L)";
    PrologQueryProxy bdgs = pl.query(query);
    
    for(PrologQueryProxy::iterator it=bdgs.begin();
	it != bdgs.end(); it++)
      {
	PrologBindings bdg = *it;
	//cout << "Found solution: " << (bool)(it == bdgs.end()) << endl;
	cout << "Location = "<< bdg["L"] << endl;
      }
  }
    
    
  //query and highlight in knowrob visualization
  {
    //initialize visualization:
    pl.query("register_ros_package(mod_vis)");
    pl.query("use_module(library('mod_vis'))");
    pl.query("mod_vis:visualisation_canvas(C)");
    //wait until it is initialized
    sleep(5);
    
    string object = "knowrob:'Spoon'";
    string classifier = "best_location_maxMaxWup"; //best_location_dtree

    string query = "highlight_" + classifier + "(" + object + ", _)";
    cout << query << endl;
    pl.query(query);
    
    sleep(10);
    
    object = "knowrob:'Milk'";
    query = "highlight_" + classifier + "(" + object + ", _)";
    cout << query << endl;
    pl.query(query);
  }
  
  return 0;
}
