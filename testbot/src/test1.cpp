//版本：整合底盤、通訊、導航用
#include <ros/ros.h>
//tf2必要
#include <tf2_ros/transform_broadcaster.h>
#include "tf2_ros/transform_listener.h"
#include <tf2/LinearMath/Quaternion.h>
#include "tf2/LinearMath/Transform.h"
//不確定用處
#include "tf2/convert.h"
#include "tf2/utils.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.h"
#include "tf2_ros/buffer.h"
#include "tf2_ros/message_filter.h"
#include "message_filters/subscriber.h"
//tf2和odom用到的訊息格式
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/TransformStamped.h>
#include <std_msgs/Int32MultiArray.h>
//額外的函式庫
#include <vector>
#include <typeinfo>
#include <stdio.h>

using namespace std;
vector<float> st1_msgs;

ros::Publisher odom_pub;
nav_msgs::Odometry odom;

//msgs格式  v , omega , x , y , theta
void ST1_callback(const std_msgs::Int32MultiArray::ConstPtr &msg)
{
    for (int i = 0; i < 2; i++)
    {
        st1_msgs[i] = float(msg->data[i]) / 1000;
    }
    for (int i = 2; i < 5; i++)
    {
        st1_msgs[i] = float(msg->data[i]) / 1000000;
    }
    static tf2_ros::TransformBroadcaster br;
    geometry_msgs::TransformStamped transformStamped;
    transformStamped.header.stamp = ros::Time::now();
    transformStamped.header.frame_id = "odom";
    transformStamped.child_frame_id = "base_footprint";
    transformStamped.transform.translation.x = st1_msgs[2];
    transformStamped.transform.translation.y = st1_msgs[3];
    transformStamped.transform.translation.z = 0.0;
    tf2::Quaternion odom_quat;
    odom_quat.setRPY(0, 0, st1_msgs[4]);
    transformStamped.transform.rotation.x = odom_quat.x();
    transformStamped.transform.rotation.y = odom_quat.y();
    transformStamped.transform.rotation.z = odom_quat.z();
    transformStamped.transform.rotation.w = odom_quat.w();
    br.sendTransform(transformStamped);

    //publish the odometry message
    odom.header.stamp = ros::Time::now();
    odom.header.frame_id = "odom";

    //set the position
    tf2::Quaternion Temporary;
    geometry_msgs::Quaternion odom_quat_pub;
    Temporary.setRPY(0, 0, st1_msgs[4]);
    tf2::convert(Temporary, odom_quat_pub);
    odom.pose.pose.position.x = st1_msgs[2];
    odom.pose.pose.position.y = st1_msgs[3];
    odom.pose.pose.position.z = 0.0;
    odom.pose.pose.orientation = odom_quat_pub;
    odom.pose.covariance = {1e-05, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1e-05, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1000000000000.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1000000000000.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1000000000000.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.001};

    //set the velocity
    odom.child_frame_id = "base_footprint";
    odom.twist.twist.linear.x = st1_msgs[0];
    odom.twist.twist.linear.y = 0.0;
    odom.twist.twist.angular.z = st1_msgs[1];
    odom.twist.covariance = {1e-05, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1e-05, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1000000000000.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1000000000000.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1000000000000.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.001};

    ROS_INFO("%f", st1_msgs[0]);
    ROS_INFO("%f", st1_msgs[1]);

    odom_pub.publish(odom);
};

// void tf2_listener(){
//     tf2_ros::Buffer tfBuffer;
//     tf2_ros::TransformListener tfListener(tfBuffer);
//     geometry_msgs::TransformStamped transformStamped_listen;
//     try{
//         ROS_INFO("listener success");
//         transformStamped_listen= tfBuffer.lookupTransform("base_footprint", "odom",ros::Time(0));
//     }
//     catch (tf2::TransformException &ex) {
//         ROS_WARN("%s",ex.what());
//         ROS_INFO("in");
//         ros::Duration(1.0).sleep();  // sleep for one second
//         continue;
//     }
// };

int main(int argc, char **argv)
{
    //default
    ros::init(argc, argv, "odometry_test");
    ros::NodeHandle n;

    ros::Subscriber ST1_sub = n.subscribe<std_msgs::Int32MultiArray>("rxST1", 20, &ST1_callback);
    odom_pub = n.advertise<nav_msgs::Odometry>("odom", 50);

    for (int i = 0; i < 5; i++)
    {
        st1_msgs.push_back(0.0);
    }
    st1_msgs[0] = 0.0; //v
    st1_msgs[1] = 0.0; //omega
    st1_msgs[2] = 0.8; //x
    st1_msgs[3] = 2.7; //y
    st1_msgs[4] = 0.0; //th

    ros::spin();
    return 0;
}
