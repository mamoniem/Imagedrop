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

#include <iostream>
#include <vector>

class BMP_Format
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

	BMP_Format() {}
	~BMP_Format()
	{
		m_Pixels.clear();
		m_Pixels.shrink_to_fit();
	}

};