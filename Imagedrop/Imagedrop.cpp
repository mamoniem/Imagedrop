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
#include "Bits.h"
#include "Macros.h"
#include "TGAFormat.h"
#include "BMPFormat.h"

size_t SizeInBytes(const TGA_Format &format)
{
	//this shall match the size found in [Right click-> properties] within explorer, if not, then there is an issue
	return (format.m_imageWidth * format.m_imageHeigh * format.m_imagePixelDepth / 8);
}
size_t SizeInBytes(const BMP_Format &format)
{
	//this shall match the size found in [Right click-> properties] within explorer, if not, then there is an issue
	return (format.m_Width * format.m_Height * format.m_BitCount / 8);
}

uint8_t IsCompressed(const TGA_Format &format)
{
	return(
		format.m_imageType == TGA_IMAGE_TYPE_RLE_ENCODED_COLOR_MAPPED ||
		format.m_imageType == TGA_IMAGE_TYPE_RLE_ENCODED_TRUE_COLOR ||
		format.m_imageType == TGA_IMAGE_TYPE_RLE_ENCODED_GRAYSCALE
		);
}

uint8_t IsGrayScale(const TGA_Format &format)
{
	return(
		format.m_imageType == TGA_IMAGE_TYPE_UNCOMPRESSED_GRAYSCALE ||
		format.m_imageType == TGA_IMAGE_TYPE_RLE_ENCODED_GRAYSCALE
		);
}

const uint8_t* NeighbourPtr(const TGA_Format &format, int xIndex, int yIndex)
{
	CLAMP_PIXEL(xIndex, 0, format.m_imageWidth - 1, yIndex, 0, format.m_imageHeigh - 1);
	return &format.m_pixels[(yIndex * format.m_channels) + xIndex * (format.m_imagePixelDepth > 24 ? format.m_imagePixelDepth > 16 ? 4 : 3 : 3)];
}

uint8_t BilinearPixelColor(const uint8_t* BL, const uint8_t* BR, const uint8_t* TL, const uint8_t* TR, float W, float H, int index)
{
	float _colorA;
	float _colorB;
	float _color;

	//The vertical (on Y) linear interpolation
	LERP(TL[index], BL[index], _colorA, W);
	LERP(TR[index], BR[index], _colorB, W);
	LERP(_colorA, _colorB, _color, H);

	CLAMP(_color, MIN_COLOR, MAX_COLOR);

	return uint8_t(_color);
}

void OnReadTGA(TGA_Format &format, const char *path)
{
#ifdef USE_LOG_TIME
	std::chrono::high_resolution_clock::time_point _startTime = std::chrono::high_resolution_clock::now();
#endif // USE_LOG_TIME

	//open the file
	//I usually use fopen, but at the same time didn't want to hide warnings with _CRT_SECURE_NO_WARNINGS in a job application test, so used the secure one
	FILE *_file;
	fopen_s(&_file, path, "rb");
	LOG(path);
	if (_file == NULL)
	{
		LOG("ERR	fopen is NULL [Read]");
		THROW_ERROR("fopen is NULL  [Read]");
	}

	format.m_id = NULL;
	format.m_colorMapData = NULL;

	//read from file with the same order & store into the TGA blocks.
	//ID Length						[1byte] 8
	//Color Map Type				[1byte] 8
	//Image Type					[1byte] 8
	//Color Map Specification		[5bytes] 16, 16, 8
	//Image Specification			[10bytes] 16, 16, 16, 16, 8, 8
	fread(&format.m_idLength, 1, 1, _file);

	fread(&format.m_colorMapType, 1, 1, _file);

	fread(&format.m_imageType, 1, 1, _file);

	fread(&format.m_colorMapFirstEntryIndex, 2, 1, _file);
	fread(&format.m_colorMapLength, 2, 1, _file);
	fread(&format.m_colorMapEntrySize, 1, 1, _file);

	fread(&format.m_imageOriginX, 2, 1, _file);
	fread(&format.m_imageOriginY, 2, 1, _file);
	fread(&format.m_imageWidth, 2, 1, _file);
	fread(&format.m_imageHeigh, 2, 1, _file);
	fread(&format.m_imagePixelDepth, 1, 1, _file);
	fread(&format.m_imageDescription, 1, 1, _file);

	if (format.m_imagePixelDepth < 24)
	{
		LOG("ERR	m_imagePixelDepth is neither 32b nor 24b");
		THROW_ERROR("m_imagePixelDepth is neither 32b nor 24b");
	}

	//It's a good place to check if any of the read values is invalid, if needed.

	//Resolve the core required data
	if (format.m_idLength > 0)
	{
		format.m_id = (uint8_t*)malloc(format.m_idLength);

		if (format.m_id == NULL)
		{
			LOG("ERR	m_id is NULL");
			THROW_ERROR("m_id is NULL");
		}

		fread(&format.m_id, format.m_idLength, 1, _file);
	}

	if (format.m_colorMapType == TGA_COLOR_MAP_TYPE_PRESENT)
	{
		format.m_colorMapData = (uint8_t*)malloc((format.m_colorMapFirstEntryIndex + format.m_colorMapLength) * format.m_colorMapEntrySize / 8);

		if (format.m_colorMapData == NULL)
		{
			LOG("ERR	m_colorMapData is NULL");
			THROW_ERROR("m_colorMapData is NULL");
		}

		fread(&format.m_colorMapData + (format.m_colorMapFirstEntryIndex * format.m_colorMapEntrySize / 8), format.m_colorMapLength * format.m_colorMapEntrySize / 8, 1, _file);
	}

	format.m_pixels.resize(SizeInBytes(format));

	//check for RLE
	if (IsCompressed(format))
	{
		LOG("ERR	RLE not supported yet!");
		THROW_ERROR("RLE not supported yet!");
	}
	else
	{
		fread(&format.m_pixels[0], SizeInBytes(format), 1, _file);
	}

	format.m_channels = format.m_imageWidth * (format.m_imagePixelDepth > 24 ? format.m_imagePixelDepth > 16 ? 4 : 3 : 3);

	//close the file
	fclose(_file);

#ifdef USE_LOG_TIME
	std::chrono::high_resolution_clock::time_point _endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> _duration = _endTime - _startTime;
	LOG("Time Spent - Reading: " << _duration.count()* 1000.f << "ms");
#endif // USE_LOG_TIME

	LOG("====================R=E=A=D=====================");
	LOG("ImageWidth: " << format.m_imageWidth);
	LOG("ImageHeigh: " << format.m_imageHeigh);
	LOG("ImageSize: " << SizeInBytes(format) << "Bytes");
	LOG("ImageBitsPerPixel: " << size_t(format.m_imagePixelDepth) << "bit");
	LOG("================================================");
}

void OnWriteTGA(TGA_Format &format, const char *path)
{
	LOG("===================W=R=I=T=E====================");
	LOG("ImageWidth: " << format.m_imageWidth);
	LOG("ImageHeigh: " << format.m_imageHeigh);
	LOG("ImageSize: " << SizeInBytes(format) << "Bytes");
	LOG("ImageBitsPerPixel: " << size_t(format.m_imagePixelDepth) << "bit");
	LOG("================================================");

#ifdef USE_LOG_TIME
	std::chrono::high_resolution_clock::time_point _startTime = std::chrono::high_resolution_clock::now();
#endif // USE_LOG_TIME

	//same here, I usually use fopen, but at the same time didn't want to hide warnings with _CRT_SECURE_NO_WARNINGS in a job application test, so used the secure one
	FILE *_file;
	fopen_s(&_file, path, "wb");
	LOG(path);
	if (_file == NULL)
	{
		LOG("ERR	fopen is NULL [Write]");
		THROW_ERROR("fopen is NULL [Write]");
	}

	//wrtie with the same order used to read (matching the file format specification order)
	//ID Length						[1byte] 8
	//Color Map Type				[1byte] 8
	//Image Type					[1byte] 8
	//Color Map Specification		[5bytes] 16, 16, 8
	//Image Specification			[10bytes] 16, 16, 16, 16, 8, 8
	fwrite(&format.m_idLength, 1, 1, _file);

	fwrite(&format.m_colorMapType, 1, 1, _file);

	fwrite(&format.m_imageType, 1, 1, _file);

	fwrite(&format.m_colorMapFirstEntryIndex, 2, 1, _file);
	fwrite(&format.m_colorMapLength, 2, 1, _file);
	fwrite(&format.m_colorMapEntrySize, 1, 1, _file);

	fwrite(&format.m_imageOriginX, 2, 1, _file);
	fwrite(&format.m_imageOriginY, 2, 1, _file);
	fwrite(&format.m_imageWidth, 2, 1, _file);
	fwrite(&format.m_imageHeigh, 2, 1, _file);
	fwrite(&format.m_imagePixelDepth, 1, 1, _file);
	fwrite(&format.m_imageDescription, 1, 1, _file);

	if (format.m_idLength > 0)
	{
		fwrite(&format.m_id, format.m_idLength, 1, _file);
	}

	if (format.m_colorMapType == TGA_COLOR_MAP_TYPE_PRESENT)
	{
		fwrite(&format.m_colorMapData + (format.m_colorMapFirstEntryIndex * format.m_colorMapEntrySize / 8), format.m_colorMapLength * format.m_colorMapEntrySize / 8, 1, _file);
	}

	if (IsCompressed(format))
	{
		//When i support RLE, need to update here!
	}
	else
	{
		fwrite(&format.m_pixels[0], SizeInBytes(format), 1, _file);
	}

	fwrite(tgaEmptyFooterBytes, tgaFooterSize, 1, _file);

	//close
	fclose(_file);

#ifdef USE_LOG_TIME
	std::chrono::high_resolution_clock::time_point _endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> _duration = _endTime - _startTime;
	LOG("Time Spent - Writing: " << _duration.count()* 1000.f << "ms");
#endif // USE_LOG_TIME
}

void OnResizeTGA(const TGA_Format &sourceFormat, TGA_Format &newFormat, float resizeMultiplier)
{
#ifdef USE_LOG_TIME
	std::chrono::high_resolution_clock::time_point _startTime = std::chrono::high_resolution_clock::now();
#endif // USE_LOG_TIME

	//Fill members of the new resized version
	//let's start with the idintical ones, info that will probably remain the same
	newFormat.m_id = sourceFormat.m_id;
	newFormat.m_colorMapData = sourceFormat.m_colorMapData;
	newFormat.m_idLength = sourceFormat.m_idLength;
	newFormat.m_colorMapType = sourceFormat.m_colorMapType;
	newFormat.m_imageType = sourceFormat.m_imageType;
	newFormat.m_colorMapFirstEntryIndex = sourceFormat.m_colorMapFirstEntryIndex;
	newFormat.m_colorMapLength = sourceFormat.m_colorMapLength;
	newFormat.m_colorMapEntrySize = sourceFormat.m_colorMapEntrySize;
	newFormat.m_imageOriginX = sourceFormat.m_imageOriginX;
	newFormat.m_imageOriginY = sourceFormat.m_imageOriginY;
	newFormat.m_imagePixelDepth = sourceFormat.m_imagePixelDepth;
	newFormat.m_imageDescription = sourceFormat.m_imageDescription;

	//of course the diminsions will be based on the scaleMultiplier
	newFormat.m_imageWidth = uint16_t(float(sourceFormat.m_imageWidth)*resizeMultiplier);
	newFormat.m_imageHeigh = uint16_t(float(sourceFormat.m_imageHeigh)*resizeMultiplier);

	newFormat.m_channels = newFormat.m_imageWidth * (newFormat.m_imagePixelDepth > 24 ? newFormat.m_imagePixelDepth > 16 ? 4 : 3 : 3);

	//expand or shrink, to fit the amount of pixels and channels for the new image size [NewWidth*NewHigh*Depth/8b]
	newFormat.m_pixels.resize((const unsigned int)(
		(sourceFormat.m_imageWidth*resizeMultiplier) *
		(sourceFormat.m_imageHeigh*resizeMultiplier) *
		(sourceFormat.m_imagePixelDepth) / 8));

	uint8_t *_currentRow = &newFormat.m_pixels[0];
	for (int y = 0; y < newFormat.m_imageHeigh; y++)
	{
		uint8_t *_pixel = _currentRow;
		float _vertical = float(y) / float(newFormat.m_imageHeigh - 1);
		for (int x = 0; x < newFormat.m_imageWidth; x++)
		{
			float _horizontal = float(x) / float(newFormat.m_imageWidth - 1);

			//start resampling in bilinear
			//TODO::may be good move this to its own function later. Also can put macros here to measure the bilinear time only if needed

			//ease the move between pixels of the TGA vertically and horizontally
			float _w = (_horizontal * sourceFormat.m_imageWidth);
			float _h = (_vertical * sourceFormat.m_imageHeigh);

			int _indexX = int(_w);
			int _indexY = int(_h);

			float _W = _w - floor(_w);
			float _H = _h - floor(_h);

			/*
				[0.1]			[1.1]
				TL				TR
				-----------------
				|			|	|
				|-----------|----
				|			|	|
				|			|	|
				|			|	|
				-----------------
				BL				BR
				[0.0]			[1.0]

				TL => Top Left
				TR => Top Right
				BL => Bottom Left
				BR => Bottom Right

				- Let's first interpolate linearlly between the two bottom points. [X]
				- Then interpolate linearlly between the two top points. [X]
				- Then we interpolate linearlly between the two results. [Y]
			*/

			//The Horizontal linear interpolation (on X) two times
			auto _BL = NeighbourPtr(sourceFormat, _indexX + 1, _indexY + 0);
			auto _BR = NeighbourPtr(sourceFormat, _indexX + 1, _indexY + 1);
			auto _TL = NeighbourPtr(sourceFormat, _indexX + 0, _indexY + 0);
			auto _TR = NeighbourPtr(sourceFormat, _indexX + 0, _indexY + 1);

			std::array<uint8_t, 3> _outRGB;
			std::array<uint8_t, 4> _outRGBA;

			//interpolate the colors for the new pixels
			if (sourceFormat.m_imagePixelDepth > 24) //32RGBA
			{
				for (int i = 0; i < 4; i++)
				{
					_outRGBA[i] = BilinearPixelColor(_BL, _BR, _TL, _TR, _W, _H, i);
				}

				//use the out results & jump forward 4*size_t
				_pixel[0] = _outRGBA[0];
				_pixel[1] = _outRGBA[1];
				_pixel[2] = _outRGBA[2];
				_pixel[3] = _outRGBA[3];
				_pixel += 4; //skip 4
			}
			else //24RGB (or something else that is not supported, but won't reach that line due to the runtime error thrown at load time if less than 24bit)
			{
				for (int i = 0; i < 3; i++)
				{
					_outRGB[i] = BilinearPixelColor(_BL, _BR, _TL, _TR, _W, _H, i);
				}

				//use the out results & jump forward 3*size_t
				_pixel[0] = _outRGB[0];
				_pixel[1] = _outRGB[1];
				_pixel[2] = _outRGB[2];
				_pixel += 3; //skip 3
			}
		}
		_currentRow += newFormat.m_channels;
	}

#ifdef USE_LOG_TIME
	std::chrono::high_resolution_clock::time_point _endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> _duration = _endTime - _startTime;
	LOG("Time Spent - Resizing: " << _duration.count()* 1000.f << "ms");
#endif // USE_LOG_TIME
}

void OnReadBMP(BMP_Format &format, const char *path)
{
#ifdef USE_LOG_TIME
	std::chrono::high_resolution_clock::time_point _startTime = std::chrono::high_resolution_clock::now();
#endif // USE_LOG_TIME

	//open the file
	//I usually use fopen, but at the same time didn't want to hide warnings with _CRT_SECURE_NO_WARNINGS in a job application test, so used the secure one
	FILE *_file;
	fopen_s(&_file, path, "rb");
	LOG(path);
	if (_file == NULL)
	{
		LOG("ERR	fopen is NULL [Read]");
		THROW_ERROR("fopen is NULL  [Read]");
	}

	//read from file with the same order & store into the TGA blocks.
	/*
	uint16_t m_Type;					//[2bytes]	-	BM 0x42 0x4D is for BMP.It is to identify the BMP and DIB. Possible values (BM, BA, CI, CP, IC, PT)
	uint32_t m_FileSize;				//[4bytes]	-	The size of the file in bytes
	uint16_t m_Reserved1;				//[2bytes]	-	Must always be set to Zero
	uint16_t m_Reserved2;				//[2bytes]	-	Must always be set to Zero
	uint32_t m_OffsetBits;				//[4bytes]	-	The offset from the beginning of the file to the bitmap data
	
	uint32_t m_Size;					//[4bytes]	-	The size of BITMAPINFOHEADER structure in bytes
	uint32_t m_Width;					//[4bytes]	-	The width of the image in pixels (singed int)
	uint32_t m_Height;					//[4bytes]	-	The height of the image in pixels (singed int)
	uint16_t m_Planes;					//[2bytes]	-	The number  of color planes (1 by defualt, some resources say it must be set to zero!)
	uint16_t m_BitCount;				//[2bytes]	-	The number of bits per pixel, which is the color depth of the image. Typical values are 1, 4, 8, 16, 24 & 32
	uint32_t m_Compression;				//[4bytes]	-	The used compression method. Zero means no compression. Check the bits.h BMP_COMPRESSION_METHOD_
	uint32_t m_SizeImage;				//[4bytes]	-	The size of the raw image data in bytes, when no compression (BI_RGB) this value can be zero
	uint32_t m_XPelsPerMeter;			//[4bytes]	-	The horizontal resolution of the image (pixel per meter, signed int). usually set to zero
	uint32_t m_YPelsPerMeter;			//[4bytes]	-	The vertical resolution of the image (pixel per meter, signed int). usually set to zero
	uint32_t m_ColorsUsed;				//[4bytes]	-	The number of colors used in the bitmap (in the color palette). If this set to 0 the number of colors is calculated using the m_BitCount structure member
	uint32_t m_ColorsImportant;			//[4bytes]	-	The number of colors that are important for the bitmap. Set to 0 when all colors are important. And generally ignored value
	*/
	fread(&format.m_Type, 2, 1, _file);
	fread(&format.m_FileSize, 4, 1, _file);
	fread(&format.m_Reserved1, 2, 1, _file);
	fread(&format.m_Reserved2, 2, 1, _file);
	fread(&format.m_OffsetBits, 4, 1, _file);

	fread(&format.m_Size, 4, 1, _file);
	fread(&format.m_Width, 4, 1, _file);
	fread(&format.m_Height, 4, 1, _file);
	fread(&format.m_Planes, 2, 1, _file);
	fread(&format.m_BitCount, 2, 1, _file);
	fread(&format.m_Compression, 4, 1, _file);
	fread(&format.m_SizeImage, 4, 1, _file);
	fread(&format.m_XPelsPerMeter, 4, 1, _file);
	fread(&format.m_YPelsPerMeter, 4, 1, _file);
	fread(&format.m_ColorsUsed, 4, 1, _file);
	fread(&format.m_ColorsImportant, 4, 1, _file);

	if (format.m_BitCount < 24)
	{
		LOG("ERR	m_imagePixelDepth is neither 32b nor 24b");
		THROW_ERROR("m_imagePixelDepth is neither 32b nor 24b");
	}

	//It's a good place to check if any of the read values is invalid, if needed.

	//Resolve the core required data
	/*
	if (format.m_colorMapType == TGA_COLOR_MAP_TYPE_PRESENT)
	{
		format.m_colorMapData = (uint8_t*)malloc((format.m_colorMapFirstEntryIndex + format.m_colorMapLength) * format.m_colorMapEntrySize / 8);

		if (format.m_colorMapData == NULL)
		{
			LOG("ERR	m_colorMapData is NULL");
			THROW_ERROR("m_colorMapData is NULL");
		}

		fread(&format.m_colorMapData + (format.m_colorMapFirstEntryIndex * format.m_colorMapEntrySize / 8), format.m_colorMapLength * format.m_colorMapEntrySize / 8, 1, _file);
	}

	format.m_pixels.resize(SizeInBytes(format));

	//check for RLE
	if (IsCompressed(format))
	{
		LOG("ERR	RLE not supported yet!");
		THROW_ERROR("RLE not supported yet!");
	}
	else
	{
		fread(&format.m_pixels[0], SizeInBytes(format), 1, _file);
	}

	format.m_channels = format.m_imageWidth * (format.m_imagePixelDepth > 24 ? format.m_imagePixelDepth > 16 ? 4 : 3 : 3);
	*/

	//close the file
	fclose(_file);

#ifdef USE_LOG_TIME
	std::chrono::high_resolution_clock::time_point _endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> _duration = _endTime - _startTime;
	LOG("Time Spent - Reading: " << _duration.count()* 1000.f << "ms");
#endif // USE_LOG_TIME

	LOG("=================R=E=A=D====B=M=P===============");
	LOG("ImageWidth: " << format.m_Width);
	LOG("ImageHeigh: " << format.m_Height);
	LOG("ImageSize: " << SizeInBytes(format) << "Bytes");
	LOG("ImageBitsPerPixel: " << size_t(format.m_BitCount) << "bit");
	LOG("================================================");
}

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
		//test to remove!
		BMP_Format _format;
		OnReadBMP(_format, argv[1]);

		//A TGA to load in, and another one to fill (scale up or down)
		TGA_Format _formatLoaded;
		TGA_Format _formatGenerated;
		float _resizeMultiplier;

		/*Prepare a path and filename for the new generated TGA
		either way, a param passed for new image name, or not, then one
		will be generated from the original file name in case there
		isn't a name been apssed through arguments)*/
		std::experimental::filesystem::path _path = argv[1];
		std::string _autoName = _path.filename().string().c_str();
		_autoName.replace(_autoName.end() - 4, _autoName.begin(), "_RESIZED");
		char _buffer[256];
		sprintf_s(_buffer, "%s.tga", _autoName.c_str());

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

		//Read the TGA passed by arguments (drag'n'drop, commandline or debugger)
		OnReadTGA(_formatLoaded, argv[1]);

		//Resize the TGA into a new empty one
		OnResizeTGA(_formatLoaded, _formatGenerated, _resizeMultiplier);

		//Write the new TGA to disk
		OnWriteTGA(_formatGenerated, (_path.string()).c_str());

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