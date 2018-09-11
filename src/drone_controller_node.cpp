/**
 * @file offb_node.cpp
 * @brief GUIDED control example node, written with MAVROS version 0.19.x, PX4 Pro Flight
 * Stack and tested in Gazebo SITL
 */

#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <mavros_msgs/CommandBool.h>
#include <mavros_msgs/SetMode.h>
#include <mavros_msgs/State.h>
#include <mavros_msgs/CommandTOL.h>

mavros_msgs::State current_state;
void state_cb(const mavros_msgs::State::ConstPtr& msg){
    current_state = *msg;
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "offb_node");
    ros::NodeHandle nh;

    ros::Subscriber state_sub = nh.subscribe<mavros_msgs::State>
            ("mavros/state", 10, state_cb);
    ros::Publisher local_pos_pub = nh.advertise<geometry_msgs::PoseStamped>
            ("mavros/setpoint_position/local", 10);




    //the setpoint publishing rate MUST be faster than 2Hz
    ros::Rate rate(20.0);

    // wait for FCU connection
    while(ros::ok() && !current_state.connected){
        ros::spinOnce();
        rate.sleep();
    }

    geometry_msgs::PoseStamped pose;
    pose.pose.position.x = 0;
    pose.pose.position.y = 0;
    pose.pose.position.z = 2;

    //send a few setpoints before starting
    for(int i = 100; ros::ok() && i > 0; --i){
        local_pos_pub.publish(pose);
        ros::spinOnce();
        rate.sleep();
    }

    mavros_msgs::SetMode offb_set_mode;
    offb_set_mode.request.custom_mode = "GUIDED";
    ros::ServiceClient set_mode_client = nh.serviceClient<mavros_msgs::SetMode>
        ("mavros/set_mode");
    if(set_mode_client.call(offb_set_mode)) {
        ROS_INFO("Change mode cmd sent %d", offb_set_mode.response.mode_sent);
    } else {
        ROS_ERROR("Failed to Change Mode Service");
        return -1;
    }

    mavros_msgs::CommandBool arm_cmd;
    arm_cmd.request.value = true;
    ros::ServiceClient arming_client = nh.serviceClient<mavros_msgs::CommandBool>
        ("mavros/cmd/arming");
    if(arming_client.call(arm_cmd)) {
        sleep(2);
        ROS_INFO("Arming command sent %d", arm_cmd.response.success);;
    } else {
        ROS_ERROR("Failed to Call Arming Service");
        return -1;
    }

    mavros_msgs::CommandTOL takeoff_command;
    takeoff_command.request.altitude = 1.5;
    ros::ServiceClient takeoff_client = nh.serviceClient<mavros_msgs::CommandTOL>
        ("mavros/cmd/takeoff");
    if(takeoff_client.call(takeoff_command)) {
        sleep(5);
        ROS_INFO("takeoff sent %d", takeoff_command.response.success);
    } else {
        ROS_ERROR("Failed to Take Off");
        return -1;
    }

    while(ros::ok()){

        local_pos_pub.publish(pose);

        ros::spinOnce();
        rate.sleep();
    }

    return 0;
}
