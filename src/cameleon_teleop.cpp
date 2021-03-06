#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <sensor_msgs/Joy.h>

  ros::Publisher vel_pub_;
  geometry_msgs::Twist twist;

  bool teleoperated = false;
  double forwardSpeed = 0;
  double forwardAcceleration=- 0;
  int linearAxis = 1;
  int angularAxis = 0;
  int flipperAxis = 4;
  double linearGain = 0.2;
  double angularGain = 0.2;
  double flipperGain = 0.2;

  ros::Subscriber joy_sub_;
  ros::Subscriber cmd_sub_;
  void cmdCallback(const geometry_msgs::Twist::ConstPtr& cmd);
  void joyCallback(const sensor_msgs::Joy::ConstPtr& joy);

void cmdCallback(const geometry_msgs::Twist::ConstPtr& cmd)
{
  if (teleoperated == false) twist=*cmd;
}

void joyCallback(const sensor_msgs::Joy::ConstPtr& joy)
{     
       	//if swiching modes, then clear velocities	
	if (teleoperated != joy->buttons[4]){
		twist.angular.z = 0.0;
		twist.linear.x = 0.0;
		twist.angular.y = 0.0;
		forwardAcceleration = 0;
		forwardSpeed = 0;
	}

	//is it teleoperated ?
	teleoperated = (joy->buttons[4] == 1);

	//if yes, do the driving
	if (teleoperated == true)
	{
		twist.angular.z = angularGain*joy->axes[angularAxis];
		forwardAcceleration = 0.05*joy->axes[linearAxis];;
		twist.angular.y = flipperGain*joy->axes[flipperAxis];
		ROS_INFO( "%i %i %i ",flipperAxis,angularAxis,linearAxis);
		ROS_INFO( "Rychlost z= %f, Rychlost x= %f ",  twist.linear.z,twist.linear.x);
	}
}

 main(int argc, char** argv)
{
	ros::init(argc, argv, "teleop_turtle");
	ros::NodeHandle nh;

	nh.param("axis_linear", linearAxis, 1);
	nh.param("axis_angular", angularAxis, 0);
	nh.param("axis_flipper", flipperAxis, 4);
	nh.param("scale_angular", angularGain, 0.2);
	nh.param("scale_linear", linearGain, 0.2);
	nh.param("scale_flipper", flipperGain, 0.2);

	vel_pub_ = nh.advertise<geometry_msgs::Twist>("cameleon/cmd_vel", 1);
	joy_sub_ = nh.subscribe<sensor_msgs::Joy>("joy", 10, joyCallback);
	cmd_sub_ = nh.subscribe<geometry_msgs::Twist>("cmd", 10, cmdCallback);

	while (ros::ok()){
		ros::spinOnce();
		if (teleoperated){
			forwardSpeed += forwardAcceleration;
			forwardSpeed = fmin(fmax(forwardSpeed,-1.0),1.0);
			twist.linear.x =  forwardSpeed*linearGain;;
		}
		vel_pub_.publish(twist);
		usleep(50000);
	}
}
