#pragma once

#include "Settings.h"

//------------------
//Macro Functions //
//------------------
//Logging
#if USE_LOG == 1
#define LOG(msg) std::cout<<"Log:"<<msg<<std::endl
#else
#define LOG(msg)
#endif

//Throwing Runtime Errs
#if USE_THROW_ERRORS == 1
#define THROW_ERROR(msg) throw std::runtime_error(msg)
#else
#define THROW_ERROR(msg)
#endif

//Main waita for a cin
#if USE_WAIT_FOR_INPUT == 1
#define WAIT_INPUT std::cin.get(); LOG("Done!")
#else
#define WAIT_INPUT LOG("Done!")
#endif


//------------------
//Math Functions //
//------------------
//Clamp
#define CLAMP(v, min, max) if (v < min) { v = min; } else if (v > max) { v = max; }

//Clamp a Pixel (a doubled clamp)
#define CLAMP_PIXEL(x, minX, maxX, y, minY, maxY) if (x < minX) { x = minX; } else if (x > maxX) { x = maxX;} if (y < minY) { y = minY; } else if (y > maxY) { y = maxY;}

//Lerp
#define LERP(a, b, v, t){ v = a * (1.0f - t) + b * t;}