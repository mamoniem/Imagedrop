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
"\0\0\0\0"					//may be add some info here if needed for copyright & such
"\0\0\0\0"					//may be add some info here if needed for copyright & such
"TRUEVISION-XFILE"
"."
"\0";
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
	uint8_t						*m_id;
	uint8_t						*m_colorMapData;
	std::vector<uint8_t>		m_pixels;

	//ID Length - date, time, ver, etc.			[1byte]
	uint8_t						m_idLength;

	//Color Map Type							[1byte]
	uint8_t						m_colorMapType;

	//Image Type								[1byte]
	uint8_t						m_imageType;

	//Color Map Specification					[5bytes]
	uint16_t					m_colorMapFirstEntryIndex;
	uint16_t					m_colorMapLength;
	uint8_t						m_colorMapEntrySize;

	//Image Specification						[10bytes]
	uint16_t					m_imageOriginX;
	uint16_t					m_imageOriginY;
	uint16_t					m_imageWidth;
	uint16_t					m_imageHeigh;
	uint8_t						m_imagePixelDepth;
	uint8_t						m_imageDescription;

	//Extension

	//make more sense of the number 32 is 4 channels and 24 is 3 channels
	long						m_channels;

	//I don't need so far to initialize the constructor with any values
	TGA_Format()
	{
		ImageFormat = EImageFormat::TGA;
	}
	//Just in case i forget to deallocate something, this may be not needed later
	~TGA_Format()
	{
		//just in case
		m_pixels.clear();
		m_pixels.shrink_to_fit();
		free(m_id);
		free(m_colorMapData);
	}

	size_t SizeInBytes(const TGA_Format &format)
	{
		//this shall match the size found in [Right click-> properties] within explorer, if not, then there is an issue
		return (format.m_imageWidth * format.m_imageHeigh * format.m_imagePixelDepth / 8);
	}

	uint8_t IsGrayScale(const TGA_Format &format)
	{
		return(
			format.m_imageType == TGA_IMAGE_TYPE_UNCOMPRESSED_GRAYSCALE ||
			format.m_imageType == TGA_IMAGE_TYPE_RLE_ENCODED_GRAYSCALE
			);
	}

	uint8_t IsCompressed(const TGA_Format &format)
	{
		return(
			format.m_imageType == TGA_IMAGE_TYPE_RLE_ENCODED_COLOR_MAPPED ||
			format.m_imageType == TGA_IMAGE_TYPE_RLE_ENCODED_TRUE_COLOR ||
			format.m_imageType == TGA_IMAGE_TYPE_RLE_ENCODED_GRAYSCALE
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

		m_id = NULL;
		m_colorMapData = NULL;

		//read from file with the same order & store into the TGA blocks.
		//ID Length						[1byte] 8
		//Color Map Type				[1byte] 8
		//Image Type					[1byte] 8
		//Color Map Specification		[5bytes] 16, 16, 8
		//Image Specification			[10bytes] 16, 16, 16, 16, 8, 8
		fread(&m_idLength, 1, 1, _file);

		fread(&m_colorMapType, 1, 1, _file);

		fread(&m_imageType, 1, 1, _file);

		fread(&m_colorMapFirstEntryIndex, 2, 1, _file);
		fread(&m_colorMapLength, 2, 1, _file);
		fread(&m_colorMapEntrySize, 1, 1, _file);

		fread(&m_imageOriginX, 2, 1, _file);
		fread(&m_imageOriginY, 2, 1, _file);
		fread(&m_imageWidth, 2, 1, _file);
		fread(&m_imageHeigh, 2, 1, _file);
		fread(&m_imagePixelDepth, 1, 1, _file);
		fread(&m_imageDescription, 1, 1, _file);

		if (m_imagePixelDepth < 24)
		{
			LOG("ERR	m_imagePixelDepth is neither 32b nor 24b");
			THROW_ERROR("m_imagePixelDepth is neither 32b nor 24b");
		}

		//It's a good place to check if any of the read values is invalid, if needed.

		//Resolve the core required data
		if (m_idLength > 0)
		{
			m_id = (uint8_t*)malloc(m_idLength);

			if (m_id == NULL)
			{
				LOG("ERR	m_id is NULL");
				THROW_ERROR("m_id is NULL");
			}

			fread(&m_id, m_idLength, 1, _file);
		}

		if (m_colorMapType == TGA_COLOR_MAP_TYPE_PRESENT)
		{
			m_colorMapData = (uint8_t*)malloc((m_colorMapFirstEntryIndex + m_colorMapLength) * m_colorMapEntrySize / 8);

			if (m_colorMapData == NULL)
			{
				LOG("ERR	m_colorMapData is NULL");
				THROW_ERROR("m_colorMapData is NULL");
			}

			fread(&m_colorMapData + (m_colorMapFirstEntryIndex * m_colorMapEntrySize / 8), m_colorMapLength * m_colorMapEntrySize / 8, 1, _file);
		}

		m_pixels.resize(SizeInBytes(*this));

		//check for RLE
		if (IsCompressed(*this))
		{
			LOG("ERR	RLE not supported yet!");
			THROW_ERROR("RLE not supported yet!");
		}
		else
		{
			fread(&m_pixels[0], SizeInBytes(*this), 1, _file);
		}

		m_channels = m_imageWidth * (m_imagePixelDepth > 24 ? m_imagePixelDepth > 16 ? 4 : 3 : 3);

		//close the file
		fclose(_file);

#ifdef USE_LOG_TIME
		std::chrono::high_resolution_clock::time_point _endTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> _duration = _endTime - _startTime;
		LOG("Time Spent - Reading: " << _duration.count()* 1000.f << "ms");
#endif // USE_LOG_TIME

		LOG("====================R=E=A=D=====================");
		LOG("ImageWidth: " << m_imageWidth);
		LOG("ImageHeigh: " << m_imageHeigh);
		LOG("ImageSize: " << SizeInBytes(*this) << "Bytes");
		LOG("ImageBitsPerPixel: " << size_t(m_imagePixelDepth) << "bit");
		LOG("================================================");
	}

	void OnImageWrite(const char *path) override
	{
		LOG("===================W=R=I=T=E====================");
		LOG("ImageWidth: " << m_imageWidth);
		LOG("ImageHeigh: " << m_imageHeigh);
		LOG("ImageSize: " << SizeInBytes(*this) << "Bytes");
		LOG("ImageBitsPerPixel: " << size_t(m_imagePixelDepth) << "bit");
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
		fwrite(&m_idLength, 1, 1, _file);

		fwrite(&m_colorMapType, 1, 1, _file);

		fwrite(&m_imageType, 1, 1, _file);

		fwrite(&m_colorMapFirstEntryIndex, 2, 1, _file);
		fwrite(&m_colorMapLength, 2, 1, _file);
		fwrite(&m_colorMapEntrySize, 1, 1, _file);

		fwrite(&m_imageOriginX, 2, 1, _file);
		fwrite(&m_imageOriginY, 2, 1, _file);
		fwrite(&m_imageWidth, 2, 1, _file);
		fwrite(&m_imageHeigh, 2, 1, _file);
		fwrite(&m_imagePixelDepth, 1, 1, _file);
		fwrite(&m_imageDescription, 1, 1, _file);

		if (m_idLength > 0)
		{
			fwrite(&m_id, m_idLength, 1, _file);
		}

		if (m_colorMapType == TGA_COLOR_MAP_TYPE_PRESENT)
		{
			fwrite(&m_colorMapData + (m_colorMapFirstEntryIndex * m_colorMapEntrySize / 8), m_colorMapLength * m_colorMapEntrySize / 8, 1, _file);
		}

		if (IsCompressed(*this))
		{
			//When i support RLE, need to update here!
		}
		else
		{
			fwrite(&m_pixels[0], SizeInBytes(*this), 1, _file);
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

	void OnImageResize(TGA_Format &newFormat, float resizeMultiplier)
	{
#ifdef USE_LOG_TIME
		std::chrono::high_resolution_clock::time_point _startTime = std::chrono::high_resolution_clock::now();
#endif // USE_LOG_TIME

		//Fill members of the new resized version
		//let's start with the idintical ones, info that will probably remain the same
		newFormat.m_id = m_id;
		newFormat.m_colorMapData = m_colorMapData;
		newFormat.m_idLength = m_idLength;
		newFormat.m_colorMapType = m_colorMapType;
		newFormat.m_imageType = m_imageType;
		newFormat.m_colorMapFirstEntryIndex = m_colorMapFirstEntryIndex;
		newFormat.m_colorMapLength = m_colorMapLength;
		newFormat.m_colorMapEntrySize = m_colorMapEntrySize;
		newFormat.m_imageOriginX = m_imageOriginX;
		newFormat.m_imageOriginY = m_imageOriginY;
		newFormat.m_imagePixelDepth = m_imagePixelDepth;
		newFormat.m_imageDescription = m_imageDescription;

		//of course the diminsions will be based on the scaleMultiplier
		newFormat.m_imageWidth = uint16_t(float(m_imageWidth)*resizeMultiplier);
		newFormat.m_imageHeigh = uint16_t(float(m_imageHeigh)*resizeMultiplier);

		newFormat.m_channels = newFormat.m_imageWidth * (newFormat.m_imagePixelDepth > 24 ? newFormat.m_imagePixelDepth > 16 ? 4 : 3 : 3);

		//expand or shrink, to fit the amount of pixels and channels for the new image size [NewWidth*NewHigh*Depth/8b]
		newFormat.m_pixels.resize((const unsigned int)(
			(m_imageWidth*resizeMultiplier) *
			(m_imageHeigh*resizeMultiplier) *
			(m_imagePixelDepth) / 8));

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
				float _w = (_horizontal * m_imageWidth);
				float _h = (_vertical * m_imageHeigh);

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
				if (m_imagePixelDepth > 24) //32RGBA
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
};
