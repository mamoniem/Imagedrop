#pragma once

//Bits (colormap, image type & description)
#define TGA_COLOR_MAP_TYPE_NO_COLOR_MAP						0
#define TGA_COLOR_MAP_TYPE_PRESENT							1

#define TGA_IMAGE_TYPE_NO_DATA								0
#define TGA_IMAGE_TYPE_UNCOMPRESSED_COLOR_MAPPED			1
#define TGA_IMAGE_TYPE_UNCOMPRESSED_TRUE_COLOR				2
#define TGA_IMAGE_TYPE_UNCOMPRESSED_GRAYSCALE				3
#define TGA_IMAGE_TYPE_RLE_ENCODED_COLOR_MAPPED				9
#define TGA_IMAGE_TYPE_RLE_ENCODED_TRUE_COLOR				10
#define TGA_IMAGE_TYPE_RLE_ENCODED_GRAYSCALE				11

#define TGA_SPECIFICATION_DESCRIPTION_ALPHA_DEPTH			(uint8_t)(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3)
#define TGA_SPECIFICATION_DESCRIPTION_RIGHT_TO_LEFT			(uint8_t)(1 << 4)
#define TGA_SPECIFICATION_DESCRIPTION_LEFT_TO_RIGHT			(uint8_t)(1 << 5)

#define BMP_PIXEL_FORMAT_1_BPP								1
#define BMP_PIXEL_FORMAT_2_BPP								2
#define BMP_PIXEL_FORMAT_4_BPP								4
#define BMP_PIXEL_FORMAT_8_BPP								8
#define BMP_PIXEL_FORMAT_16_BPP								16
#define BMP_PIXEL_FORMAT_24_BPP								24
#define BMP_PIXEL_FORMAT_32_BPP								32

#define BMP_COMPRESSION_METHOD_BI_RGB						0
#define BMP_COMPRESSION_METHOD_BI_RLE8						1
#define BMP_COMPRESSION_METHOD_BI_RLE4						2
#define BMP_COMPRESSION_METHOD_BI_BITFIELDS					3
#define BMP_COMPRESSION_METHOD_BI_JPEG						4
#define BMP_COMPRESSION_METHOD_BI_PNG						5
#define BMP_COMPRESSION_METHOD_BI_ALPHABITFIELDS			6
#define BMP_COMPRESSION_METHOD_BI_CMYK						7
#define BMP_COMPRESSION_METHOD_BI_CMYKRLE8					8
#define BMP_COMPRESSION_METHOD_BI_CMYKRLE4					9