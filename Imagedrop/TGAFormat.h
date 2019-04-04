/*
More about the file format specification
https://en.wikipedia.org/wiki/Truevision_TGA
http://www.dca.fee.unicamp.br/~martino/disciplinas/ea978/tgaffs.pdf
http://tfc.duke.free.fr/coding/tga_specs.pdf
*/
#pragma once

#include <iostream>
#include <vector>

/*
- I decided to go with class
- All public
- No Getters and Setters, and they make no sense in such small size commandline app
- Mehtods are independent as fucntions, not within a class, so I can extend this later
	to read more file types (no move them within the class as methods if inhiritaned this
	in the future from let' say a parent "ImageFile" class)
*/
class TGA_Format
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
	TGA_Format() {}
	//Just in case i forget to deallocate something, this may be not needed later
	~TGA_Format()
	{
		//just in case
		m_pixels.clear();
		m_pixels.shrink_to_fit();
		free(m_id);
		free(m_colorMapData);
	}
};

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
