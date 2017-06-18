#include "ros/ros.h"
#include "geometry_msgs/Twist.h"
#include "nav_msgs/Odometry.h"
#include "sensor_msgs/Range.h"
#include <sstream>

double x;
double y;
double w;
double newx;
double newy;
double neww;
double oldx;
double oldy;
double oldz;
double distancetraveled;
double obst_dist;

void getPosition( const nav_msgs::Odometry& current_position){
  x = current_position.pose.pose.position.x;
  y = current_position.pose.pose.position.y;
  w = current_position.pose.pose.orientation.w;
}

void ultrasoundDistance( const sensor_msgs::Range& ultra_msg){
  obst_dist = ultra_msg.range;
}

void findDistance(){
  double newxminusoldx;
  double newyminusoldy;
  double nxmoxsquared;
  double nymoysquared;
  double nxmoxspnymoys;
  newxminusoldx = newx - oldx;
  newyminusoldy = newy - oldy;
  nxmoxsquared = newxminusoldx * newxminusoldx;
  nymoysquared = newyminusoldy * newyminusoldy;
  nxmoxspnymoys = nxmoxsquared + nymoysquared;
  distancetraveled = sqrt(nxmoxspnymoys);
  distancetraveled = abs(distancetraveled);
}

int main(int argc, char **argv)
{
  
  ros::init(argc, argv, "obstacle_avoid_node");

  ros::NodeHandle n;

  ros::Publisher avoid_pub = n.advertise<geometry_msgs::Twist>("/cmd_vel_mux/input/obstacleavoidance", 1000);
  ros::Subscriber odom_sub = n.subscribe("odom", 1000, getPosition);
  ros::Subscriber ultrasound_sub = n.subscribe("range", 1000, ultrasoundDistance);
  ros::Publisher test_pub = n.advertise<nav_msgs::Odometry>("testodom",1000);

  ros::Rate loop_rate(10);

  while (ros::ok())
  {
    geometry_msgs::Twist cmd_vel_msg;
    nav_msgs::Odometry test_odom_msg;

    cmd_vel_msg.linear.x = .2;

    test_odom_msg.pose.pose.position.x = x;
    test_odom_msg.pose.pose.position.y = y;
    test_odom_msg.pose.pose.orientation.w = w;

    ROS_INFO("%f", cmd_vel_msg.linear.x);
  
    avoid_pub.publish(cmd_vel_msg);
    test_pub.publish(test_odom_msg);

    ros::spinOnce();
    loop_rate.sleep();
  }


  return 0;
}
