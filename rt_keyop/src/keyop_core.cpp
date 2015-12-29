#include <ros/ros.h>
#include <ecl/time.hpp>
#include <ecl/exceptions.hpp>
#include <std_srvs/Empty.h>
#include <kobuki_msgs/MotorPower.h>
#include "../include/keyop_core/keyop_core.hpp"


#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/select.h>
#include <poll.h>

#include <ros/ros.h>
#include <arpa/inet.h>

#define MAX_SIZE 2000
#define TIME_OUT 10000
#define MAX_CONN 1024

namespace keyop_core {

KeyOpCore::KeyOpCore() :
		last_zero_vel_sent(true), // avoid zero-vel messages from the beginning
		accept_incoming(true), power_status(false), wait_for_connection_(true), cmd(
				new geometry_msgs::Twist()), cmd_stamped(
				new geometry_msgs::TwistStamped()), linear_vel_step(0.1), linear_vel_max(
				3.4), angular_vel_step(0.02), angular_vel_max(1.2), quit_requested(
				false), key_file_descriptor(0) {
	tcgetattr(key_file_descriptor, &original_terminal_state); // get terminal properties
}

KeyOpCore::~KeyOpCore() {
	tcsetattr(key_file_descriptor, TCSANOW, &original_terminal_state);
}

bool KeyOpCore::init() {
	ros::NodeHandle nh("~");

	name = nh.getUnresolvedNamespace();

	nh.getParam("linear_vel_step", linear_vel_step);
	nh.getParam("linear_vel_max", linear_vel_max);
	nh.getParam("angular_vel_step", angular_vel_step);
	nh.getParam("angular_vel_max", angular_vel_max);
	nh.getParam("wait_for_connection", wait_for_connection_);
	nh.getParam("ip_robot", ip_robot);

	ROS_INFO_STREAM(
			"KeyOpCore : using linear  vel step [" << linear_vel_step << "].");
	ROS_INFO_STREAM(
			"KeyOpCore : using linear  vel max  [" << linear_vel_max << "].");
	ROS_INFO_STREAM(
			"KeyOpCore : using angular vel step [" << angular_vel_step << "].");
	ROS_INFO_STREAM(
			"KeyOpCore : using angular vel max  [" << angular_vel_max << "].");

	keyinput_subscriber = nh.subscribe("teleop", 1,
			&KeyOpCore::remoteKeyInputReceived, this);

	velocity_publisher_ = nh.advertise<geometry_msgs::Twist>("cmd_vel", 1);
	motor_power_publisher_ = nh.advertise<kobuki_msgs::MotorPower>(
			"motor_power", 1);

	cmd->linear.x = 0.0;
	cmd->linear.y = 0.0;
	cmd->linear.z = 0.0;
	cmd->angular.x = 0.0;
	cmd->angular.y = 0.0;
	cmd->angular.z = 0.0;

	if (!wait_for_connection_) {
		return true;
	}
	ecl::MilliSleep millisleep;
	int count = 0;
	bool connected = false;
	while (!connected) {
		if (motor_power_publisher_.getNumSubscribers() > 0) {
			connected = true;
			break;
		}
		if (count == 6) {
			connected = false;
			break;
		} else {
			ROS_WARN_STREAM(
					"KeyOp: could not connect, trying again after 500ms...");
			try {
				millisleep(500);
			} catch (ecl::StandardException &e) {
				ROS_ERROR_STREAM("Waiting has been interrupted.");
				ROS_DEBUG_STREAM(e.what());
				return false;
			}
			++count;
		}
	}
	if (!connected) {
		ROS_ERROR("KeyOp: could not connect.");
		ROS_ERROR("KeyOp: check remappings for enable/disable topics).");
	} else {
		kobuki_msgs::MotorPower power_cmd;
		power_cmd.state = kobuki_msgs::MotorPower::ON;
		motor_power_publisher_.publish(power_cmd);
		ROS_INFO("KeyOp: connected.");
		power_status = true;
	}

	// start keyboard input thread
	thread.start(&KeyOpCore::keyboardInputLoop, *this);
	return true;
}

void KeyOpCore::spin() {
	ros::Rate loop_rate(10);

	while (!quit_requested && ros::ok()) {
		// Avoid spamming robot with continuous zero-velocity messages
		if ((cmd->linear.x != 0.0) || (cmd->linear.y != 0.0)
				|| (cmd->linear.z != 0.0) || (cmd->angular.x != 0.0)
				|| (cmd->angular.y != 0.0) || (cmd->angular.z != 0.0)) {
			velocity_publisher_.publish(cmd);
			last_zero_vel_sent = false;
		} else if (last_zero_vel_sent == false) {
			velocity_publisher_.publish(cmd);
			last_zero_vel_sent = true;
		}
		accept_incoming = true;
		ros::spinOnce();
		loop_rate.sleep();
	}
	if (quit_requested) { // ros node is still ok, send a disable command
		disable();
	} else {
		// just in case we got here not via a keyboard quit request
		quit_requested = true;
		thread.cancel();
	}
	thread.join();
}

void KeyOpCore::keyboardInputLoop() {
	struct termios raw;
	memcpy(&raw, &original_terminal_state, sizeof(struct termios));

	raw.c_lflag &= ~(ICANON | ECHO);
	// Setting a new line, then end of file
	raw.c_cc[VEOL] = 1;
	raw.c_cc[VEOF] = 2;
	tcsetattr(key_file_descriptor, TCSANOW, &raw);

	puts("READING MESSAGE FROM CLIENT!");
	puts("---------------------------");
	puts("Forward/back arrows : linear velocity incr/decr.");
	puts("Right/left arrows : angular velocity incr/decr.");
	puts("Spacebar : reset linear/angular velocities.");
	puts("d : disable motors.");
	puts("e : enable motors.");
	puts("--------------------");
	puts("w : Go straight.");
	puts("s : Go back.");
	puts("a : Turn left.");
	puts("d : Turn right.");
	puts("space : Stop.");
	puts("--------------------");
	puts("q : quit.");
	char c;




	// while (!quit_requested) {
	// 	if (read(key_file_descriptor, &c, 1) < 0) {
	// 		perror("read char failed():");
	// 		exit(-1);
	// 	}
	// 	processKeyboardInput(c);
	// }

  int sockfd,acceptfd,fd;
  int choose;
  int countTotalRecvData[MAX_CONN], countTotalSendData[MAX_CONN], countRecvData=0, countSendData=0;
  struct sockaddr_in server,clients;
  socklen_t socksize=sizeof(struct sockaddr_in);
  char * message;
  // char *message=malloc(MAX_SIZE);
  message = (char*) malloc (MAX_SIZE);
  strcpy(message,"");
  struct pollfd client[MAX_CONN];
  int max_client;
  int i,j;
  int numberClient=0;
  int activity;
  printf("Creat a socket, please wait...\n");
  if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1){
    printf("Socket retrieve failed!\n");
    exit(1);
  }else printf("Socket retrieve success!...\n");
  memset(&server,'0',sizeof(server));
  server.sin_family=AF_INET;
  server.sin_addr.s_addr = inet_addr(ip_robot.c_str());
  server.sin_port=htons(5678);
  printf("Bind the socket, please wait...\n");
  if(bind(sockfd,(struct sockaddr*)&server,sizeof(server))==-1){
    printf("Bind failed...\n");
    exit(1);
  }else  printf("Bind done...\n");
  printf("Listen on the socket, please wait...\n");
  if((listen(sockfd,1))==-1){
    printf("Failed to listen\n");
    exit(1);
  }else printf("Success to listen\n");
  client[0].fd=sockfd;
  client[0].events=POLLIN;
  max_client=0;
  for(i=1;i<MAX_CONN;i++)
    {
      client[i].fd=-1;
      countTotalSendData[i]=0;
      countTotalRecvData[i]=0;
    }
  printf("Ready to communicate with client.\n");
  for(;;){
    activity=poll(client,max_client+1,TIME_OUT);
    if(activity<0)
      {
	printf("Poll failed!\n");
	exit(1);
      }
    if(client[0].revents&POLLIN)
      {
	acceptfd=accept(sockfd,(struct sockaddr *)&clients,&socksize);
	//countTotalSendData=0;
	//countTotalRecvData=0;
	if(acceptfd==-1){
	  printf("Accept failed!\n");
	  exit(1);
	}else{
	  numberClient++;
	  printf("Accepted request! Having %d client(s) connect to server in this time.\n",numberClient);
	}
	for(i=1;i<=MAX_CONN;i++)
	  {
	    if(client[i].fd==-1)
	      {
		client[i].fd=acceptfd;
		break;
	      }
	  }
	if(i==MAX_CONN)
	  {
	    printf("FULL SLOTS.\n");
	    exit(1);
	  }
	client[i].events=POLLIN;
	if(i>max_client) max_client=i;
	if(--activity<=0) continue;
      }
    for(j=1;j<=max_client;j++)
      {
	if((fd=client[j].fd)==-1) continue;
	if(client[j].revents & (POLLIN | POLLERR))
	  {
	    //while(1){
	    // message=malloc(MAX_SIZE);
	    message = (char*) malloc (MAX_SIZE);
	    strcpy(message,"");
	    countRecvData=recv(fd,message,2000,0);
	    printf("Message from client in port %d: %s\n",fd,message);
	    
	    
	    if (strcmp(message,"Up") == 0)
	    		c = kobuki_msgs::KeyboardInput::KeyCode_Up;
	    if (strcmp(message,"Down") == 0)
	    		c = kobuki_msgs::KeyboardInput::KeyCode_Down;
	    if (strcmp(message,"Left") == 0)
	    		c = kobuki_msgs::KeyboardInput::KeyCode_Left;
	    if (strcmp(message,"Right") == 0)
	    		c = kobuki_msgs::KeyboardInput::KeyCode_Right;
	    if (strcmp(message,"Space") == 0)
	    		c = kobuki_msgs::KeyboardInput::KeyCode_Space;
	    if (strcmp(message,"Disable motors") == 0)
	    		c = 'r';
	    if (strcmp(message,"Enable motors") == 0)
	    		c = 'e';
	    if (strcmp(message,"Quit") == 0)
	    		c = 'q';
	    

	    processKeyboardInput(c);
	    if(countRecvData==-1){
	      printf("Receive data failed!\n");
	      shutdown(fd,2);shutdown(sockfd,2);
	      exit(1);
	    }else if((strcmp(message,"q")==0)||(strcmp(message,"Q")==0)){
	      c = kobuki_msgs::KeyboardInput::KeyCode_Space;	
	      processKeyboardInput(c);
	      numberClient--;
	      printf("Client in port %d disconnected! Having %d client(s) connect to server in this time. \n",fd,numberClient);
	      printf("Total size of data received from port %d: %d\n",fd,countTotalRecvData[j]);
	      printf("Total size of data sent to port %d: %d\n",fd,countTotalSendData[j]);
	      shutdown(fd,2);
	      client[j].fd=-1;
	      if(numberClient==0){
		do{
		  printf("Do you want to keep server working?\n");
		  printf("1. Yes\n");
		  printf("2. No\n");
		  scanf("%d",&choose);
		  if(choose!=1&&choose!=2) printf("Input failed!\n");
		}while(choose!=1&&choose!=2);
		if(choose==1){
		  printf("Server is working!\n");
		  continue;
		}
		else if(choose==2){
		  printf("Server is closed!\n");	  
		  shutdown(sockfd,2);
		  exit(0);
		}
	      }
	    }else{
	      countTotalRecvData[j]=countTotalRecvData[j]+countRecvData;
	      countSendData=send(fd,message,strlen(message),0);
	      // message=malloc(MAX_SIZE);
	      message = (char*) malloc (MAX_SIZE);
	      strcpy(message,"");
	      if(countSendData==-1){
		printf("Send data failed!\n");
		shutdown(fd,2);shutdown(sockfd,2);
		exit(1);
	      }else{
		countTotalSendData[j]=countTotalSendData[j]+countSendData;
	      }
	    }
	    if(--activity<=0)break;
	  }
      }
  }




}

void KeyOpCore::remoteKeyInputReceived(const kobuki_msgs::KeyboardInput& key) {
	processKeyboardInput(key.pressedKey);
}

void KeyOpCore::processKeyboardInput(char c) {

	switch (c) {
	case kobuki_msgs::KeyboardInput::KeyCode_Left: {
		incrementAngularVelocity();
		break;
	}
	case kobuki_msgs::KeyboardInput::KeyCode_Right: {
		decrementAngularVelocity();
		break;
	}
	case kobuki_msgs::KeyboardInput::KeyCode_Up: {
		incrementLinearVelocity();
		break;
	}
	case kobuki_msgs::KeyboardInput::KeyCode_Down: {
		decrementLinearVelocity();
		break;
	}
	case kobuki_msgs::KeyboardInput::KeyCode_Space: {
		resetVelocity();
		break;
	}
	case 'q': {
		quit_requested = true;
		break;
	}
	case 'r': {
		disable();
		break;
	}
	case 'e': {
		enable();
		break;
	}
	default: {
		break;
	}
	}
}

void KeyOpCore::disable() {
	cmd->linear.x = 0.0;
	cmd->angular.z = 0.0;
	velocity_publisher_.publish(cmd);
	accept_incoming = false;

	if (power_status) {
		ROS_INFO(
				"KeyOp: die, die, die (disabling power to the device's motor system).");
		kobuki_msgs::MotorPower power_cmd;
		power_cmd.state = kobuki_msgs::MotorPower::OFF;
		motor_power_publisher_.publish(power_cmd);
		power_status = false;
	} else {
		ROS_WARN("KeyOp: Motor system has already been powered down.");
	}
}

void KeyOpCore::enable() {
	accept_incoming = false;

	cmd->linear.x = 0.0;
	cmd->angular.z = 0.0;
	velocity_publisher_.publish(cmd);

	if (!power_status) {
		ROS_INFO("KeyOp: Enabling power to the device subsystem.");
		kobuki_msgs::MotorPower power_cmd;
		power_cmd.state = kobuki_msgs::MotorPower::ON;
		motor_power_publisher_.publish(power_cmd);
		power_status = true;
	} else {
		ROS_WARN("KeyOp: Device has already been powered up.");
	}
}

void KeyOpCore::incrementLinearVelocity() {
	if (power_status) {
		if (cmd->linear.x <= linear_vel_max) {
			cmd->linear.x += linear_vel_step;
		}
		ROS_INFO_STREAM(
				"KeyOp: linear  velocity incremented [" << cmd->linear.x << "|" << cmd->angular.z << "]");
	} else {
		ROS_WARN_STREAM("KeyOp: motors are not yet powered up.");
	}
}

void KeyOpCore::decrementLinearVelocity() {
	if (power_status) {
		if (cmd->linear.x >= -linear_vel_max) {
			cmd->linear.x -= linear_vel_step;
		}
		ROS_INFO_STREAM(
				"KeyOp: linear velocity decremented [" << cmd->linear.x << "|" << cmd->angular.z << "]");
	} else {
		ROS_WARN_STREAM("KeyOp: motors are not yet powered up.");
	}
}

void KeyOpCore::incrementAngularVelocity() {
	if (power_status) {
		if (cmd->angular.z <= angular_vel_max) {
			cmd->angular.z += angular_vel_step;
		}
		ROS_INFO_STREAM(
				"KeyOp: angular velocity incremented [" << cmd->linear.x << "|" << cmd->angular.z << "]");
	} else {
		ROS_WARN_STREAM("KeyOp: motors are not yet powered up.");
	}
}

void KeyOpCore::decrementAngularVelocity() {
	if (power_status) {
		if (cmd->angular.z >= -angular_vel_max) {
			cmd->angular.z -= angular_vel_step;
		}
		ROS_INFO_STREAM(
				"KeyOp: angular velocity decremented [" << cmd->linear.x << "|" << cmd->angular.z << "]");
	} else {
		ROS_WARN_STREAM("KeyOp: motors are not yet powered up.");
	}
}

void KeyOpCore::resetVelocity() {
	if (power_status) {
		cmd->angular.z = 0.0;
		cmd->linear.x = 0.0;
		ROS_INFO_STREAM("KeyOp: reset linear/angular velocities.");
	} else {
		ROS_WARN_STREAM("KeyOp: motors are not yet powered up.");
	}
}

}
