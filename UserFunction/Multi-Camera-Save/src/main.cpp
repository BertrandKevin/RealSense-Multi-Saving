//
// main.cpp
// Intel Camera
//
// created by Kevin Bertrand on 17/08/18
// Copyright © 2018 KevinBertrand. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////////////
//				Include librairies				//
//////////////////////////////////////////////////////////////////////////////////
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "../includes/functions.hpp"
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#include "../includes/api_how_to.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <librealsense2/rs.hpp>
#include <librealsense2/rs_advanced_mode.hpp>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/time.h>
#include <vector>

//////////////////////////////////////////////////////////////////////////////////
// 			    Global variables declaration			//
//////////////////////////////////////////////////////////////////////////////////
const std::string noCameraMessage = "No camera connected, please connect at least 1 new camera";
const std::string platform_camera_name = "Platform Camera";
std::vector<rs2::pipeline> cameras;

//////////////////////////////////////////////////////////////////////////////////
//				      Functions					//
//////////////////////////////////////////////////////////////////////////////////
//Date and time formating for the path
std::string dateFormatting(std::string dateRaw)
{
    	// Delete the "\n" at the end of the string
	dateRaw.erase(dateRaw.length()-1, 1);

    	std::string day = "";
    	if(dateRaw[8] == ' ')
    	{
    		day += '0';
    	}
    	else
    	{
     		day += dateRaw[8];
    	}
    	day += dateRaw[9];

    	std::string month = "";
    	switch(dateRaw[4])
    	{
        	case 'J':
            		switch(dateRaw[5])
        		{
            			case 'a': month = "01";
                			break;
            			case 'u':
                			switch(dateRaw[6])
            				{
                				case 'n': month = "06";
                    					break;
                				case 'l': month = "07";
                    					break;
                				default: month = "00";
            				}
                			break;
            			default: month = "00";
        		}
            		break;
        	case 'F': month = "02";
            		break;
        	case 'M':
            		switch(dateRaw[6])
        		{
            			case 'r': month = "03";
                			break;
            			case 'y': month = "05";
                			break;
            			default: month = "00";
        		}
            		break;
        	case 'A':
            		switch(dateRaw[5])
        		{
            			case 'p': month = "04";
                			break;
            			case 'u': month = "08";
                			break;
            			default: month ="00";
        		}
            		break;
        	case 'S': month = "09";
            		break;
        	case 'O': month = "10";
            		break;
        	case 'N': month = "11";
            		break;
        	case 'D': month = "12";
            		break;
        	default: month = "00";
    	}

    	std::string year = "";
    	year += dateRaw[20];
    	year += dateRaw[21];
    	year += dateRaw[22];
    	year += dateRaw[23];

    	std::string hour = "";
    	hour += dateRaw[11];
    	hour += dateRaw[12];

    	std::string minute = "";
    	minute += dateRaw[14];
    	minute += dateRaw[15];

    	std::string second = "";
    	second += dateRaw[17];
    	second += dateRaw[18];

    	std::string dateFormated = year;
    	dateFormated += "-" + month;
    	dateFormated += "-" + day;
    	dateFormated += "_" + hour;
    	dateFormated += "h" + minute;
    	dateFormated += "min" + second;
    	dateFormated += "sec";

    	return dateFormated;
}

//Create the CSV file with metadata
void metadata_to_csv(const rs2::frame& frm, const std::string& filename)
{
	std::ofstream csv;

	csv.open(filename);

	csv << "Stream," << rs2_stream_to_string(frm.get_profile().stream_type()) << "\nMetadata Attribute,Value\n";

	for(size_t i = 0; i < RS2_FRAME_METADATA_COUNT; i++)
	{
		if(frm.supports_frame_metadata((rs2_frame_metadata_value)i))
		{
			csv << rs2_frame_metadata_to_string((rs2_frame_metadata_value)i) << "," << frm.get_frame_metadata((rs2_frame_metadata_value)i) << "\n";
		}
	}

	csv.close();

	chmod(filename.c_str(), 0777);
}

//Create the dictory to store all files
std::string createDirectory(std::string dateTime)
{
	std::string directoryPath;

	int status;

	for(int tmp = 0; tmp < 2; tmp++)
	{
		if(tmp == 0)
		{
			directoryPath = "/home/upsquared/Documents/Cameras/pictures/";
			directoryPath += dateTime;
			
		}
		else
		{
			directoryPath = "/home/upsquared/Documents/Cameras/ply/";
			directoryPath += dateTime;
		}
		std::cout << directoryPath << std::endl;
		status = mkdir(directoryPath.c_str(), 0777);

		if(status == -1)
		{
			std::cout << "Folder not created" << std::endl;
		}
	}

	directoryPath = "/home/upsquared/Documents/Cameras/"; 

	return directoryPath;
}

//Save images
void saveImages(rs2::frame frame, std::string directoryPath, std::string directoryTime, int numberCamera, std::string dateTime)
{
	auto vf = frame.as<rs2::video_frame>();

	if(vf.is<rs2::depth_frame>())
	{
		rs2::colorizer color_map;

		vf = color_map.process(frame);
	}

	std::stringstream png_file;
	png_file << directoryPath << "pictures/" << directoryTime << "/Camera " << std::to_string(numberCamera) << " - " << dateTime << " - " << vf.get_profile().stream_name() << ".png";
	chmod(png_file.str().c_str(), 0777);

	stbi_write_png(png_file.str().c_str(), vf.get_width(), vf.get_height(), vf.get_bytes_per_pixel(), vf.get_data(), vf.get_stride_in_bytes());

	std::stringstream csv_file;
	csv_file << directoryPath << "pictures/" << directoryTime << "/Camera " << std::to_string(numberCamera) << " - " << dateTime << " - " << vf.get_profile().stream_name() << "-metadata.csv";
	metadata_to_csv(vf, csv_file.str());
}

//Get images from each cameras
void getImages( std::string directoryPath, std::string directoryTime, std::vector<rs2::device> devicesTab)
{
	std::vector<std::string> pngFiles;
	std::vector<std::string> csvFiles;
	std::vector<rs2::frame> frameFiles;
	std::vector<rs2::frameset> framesTab;
	std::vector<std::string> pathPly;
	std::vector<std::string> timeframes;
	std::vector<int> numCameras;

	int nbImages(0);
	int sizeVector(cameras.size());
	int i(0);
	int j(0);

	// Capture 150 frames to give autoexposure, etc. a chance to settle 
	for(i = 0; i < 150; i++)
	{
		for(j = 0; j < sizeVector; j++)
		{
			cameras[j].wait_for_frames();
		}
	}

	while(1)
	{
		for(j = 0; j < sizeVector; j++)
		{
			// Get frames
			auto frames = cameras[j].wait_for_frames();
			framesTab.push_back(frames);

			// Search time of the frame
			timeval curTime;
			gettimeofday(&curTime, NULL);
			int milli = curTime.tv_usec / 1000;

			char buffer [80];
			strftime(buffer, 80, "%Y-%m-%d_%Hh%Mmin%Ssec", localtime(&curTime.tv_sec));

			char currentTime[84] = "";
			sprintf(currentTime, "%s:%d", buffer, milli);
			timeframes.push_back(currentTime);

			// Save camera's number
			numCameras.push_back(j+1);

			// Save time
			time_t startProgram = time(0);
    			char* temp = ctime(&startProgram);
			std::string dateRaw = temp;
			std::string dateTime;
			dateTime = dateFormatting(dateRaw);

			std::string path;
			path = directoryPath + "ply/";
			path += directoryTime + "/Camera ";
			path += std::to_string(j+1) + " - ";
			path += dateTime + ".ply";
			pathPly.push_back(path);
		}

		nbImages++;

		if(nbImages >= 15)
		{
			break;
		}
	}

	// Save PLY files
	rs2::frameset frames;
	rs2::pointcloud pc;
	rs2::points pointsImage;

	for(j = framesTab.size() - sizeVector; j < framesTab.size(); j++)
	{
		frames = framesTab[j];

		// Get colot and depth frames
		rs2::frame depth = frames.get_depth_frame();
		rs2::frame color = frames.get_color_frame();
		auto infrared = frames.get_infrared_frame(1);
		std::string dateTime = timeframes[j];

		// Save pointcloud
		for(int k = 0; k < 15; k++)
		{
			depth = frames.get_depth_frame();
			color = frames.get_color_frame();
			pc.map_to(color);
			pointsImage = pc.calculate(depth);
		}
		pointsImage.export_to_ply(pathPly[j], color);
		chmod(pathPly[j].c_str(), 0777);

		std::cout << "Saved PLY - camera: " << std::to_string(numCameras[j]) << std::endl;

		// Save color frame
		saveImages(color, directoryPath, directoryTime, numCameras[j], dateTime);
		std::cout << "Saved color frame - camera: " << std::to_string(numCameras[j]) << std::endl;

		// Save RGB frame
		saveImages(depth, directoryPath, directoryTime, numCameras[j], dateTime);

		std::cout << "Saved depth frame - camera: " << std::to_string(numCameras[j]) << std::endl;

		// Save infrared frame
		saveImages(infrared, directoryPath, directoryTime, numCameras[j], dateTime);

		std::cout << "Saved infrared frames - camera: " << std::to_string(numCameras[j]) << std::endl;
	}
}

class device_container
{
	struct view_port
	{
		std::map<int, rs2::frame> frames_per_stream;
		rs2::colorizer colorize_frame;
		texture tex;
		rs2::pipeline pipe;
		rs2::pipeline_profile profile;
	};

	public:
		void enable_device(rs2::device dev)
		{
			std::string serial_number(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
			std::lock_guard<std::mutex> lock(_mutex);

			rs2::colorizer color_map;

			if(_devices.find(serial_number) != _devices.end())
			{
				return;
			}

			// Ignoring platform cameras (webcams, etc..)
			if(platform_camera_name == dev.get_info(RS2_CAMERA_INFO_NAME))
			{
				return;
			}

			// Create a pipeline from the given device
			rs2::pipeline p;
			rs2::config c;
        		c.enable_device(serial_number);

			// Configuration of frames
			c.enable_stream(RS2_STREAM_COLOR, 1280, 720, RS2_FORMAT_RGB8);
			c.enable_stream(RS2_STREAM_DEPTH,  1280, 720);
			c.enable_stream(RS2_STREAM_INFRARED, 1, 1280, 720, RS2_FORMAT_Y8, 30);

			// Start the pipeline with the configuration
			rs2::pipeline_profile profile = p.start(c);

			// Save each pipeline on a vector
			cameras.push_back(p);

			return;
		}

		void remove_devices(const rs2::event_information& info)
		{
			std::lock_guard<std::mutex> lock(_mutex);

			// Go over the list of devices and check if it was disconnected
			auto itr = _devices.begin();
			while(itr != _devices.end())
			{
				if(info.was_removed(itr->second.profile.get_device()))
				{
					itr = _devices.erase(itr);
				}
				else
				{
					++itr;
				}
			}
		}

	private:
		std::mutex _mutex;
		std::map<std::string, view_port> _devices;
};

int main() try
{
	device_container connected_devices;
	rs2::context ctx;

	std::string directoryPath = "";

	time_t startProgram = time(0);
    	char* temp = ctime(&startProgram);
    	std::string dateRaw = temp;
	std::string directoryTime = dateFormatting(dateRaw);

	directoryPath = createDirectory(directoryTime);

	// Register callback for tracking which devices are currently connected
	ctx.set_devices_changed_callback([&](rs2::event_information& info)
	{
		connected_devices.remove_devices(info);
	});

	// Initial populget_a_sensor_from_a_deviceation of the device list
	for(auto&& dev : ctx.query_devices())
	{
		connected_devices.enable_device(dev);
	}

	rs2::device_list devices = ctx.query_devices();
	std::vector<rs2::device> devicesTab;

	for (rs2::device device : devices)
        {
		devicesTab.push_back(device);
	}

	getImages(directoryPath, directoryTime, devicesTab);

	for(int j = 0; j < cameras.size(); j++)
	{
		cameras[j].stop();
	}

	return EXIT_SUCCESS;
}
catch(const rs2::error & e)
{
    	std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;

	for(int j = 0; j < cameras.size(); j++)
	{
		cameras[j].stop();
	}

    	return EXIT_FAILURE;
}
catch(const std::exception & e)
{
    	std::cerr << e.what() << std::endl;

	for(int j = 0; j < cameras.size(); j++)
	{
		cameras[j].stop();
	}

    	return EXIT_FAILURE;
}
