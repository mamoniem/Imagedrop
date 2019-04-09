/*
More about the file format specification
https://en.wikipedia.org/wiki/Truevision_TGA
http://www.dca.fee.unicamp.br/~martino/disciplinas/ea978/tgaffs.pdf
http://tfc.duke.free.fr/coding/tga_specs.pdf
*/
#pragma once

#include "ImageFormatBase.h"

/*
As we deal with TGA Ver.2, then have to fill 26bytes for the footer
Extension offset	[4 bytes]
Developer offset	[4 bytes]
Signature			[16 byte]
"."					[1 byte]
NULL				[1 byte]
----------------------------------
Total				[26 bytes]
*/
static const char tgaEmptyFooterBytes[] =
"\0\0\0\0"										//[4bytes]	-	Extension offset	Offset in bytes from the beginning of the file
"\0\0\0\0"										//[4bytes]	-	Developer area offset	Offset in bytes from the beginning of the file
"TRUEVISION-XFILE"								//[16bytes]	-	Signature	Contains "TRUEVISION-XFILE"
"."												//[1byte]	-	Contains "."
"\0";											//[1byte]	-	Contains NULL
static const size_t tgaFooterSize = 26;

/*
- I decided to go with class
- All public
- No Getters and Setters, and they make no sense in such small size commandline app
- Mehtods are independent as fucntions, not within a class, so I can extend this later
	to read more file types (no move them within the class as methods if inhiritaned this
	in the future from let' say a parent "ImageFile" class)
*/
class TGA_Format : public ImageFormatBase
{
public:
	//Header
	uint8_t m_IdLength;							//[1byte]	-	Image ID length (field 1), ID Length - date, time, ver, etc.
	uint8_t m_ColorMapType;						//[1byte]	-	Color map type (field 2), 0 if image file contains no color map || 1 if present || 2–127 reserved by Truevision || 128–255 available for developer use
	uint8_t m_ImageType;						//[1byte]	-	Image type (field 3), 0 no image data is present || 1 uncompressed color-mapped image || 2 uncompressed true-color image || 3 uncompressed black-and-white (grayscale) image || 9 run-length encoded color-mapped image || 10 run-length encoded true-color image || 11 run-length encoded black-and-white (grayscale) image

	//Color map specification (field 4)			//[5bytes]	-	Split into 2, 2, 1
	uint16_t m_ColorMapFirstEntryIndex;			//[2bytes]	-	Index of first color map entry that is included in the file
	uint16_t m_ColorMapLength;					//[2bytes]	-	Number of entries of the color map that are included in the file
	uint8_t m_ColorMapEntrySize;				//[1byte]	-	Number of bits per pixel

	//Image specification (field 5)				//[10bytes]	-	Split into 2, 2, 2, 2, 1, 1
	uint16_t m_ImageOriginX;					//[2bytes]	-	Absolute coordinate of lower-left corner for displays where origin is at the lower left
	uint16_t m_ImageOriginY;					//[2bytes]	-	As for X-origin
	uint16_t m_ImageWidth;						//[2bytes]	-	Width in pixels
	uint16_t m_ImageHeigh;						//[2bytes]	-	Height in pixels
	uint8_t m_ImagePixelDepth;					//[1byte]	-	Bits per pixel
	uint8_t m_ImageDescription;					//[1byte]	-	Bits 3-0 give the alpha channel depth, bits 5-4 give direction

	//Developer area (optional)

	//Extension area (optional)

	//File footer (optional)

	uint8_t *m_Id;
	uint8_t *m_ColorMapData;
	std::vector<uint8_t> m_Pixels;
	long m_Channels;							//make more sense of the number 32 is 4 channels and 24 is 3 channels

	//I don't need so far to initialize the constructor with any values
	TGA_Format()
	{
		ImageFormat = EImageFormat::TGA;
	}
	//Just in case i forget to deallocate something, this may be not needed later
	~TGA_Format()
	{
		//just in case
		m_Pixels.clear();
		m_Pixels.shrink_to_fit();
		free(m_Id);
		free(m_ColorMapData);
	}

	size_t SizeInBytes() override
	{
		//this shall match the size found in [Right click-> properties] within explorer, if not, then there is an issue
		return (m_ImageWidth * m_ImageHeigh * m_ImagePixelDepth / 8);
	}

	uint8_t IsGrayScale(const TGA_Format &format)
	{
		return(
			format.m_ImageType == TGA_IMAGE_TYPE_UNCOMPRESSED_GRAYSCALE ||
			format.m_ImageType == TGA_IMAGE_TYPE_RLE_ENCODED_GRAYSCALE
			);
	}

	uint8_t IsCompressed(const TGA_Format &format)
	{
		return(
			format.m_ImageType == TGA_IMAGE_TYPE_RLE_ENCODED_COLOR_MAPPED ||
			format.m_ImageType == TGA_IMAGE_TYPE_RLE_ENCODED_TRUE_COLOR ||
			format.m_ImageType == TGA_IMAGE_TYPE_RLE_ENCODED_GRAYSCALE
			);
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

		m_Id = NULL;
		m_ColorMapData = NULL;

		//read from file with the same order & store into the TGA blocks.
		//ID Length						[1byte] 8
		//Color Map Type				[1byte] 8
		//Image Type					[1byte] 8
		//Color Map Specification		[5bytes] 16, 16, 8
		//Image Specification			[10bytes] 16, 16, 16, 16, 8, 8
		fread(&m_IdLength, 1, 1, _file);

		fread(&m_ColorMapType, 1, 1, _file);

		fread(&m_ImageType, 1, 1, _file);

		fread(&m_ColorMapFirstEntryIndex, 2, 1, _file);
		fread(&m_ColorMapLength, 2, 1, _file);
		fread(&m_ColorMapEntrySize, 1, 1, _file);

		fread(&m_ImageOriginX, 2, 1, _file);
		fread(&m_ImageOriginY, 2, 1, _file);
		fread(&m_ImageWidth, 2, 1, _file);
		fread(&m_ImageHeigh, 2, 1, _file);
		fread(&m_ImagePixelDepth, 1, 1, _file);
		fread(&m_ImageDescription, 1, 1, _file);

		if (m_ImagePixelDepth < 24)
		{
			LOG("ERR	m_imagePixelDepth is neither 32b nor 24b");
			THROW_ERROR("m_imagePixelDepth is neither 32b nor 24b");
		}

		//It's a good place to check if any of the read values is invalid, if needed.

		//Resolve the core required data
		if (m_IdLength > 0)
		{
			m_Id = (uint8_t*)malloc(m_IdLength);

			if (m_Id == NULL)
			{
				LOG("ERR	m_id is NULL");
				THROW_ERROR("m_id is NULL");
			}

			fread(&m_Id, m_IdLength, 1, _file);
		}

		if (m_ColorMapType == TGA_COLOR_MAP_TYPE_PRESENT)
		{
			m_ColorMapData = (uint8_t*)malloc((m_ColorMapFirstEntryIndex + m_ColorMapLength) * m_ColorMapEntrySize / 8);

			if (m_ColorMapData == NULL)
			{
				LOG("ERR	m_colorMapData is NULL");
				THROW_ERROR("m_colorMapData is NULL");
			}

			fread(&m_ColorMapData + (m_ColorMapFirstEntryIndex * m_ColorMapEntrySize / 8), m_ColorMapLength * m_ColorMapEntrySize / 8, 1, _file);
		}

		m_Pixels.resize(SizeInBytes());

		//check for RLE
		if (IsCompressed(*this))
		{
			LOG("ERR	RLE not supported yet!");
			THROW_ERROR("RLE not supported yet!");
		}
		else
		{
			fread(&m_Pixels[0], SizeInBytes(), 1, _file);
		}

		m_Channels = m_ImageWidth * (m_ImagePixelDepth > 24 ? m_ImagePixelDepth > 16 ? 4 : 3 : 3);

		//close the file
		fclose(_file);

#ifdef USE_LOG_TIME
		std::chrono::high_resolution_clock::time_point _endTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> _duration = _endTime - _startTime;
		LOG("Time Spent - Reading: " << _duration.count()* 1000.f << "ms");
#endif // USE_LOG_TIME

#ifdef USE_LOG_IMAGE_DATA
		if (m_Pixels.size() != 0)
			LOG(m_Pixels.data());
		else
			LOG("ERR, the pixels vector is empty or null!");
#endif // USE_LOG_IMAGE_DATA


		LOG("=================R=E=A=D====T=G=A===============");
		LOG("ImageWidth: " << m_ImageWidth);
		LOG("ImageHeigh: " << m_ImageHeigh);
		LOG("ImageSize: " << SizeInBytes() << "Bytes");
		LOG("ImageBitsPerPixel: " << size_t(m_ImagePixelDepth) << "bit");
		LOG("================================================");
	}

	void OnImageWrite(const char *path) override
	{
		LOG("===================W=R=I=T=E====================");
		LOG("ImageWidth: " << m_ImageWidth);
		LOG("ImageHeigh: " << m_ImageHeigh);
		LOG("ImageSize: " << SizeInBytes() << "Bytes");
		LOG("ImageBitsPerPixel: " << size_t(m_ImagePixelDepth) << "bit");
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
		fwrite(&m_IdLength, 1, 1, _file);

		fwrite(&m_ColorMapType, 1, 1, _file);

		fwrite(&m_ImageType, 1, 1, _file);

		fwrite(&m_ColorMapFirstEntryIndex, 2, 1, _file);
		fwrite(&m_ColorMapLength, 2, 1, _file);
		fwrite(&m_ColorMapEntrySize, 1, 1, _file);

		fwrite(&m_ImageOriginX, 2, 1, _file);
		fwrite(&m_ImageOriginY, 2, 1, _file);
		fwrite(&m_ImageWidth, 2, 1, _file);
		fwrite(&m_ImageHeigh, 2, 1, _file);
		fwrite(&m_ImagePixelDepth, 1, 1, _file);
		fwrite(&m_ImageDescription, 1, 1, _file);

		if (m_IdLength > 0)
		{
			fwrite(&m_Id, m_IdLength, 1, _file);
		}

		if (m_ColorMapType == TGA_COLOR_MAP_TYPE_PRESENT)
		{
			fwrite(&m_ColorMapData + (m_ColorMapFirstEntryIndex * m_ColorMapEntrySize / 8), m_ColorMapLength * m_ColorMapEntrySize / 8, 1, _file);
		}

		if (IsCompressed(*this))
		{
			//When i support RLE, need to update here!
		}
		else
		{
			fwrite(&m_Pixels[0], SizeInBytes(), 1, _file);
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

	const uint8_t* NeighbourPtr(const TGA_Format &format, int xIndex, int yIndex)
	{
		CLAMP_PIXEL(xIndex, 0, format.m_ImageWidth - 1, yIndex, 0, format.m_ImageHeigh - 1);
		return &format.m_Pixels[(yIndex * format.m_Channels) + xIndex * (format.m_ImagePixelDepth > 24 ? format.m_ImagePixelDepth > 16 ? 4 : 3 : 3)];
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

	void OnImageResize(TGA_Format &newFormat, float resizeMultiplier)
	{
#ifdef USE_LOG_TIME
		std::chrono::high_resolution_clock::time_point _startTime = std::chrono::high_resolution_clock::now();
#endif // USE_LOG_TIME

		//Fill members of the new resized version
		//let's start with the idintical ones, info that will probably remain the same
		newFormat.m_Id = m_Id;
		newFormat.m_ColorMapData = m_ColorMapData;
		newFormat.m_IdLength = m_IdLength;
		newFormat.m_ColorMapType = m_ColorMapType;
		newFormat.m_ImageType = m_ImageType;
		newFormat.m_ColorMapFirstEntryIndex = m_ColorMapFirstEntryIndex;
		newFormat.m_ColorMapLength = m_ColorMapLength;
		newFormat.m_ColorMapEntrySize = m_ColorMapEntrySize;
		newFormat.m_ImageOriginX = m_ImageOriginX;
		newFormat.m_ImageOriginY = m_ImageOriginY;
		newFormat.m_ImagePixelDepth = m_ImagePixelDepth;
		newFormat.m_ImageDescription = m_ImageDescription;

		//of course the diminsions will be based on the scaleMultiplier
		newFormat.m_ImageWidth = uint16_t(float(m_ImageWidth)*resizeMultiplier);
		newFormat.m_ImageHeigh = uint16_t(float(m_ImageHeigh)*resizeMultiplier);

		newFormat.m_Channels = newFormat.m_ImageWidth * (newFormat.m_ImagePixelDepth > 24 ? newFormat.m_ImagePixelDepth > 16 ? 4 : 3 : 3);

		//expand or shrink, to fit the amount of pixels and channels for the new image size [NewWidth*NewHigh*Depth/8b]
		newFormat.m_Pixels.resize((const unsigned int)(
			(m_ImageWidth*resizeMultiplier) *
			(m_ImageHeigh*resizeMultiplier) *
			(m_ImagePixelDepth) / 8));

		uint8_t *_currentRow = &newFormat.m_Pixels[0];
		for (int y = 0; y < newFormat.m_ImageHeigh; y++)
		{
			uint8_t *_pixel = _currentRow;
			float _vertical = float(y) / float(newFormat.m_ImageHeigh - 1);
			for (int x = 0; x < newFormat.m_ImageWidth; x++)
			{
				float _horizontal = float(x) / float(newFormat.m_ImageWidth - 1);

				//start resampling in bilinear
				//TODO::may be good move this to its own function later. Also can put macros here to measure the bilinear time only if needed

				//ease the move between pixels of the TGA vertically and horizontally
				float _w = (_horizontal * m_ImageWidth);
				float _h = (_vertical * m_ImageHeigh);

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
				auto _BL = NeighbourPtr(*this, _indexX + 1, _indexY + 0);
				auto _BR = NeighbourPtr(*this, _indexX + 1, _indexY + 1);
				auto _TL = NeighbourPtr(*this, _indexX + 0, _indexY + 0);
				auto _TR = NeighbourPtr(*this, _indexX + 0, _indexY + 1);

				std::array<uint8_t, 3> _outRGB;
				std::array<uint8_t, 4> _outRGBA;

				//interpolate the colors for the new pixels
				if (m_ImagePixelDepth > 24) //32RGBA
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
			_currentRow += newFormat.m_Channels;
		}

#ifdef USE_LOG_TIME
		std::chrono::high_resolution_clock::time_point _endTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> _duration = _endTime - _startTime;
		LOG("Time Spent - Resizing: " << _duration.count()* 1000.f << "ms");
#endif // USE_LOG_TIME
	}
};
