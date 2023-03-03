/*
 * Copyright 2015 Fadri Furrer, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Michael Burri, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Mina Kamel, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Janosch Nikolic, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Markus Achtelik, ASL, ETH Zurich, Switzerland
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Eigen/Eigen>

#include <mav_msgs/conversions.h>
#include <mav_msgs/eigen_mav_msgs.h>
#include <mav_msgs/CommandMotorSpeed.h>
#include <mav_msgs/CommandRollPitchYawrateThrust.h>
#include <mav_msgs/MotorSpeed.h>
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/PoseStamped.h>
#include <sensor_msgs/Imu.h>
#include <planning_msgs/WayPoint.h>
#include <planning_msgs/eigen_planning_msgs.h>
#include <planning_msgs/conversions.h>
#include <ros/ros.h>
#include <ros/callback_queue.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_datatypes.h>

#include "quad_control/parameters_ros.h"

namespace quad_control {

class ControllerUtility{
  public:
  ControllerUtility();
  ~ControllerUtility();

  //Utility functions 
  double map(double x, double in_min, double in_max, double out_min, double out_max);
  double limit( double in, double min, double max);
  bool GetSwitchValue(void);
  bool UpdateSwitchValue(bool currInput);
  Eigen::Vector3d rotateGFtoBF(double GF_x, double GF_y, double GF_z, double GF_roll, double GF_pitch, double GF_yaw);


  private:
    bool switchValue;
    bool prevInput;

};

class PositionController{
  public:
   PositionController();
   ~PositionController();

  void InitializeParameters(const ros::NodeHandle& pnh);

  void CalculatePositionControl(mav_msgs::CommandTrajectory wp, nav_msgs::Odometry current_gps, mav_msgs::CommandRollPitchYawrateThrust *des_attitude_output);

  mav_msgs::CommandRollPitchYawrateThrust des_attitude_cmds;

  private:

  //General
  tf::Quaternion q;
  double gps_roll, gps_pitch, gps_yaw;
  double gps_x, gps_y, gps_z;
  double mass;

  ros::Time last_time;
  ros::Time sim_time;
  double dt;

  Eigen::Vector3d wp_BF;
  Eigen::Vector3d pos_BF;
  Eigen::Vector3d vel_BF;
  ControllerUtility controller_utility_;

  //Position Controller
  double x_er, y_er, z_er, yaw_er;
  double x_er_sum, y_er_sum, z_er_sum, yaw_er_sum;
  double cp, ci, cd;

  //X PID
  double x_KI_max;
  double x_KP;
  double x_KI;
  double x_KD;

  //Y PID
  double y_KI_max;
  double y_KP;
  double y_KI;
  double y_KD;

  //Z PID
  double z_KI_max;
  double z_KP;
  double z_KI;
  double z_KD;
  double z_target;

  //Yaw PID
  double yaw_KI_max;
  double yaw_KP;
  double yaw_KI;
  double yaw_KD;
  double yaw_target;

  double roll_des, pitch_des, yaw_des, thrust_des;

};

class AttitudeControllerParameters {
 public:

  Eigen::Matrix4Xd allocation_matrix_;
  Eigen::Vector3d attitude_gain_;
  Eigen::Vector3d angular_rate_gain_;

};

class AttitudeController {
 public:
  AttitudeController();
  ~AttitudeController();

  void InitializeParameters(const ros::NodeHandle& pnh);

  void CalculateAttitudeControl(mav_msgs::CommandRollPitchYawrateThrust control_cmd_input, sensor_msgs::Imu current_imu, Eigen::VectorXd* des_rate_output);

  void CalculateRateControl(mav_msgs::CommandRollPitchYawrateThrust control_cmd_input, sensor_msgs::Imu current_imu, Eigen::VectorXd des_rate_input, Eigen::VectorXd* des_control_output);

  void CalculateMotorCommands(Eigen::VectorXd control_inputs, Eigen::VectorXd* des_rotor_velocities_output);

  rotors_control::VehicleParameters vehicle_parameters_;
  mav_msgs::CommandRollPitchYawrateThrust current_control_cmd_;
  sensor_msgs::Imu current_imu_;
  Eigen::VectorXd desired_angular_rates;
  Eigen::VectorXd desired_control_cmds;
  Eigen::VectorXd desired_motor_velocities;

 private:

  //General
  tf::Quaternion q;
  double meas_roll, meas_pitch, meas_yaw;

  ros::Time last_time;
  ros::Time sim_time;
  double dt;

  ControllerUtility controller_utility_;

  //Attitude Controller
  double roll_er, pitch_er, yaw_er;
  double roll_er_sum, pitch_er_sum, yaw_er_sum;
  double cp, ci, cd;

  //Roll PID
  double roll_KI_max;
  double roll_KP;
  double roll_KI;
  double roll_KD;

  //Pitch PID
  double pitch_KI_max;
  double pitch_KP;
  double pitch_KI;
  double pitch_KD;

  //Yaw PID
  double yaw_KI_max;
  double yaw_KP;
  double yaw_KI;
  double yaw_KD;
  double yaw_target;

  double p_des, q_des, r_des;

  //Rate Controller
  double p_er, q_er, r_er;
  double p_er_sum, q_er_sum, r_er_sum;
  
  //P Controller
  double p_KI_max;
  double p_KP;
  double p_KI;
  double p_KD;
  double x_ang_acc;
  double last_ang_vel_x;

  //Q Controller
  double q_KI_max;
  double q_KP;
  double q_KI;
  double q_KD;
  double y_ang_acc;
  double last_ang_vel_y;

  //R Controller
  double r_KI_max;
  double r_KP;
  double r_KI;
  double r_KD;
  double yaw_vel_target;
  double z_ang_acc;
  double last_ang_vel_z;

  double U1, U2, U3, U4;

  //Motor Mapping 
  double KT;
  double Kd;
  double l;
  double motor_lim;
  double w1, w2, w3, w4;

};

}


