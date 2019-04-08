/*
More about the file format specification
https://en.wikipedia.org/wiki/BMP_file_format
http://www.digicamsoft.com/bmp/bmp.html
https://web.archive.org/web/20080912171714/http://www.fortunecity.com/skyscraper/windows/364/bmpffrmt.html
*/

/*
https://graphicdesign.stackexchange.com/questions/47133/is-32-bit-color-depth-enough
This note probably need to be moved to the parent class, when in the future this BMP get inhirited from a parent:
BitCount specifies the color resolution of the bitmap, and values are:
- 1		(black/white)
- 4		(16 colors)
- 8		(256 colors)
- 24	(16.7 million colors)
*/
#pragma once

#include "ImageFormatBase.h"

class BMP_Format : public ImageFormatBase
{
public:
	//BitmapFileHeader
	uint16_t m_Type;					//[2bytes]	-	BM 0x42 0x4D is for BMP.It is to identify the BMP and DIB. Possible values (BM, BA, CI, CP, IC, PT)
	uint32_t m_FileSize;				//[4bytes]	-	The size of the file in bytes
	uint16_t m_Reserved1;				//[2bytes]	-	Must always be set to Zero
	uint16_t m_Reserved2;				//[2bytes]	-	Must always be set to Zero
	uint32_t m_OffsetBits;				//[4bytes]	-	The offset from the beginning of the file to the bitmap data
	
	//BitmapInfoHeader	
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

	std::vector<uint8_t> m_Pixels;

	BMP_Format()
	{
		ImageFormat = EImageFormat::BMP;
	}
	~BMP_Format()
	{
		m_Pixels.clear();
		m_Pixels.shrink_to_fit();
	}

	size_t SizeInBytes(const BMP_Format &format)
	{
		//this shall match the size found in [Right click-> properties] within explorer, if not, then there is an issue
		return (format.m_Width * format.m_Height * format.m_BitCount / 8);
	}

	void OnImageRead(const char *path) override
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
		fread(&m_Type, 2, 1, _file);
		fread(&m_FileSize, 4, 1, _file);
		fread(&m_Reserved1, 2, 1, _file);
		fread(&m_Reserved2, 2, 1, _file);
		fread(&m_OffsetBits, 4, 1, _file);

		fread(&m_Size, 4, 1, _file);
		fread(&m_Width, 4, 1, _file);
		fread(&m_Height, 4, 1, _file);
		fread(&m_Planes, 2, 1, _file);
		fread(&m_BitCount, 2, 1, _file);
		fread(&m_Compression, 4, 1, _file);
		fread(&m_SizeImage, 4, 1, _file);
		fread(&m_XPelsPerMeter, 4, 1, _file);
		fread(&m_YPelsPerMeter, 4, 1, _file);
		fread(&m_ColorsUsed, 4, 1, _file);
		fread(&m_ColorsImportant, 4, 1, _file);

		if (m_BitCount < 24)
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
		LOG("ImageWidth: " << m_Width);
		LOG("ImageHeigh: " << m_Height);
		LOG("ImageSize: " << SizeInBytes(*this) << "Bytes");
		LOG("ImageBitsPerPixel: " << size_t(m_BitCount) << "bit");
		LOG("================================================");
	}

	void OnImageWrite(const char *path) override {}
	void OnImageResize(ImageFormatBase &newFormat, float resizeMultiplier) override {}
};