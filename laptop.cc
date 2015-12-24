#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
using namespace std;

int main()
{
  system("gnome-terminal -x sh -c 'cd /home/GR/catkin_ws/wandrian ; export ROS_MASTER_URI=http://localhost:11311 ; roslaunch rt_keyop keyop.launch'");
  cout << "Done\n";
  return 0;
}
