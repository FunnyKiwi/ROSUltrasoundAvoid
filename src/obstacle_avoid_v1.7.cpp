#include "ros/ros.h"
#include "geometry_msgs/Twist.h"
#include "nav_msgs/Odometry.h"
#include "sensor_msgs/Range.h"
#include <sstream>
#include <iostream>

double x;
double y;
double w;
double newx;
double newy;
double neww;
double oldx;
double oldy;
double oldw;
double distancetraveled;
double maxdistance = 1;
double obst_dist = 0.5;
double trigger_dist = 1;
double testDirNum;
bool turning_r = false;
bool turning_l = false;
int selection;

geometry_msgs::Twist cmd_vel_msg;
nav_msgs::Odometry test_odom_msg;

void getPosition( const nav_msgs::Odometry& current_position){
  x = current_position.pose.pose.position.x;
  y = current_position.pose.pose.position.y;
  w = current_position.pose.pose.orientation.w;
}

void ultrasoundDistance( const sensor_msgs::Range& ultra_msg){
  obst_dist = ultra_msg.range;
}

void findDistance(){
  float newxminusoldx;
  float newyminusoldy;
  float nxmoxsquared;
  float nymoysquared;
  float nxmoxspnymoys;
  newxminusoldx = newx - oldx;
  newyminusoldy = newy - oldy;
  nxmoxsquared = newxminusoldx * newxminusoldx;
  nymoysquared = newyminusoldy * newyminusoldy;
  nxmoxspnymoys = nxmoxsquared + nymoysquared;
  distancetraveled = sqrt(nxmoxspnymoys);
  distancetraveled = abs(distancetraveled);
}

void swerveR(){
  if(distancetraveled < maxdistance && turning_r == true){
    cmd_vel_msg.linear.x = .2;
    cmd_vel_msg.angular.z = .5;
		newx = x;
		newy = y;
  }
	else{
		turning_r = false;
	}
}

void obstacleScanner(){
  if(obst_dist < trigger_dist && obst_dist != 0 && turning_r == false){
    turning_r = true;
		oldx = x;
		oldy = y;
		oldw = w;
  }
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
    obstacleScanner();
    findDistance();
		swerveR();

		test_odom_msg.pose.pose.position.x = oldx;
		test_odom_msg.pose.pose.position.y = oldy;
		test_odom_msg.pose.pose.position.z = distancetraveled;
		
		ROS_INFO("%s",
		"---------------------------------------");
  	ROS_INFO("%s", "selection:");
		ROS_INFO("%f", selection);
		ROS_INFO("%s", "turning_r:");
		ROS_INFO("%s", turning_r ? "true" : "false");
		ROS_INFO("%s", "turning_l:");
		ROS_INFO("%s", turning_l ? "true" : "false");
		ROS_INFO("%s", "distancetraveled:");
		ROS_INFO("%f", distancetraveled);	
		ROS_INFO("%s", "maxdistance:");
		ROS_INFO("%f", maxdistance);
  
    avoid_pub.publish(cmd_vel_msg);
    test_pub.publish(test_odom_msg);

    ros::spinOnce();
    loop_rate.sleep();
  }
}
