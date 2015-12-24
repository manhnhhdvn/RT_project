####Description:

    Stream video use mjpeg
    Control kobuki robot with keyop

####Command:

Change your current folder to catkin folder and run:

    $ catkin_make install

If error packet gstreamer-app-0.10 not found, you can run this command (Ubuntu 14.04 32bit):

    $ sudo apt-get install libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev

Continue run this command

    $ roslaunch gscam YOUR_LAUNCH_FILE

And open new terminal and run

    $ rosrun mjpeg_server mjpeg_server 

Open your browser and go this link

    http://localhost:8080/stream?topic=/YOUR_IMAGE_TOPIC
