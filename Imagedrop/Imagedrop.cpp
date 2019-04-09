/*
  _____                                _
 |_   _|                              | |
   | |  _ __ ___   __ _  __ _  ___  __| |_ __ ___  _ __
   | | | '_ ` _ \ / _` |/ _` |/ _ \/ _` | '__/ _ \| '_ \
  _| |_| | | | | | (_| | (_| |  __/ (_| | | | (_) | |_) |
 |_____|_| |_| |_|\__,_|\__, |\___|\__,_|_|  \___/| .__/
						 __/ |                    | |
						|___/                     |_|
	Written by: Muhammad A.Moniem (@_mamoniem)
				All rights reserved
						© 2019
*/

/*
The usage:
#EXE
	- You can drag and drop an image into the exe file.
		- This will create a half size image version of it.
		- New Image name will be the same as the old one, with a suffix.
		- New generated images can be anything but the half size by changing the default const value.

#Commandline
	After adding the exe to the PATH environment variables
	- You can launch the exe the same way requested within the assignment document, by passing old file [../name] and new file [name].
	- You can pass a third argument to the command line (well fourth if u count the exe name) which is a float value for the desired resize factor (scale up or down, ex. 0.5 or 3.5).
	example:
		Imagedrop.exe D:\testImages\sample_2.tga
		Imagedrop.exe D:\testImages\sample_2.tga newImage.tga
		Imagedrop.exe D:\testImages\sample_2.tga newImage.tga 0.5
	- When use command line, you need the source image location, not only name, so it can work regardless where the image is located at your PC

#VS Debugger
	- Just make sure to put some "Command Arguments" within the "Debugging" secion of the project settings.
	- All types of arguments expalined with the #Commandline section, are applicable here too.
*/

//Includes
//It is in purpose to not put the headers within their own folder, the project is samll & its okay to keep them alongside the main cpp
#include <iostream>
#include <filesystem>
#include <chrono>
#include <array>
#include "Consts.h"
#include "Bits.h"
#include "Macros.h"
#include "ImageFormatBase.h"
#include "BMPFormat.h"
#include "TGAFormat.h"

//void OnReadTGA(TGA_Format &format, const char *path){}
//void OnWriteTGA(TGA_Format &format, const char *path){}
//void OnResizeTGA(const TGA_Format &sourceFormat, TGA_Format &newFormat, float resizeMultiplier){}
//void OnReadBMP(BMP_Format &format, const char *path){}

int main(int argc, char *argv[])
{
	//-----------------------------------------------------------------------
	//Entry point -> Heap read [~85.00kb] the deafault for an empty main/app
	//-----------------------------------------------------------------------

	/*
	The arguments i expect to be passed shall not be less than 2 or more than 4
	[0] exe		[1] Image		[2] New Name		[3] Scale Factor
	*/

	if (argc < 2 || argc > 4)
	{
		LOG("ERR	Few or many arguments been passed to the app, make sure to pass params correctly");
		THROW_ERROR("Few or many arguments been passed to the app, make sure to pass params correctly");
	}
	else
	{
		//the resize multiplier, either passed to the app or auto set to 0.5
		float _resizeMultiplier;

		/*Prepare a path and filename for the new generated image
		either way, a param passed for new image name, or not, then one
		will be generated from the original file name in case there
		isn't a name been apssed through arguments using the same original
		source image file format)*/
		std::experimental::filesystem::path _path = argv[1];
		std::string _fileFormat = _path.extension().string().c_str();
		std::string _autoName = _path.filename().string().c_str();
		_autoName.replace(_autoName.end() - 4, _autoName.begin(), "_RESIZED");
		char _buffer[256];
		sprintf_s(_buffer, "%s%s", _autoName.c_str(), _fileFormat.c_str());

		//Check if user input a new file name, or we use the generated value above
		if (argc < 3)
			_path.replace_filename(_buffer);
		else
			_path.replace_filename(argv[2]);

		//Check if user input a resize multiplier, or we use the defualt value
		if (argc > 3)
			_resizeMultiplier = (float)atof(argv[3]);
		else
			_resizeMultiplier = DEFAULT_RESIZE_MULTIPLIER;

#ifdef USE_LOG_TIME
		std::chrono::high_resolution_clock::time_point _startTime = std::chrono::high_resolution_clock::now();
#endif // USE_LOG_TIME

		if (_fileFormat == IMG_FORMAT_BMP)
		{
			BMP_Format _formatLoaded;
			BMP_Format _formatGenerated;
			_formatLoaded.OnImageRead(argv[1]);
		}
		else if (_fileFormat == IMG_FORMAT_JPG)
		{

		}
		else if (_fileFormat == IMG_FORMAT_PNG)
		{

		}
		else if (_fileFormat == IMG_FORMAT_TGA)
		{
			//A TGA to load in, and another one to fill (scale up or down)
			TGA_Format _formatLoaded;
			TGA_Format _formatGenerated;
			//Read the TGA passed by arguments (drag'n'drop, commandline or debugger)
			_formatLoaded.OnImageRead(argv[1]);
			//Resize the TGA into a new empty one
			_formatLoaded.OnImageResize(_formatGenerated, _resizeMultiplier);
			//Write the new TGA to disk
			_formatGenerated.OnImageWrite((_path.string()).c_str());
		}

		

#ifdef USE_LOG_TIME
		std::chrono::high_resolution_clock::time_point _endTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> _duration = _endTime - _startTime;
		LOG("Time Spent - Total: " << _duration.count()* 1000.f << "ms");
#endif // USE_LOG_TIME	


		WAIT_INPUT;

		//-----------------------------------------------------------------------
		//Exit point -> Heap read [~86.00kb] 
		//-----------------------------------------------------------------------
	}
}