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
double newoldw;
double distancetraveled;
double maxdistance = 1;
double obst_dist = 0.5;
double trigger_dist = 1;
double testDirNum;
bool turning_r = false;
bool turning_l = false;
double selection;
double starttime;
double currenttime;
bool reset_ang = false;
bool scan = false;

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

void resetAngle(){
	if(reset_ang == true){
		neww = w;
		if(neww < oldw + 0.1 && neww > oldw - 0.1){
			reset_ang = false;
			scan = true;
		}		
		cmd_vel_msg.linear.x = 0;
		if(neww > oldw){
			cmd_vel_msg.angular.z = 1;
		}
		if(neww < oldw){
			cmd_vel_msg.angular.z = -1;
		}
		scan = false;
	}
}

void swerveR(){
  if(currenttime < starttime && turning_r == true && reset_ang == false){
    cmd_vel_msg.linear.x = .2;
    cmd_vel_msg.angular.z = .5;
		newoldw = w;
    newx = x;
    newy = y;
		scan = false;
  }
	if(currenttime >= starttime && turning_r == true && reset_ang == false){
		reset_ang = true;
		turning_r = false;
		scan = false;
	}
}

void obstacleScanner(){
  if(obst_dist < trigger_dist && obst_dist != 0 && scan == true){
    turning_r = true;
    oldx = x;
    oldy = y;
    oldw = w;
		starttime = ros::Time::now().toSec();
		starttime = starttime + 2;
		scan = false;
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
    		currenttime = ros::Time::now().toSec();
				scan = true;
				resetAngle();
				if(reset_ang != true){        	
					swerveR();
				}
				obstacleScanner();

        test_odom_msg.pose.pose.position.x = oldx;
        test_odom_msg.pose.pose.position.y = oldy;
        test_odom_msg.pose.pose.position.z = distancetraveled;
        
        ROS_INFO("%s",
        "---------------------------------------");
        ROS_INFO("%s", "starttime:");
        ROS_INFO("%f", starttime);
				ROS_INFO("%s", "currenttime:");
        ROS_INFO("%f", currenttime);
				ROS_INFO("%s", "reset_ang:");
        ROS_INFO("%s", reset_ang ? "true" : "false");
        ROS_INFO("%s", "turning_r:");
        ROS_INFO("%s", turning_r ? "true" : "false");
        ROS_INFO("%s", "scan:");
        ROS_INFO("%s", scan ? "true" : "false");
        ROS_INFO("%s", "oldw:");
        ROS_INFO("%f", oldw);   
        ROS_INFO("%s", "neww:");
        ROS_INFO("%f", neww);
		
		if(turning_r == true || turning_l == true || reset_ang == true){
				avoid_pub.publish(cmd_vel_msg);
		}
    test_pub.publish(test_odom_msg);

    ros::spinOnce();
    loop_rate.sleep();
  }
}
