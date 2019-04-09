#pragma once

#include <iostream>
#include <vector>

enum EImageFormat
{
	BMP,
	JPG,
	PNG,
	TGA
};

class ImageFormatBase
{
public:
	EImageFormat ImageFormat;

	virtual size_t SizeInBytes() { return 0; }

	virtual void OnImageRead(const char *path) {} //virtual void OnImageRead(ImageFormatBase &format, const char *path);
	virtual void OnImageWrite(const char *path) {} //virtual void OnImageWrite(ImageFormatBase &format, const char *path);
	virtual void OnImageResize(ImageFormatBase &newFormat, float resizeMultiplier) {}
		
	ImageFormatBase(){}
	~ImageFormatBase() {}
};