# RealSense Multi-Saving on an UpSquared with Ubuntu 18 LTS
## Information
__*For this project, you need to create and use a username "upsquared".*__

## Dowloading the software
1. Navigate to the "Home" folder: `cd ~/`
2. Intall the git librairy: `sudo apt-get install git`
2. Download the repository "RealSense-Multi-Saving-master" on the UpSquared: `sudo git clone https://github.com/BertrandKevin/RealSense-Multi-Saving.git`

## Configuration of the UpSquared
1. You need to register the server's public key: `sudo apt-key adv --keyserver keys.gnupg.net --recv-key C8B3A55A6F3EFCDE || sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-key C8B3A55A6F3EFCDE`
2. You need to add the server to the list of repositories: `sudo add-apt-repository "deb http://realsense-hw-public.s3.amazonaws.com/Debian/apt-repo bionic main" -u`
3. Then, you have to install some librairies:
   - `sudo apt-get install librealsense2-dkms`
   - `sudo apt-get install librealsense2-utils`
   - `sudo apt-get install librealsense2-dev`
   - `sudo apt-get install librealsense2-dbg`
   - `sudo apt-get install cmake`
4. Add permissions to the "RealSense-Multi-Saving" folder: `sudo chmod 777 RealSense-Multi-Saving/`
5. Final, update/upgrade the Up : `sudo apt-get update && sudo apt-get upgrade && sudo apt-get dist-upgrade`
5. Final, update/upgrade the Up : `sudo apt-get update && sudo apt-get upgrade && sudo apt-get dist-upgrade`

## Installation of the software
1. Go to the software repository: `cd ~/RealSense-Multi-Saving`
2. Install some package: 
   - `sudo apt-get install git libssl-dev libusb-1.0-0-dev pkg-config libgtk-3-dev`
   - `sudo apt-get install libglfw3-dev libgl1-mesa-dev libglu1-mesa-dev`
3. Install permissions:
   - `sudo chmod 777 scripts/setup_udev_rules.sh`
   - `./scripts/setup_udev_rules.sh`
4. Install Ubuntu 18.04 LTS patch:
   - `sudo chmod 777 scripts/patch-realsense-ubuntu-lts.sh`
   - `./scripts/patch-realsense-ubuntu-lts.sh`
5. Check if the gcc is correct: `gcc -v`

   -__If the last line shows "gcc version 5.5.0", you can skip the following hyphen and directly go to the 6th point.__
   
   -__If the last line doen't show "gcc version 5.5.0", enter these follwing command lines:__
      - `sudo add-apt-repository ppa:ubuntu-toolchain-r/test`
      - `sudo apt-get update`
      - `sudo apt-get install gcc-5 g++-5`
      - `sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 60 --slave /usr/bin/g++ g++ /usr/bin/g++-5`
      - `sudo update-alternatives --set gcc "/usr/bin/gcc-5"`
6. Create the storing folder for the pictures:
   - `sudo mkdir ~/Documents/Cameras`
   - `sudo mkdir ~/Documents/Cameras/ply && sudo mkdir ~/Documents/Cameras/pictures`
7. Create the build folder:
   `sudo mkdir build && cd build`
8. Run Cmake:
   `sudo cmake ../ -DBUILD_EXAMPLES=true`
9. Compile the software (this may take some times):
   `sudo make uninstall && sudo make clean && sudo  make -j4 && sudo make install`

## Starting the software
To start the software, you have to open a new Terminal Window and enter:
`multi-saving`
If there is at least a camera connected, the software will start and save all pictures/data.

## How to see the pictures/data?
To see the pictures and the data, you have to go to your Documents and then you will find a folder named "Cameras". In this folder, you will find 2 other folder:

   -__*ply*__: In this folder, you will find all 3D images 
   
   -__*pictures*__: In this folder, you will find all colored pictures, infrared pictures and depth pictures.
