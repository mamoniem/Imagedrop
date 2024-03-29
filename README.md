# Imagedrop
Imagedrop started as experimental test project to write a fast console application that can take a TGA image and scale down by half. Later been decided to be improved and converted into an entire full library that can be used as console application, or simply added to any project. With the ability to scale several image formats, up or down while keeping the best out of it, and even store the new result as totally different format.

**The current features are:**

- Full Commandline support
- Ability to scale up or down
- Ability to define a new file name
- Full read & write TGA file formats
- Read BMP file formats
- 32b & 24b images support
- [Bilinear interpolation](https://en.wikipedia.org/wiki/Bilinear_interpolation) support


**What is coming:**

- Excution for multiple images at the same time
- Multithreaded multiple images processing
- Multithreaded single image processing (experimental)
- RLE support
- 16b & 8b images support
- [Nearest interpolation](https://en.wikipedia.org/wiki/Nearest-neighbor_interpolation)
- [Bicubic interpolation](https://en.wikipedia.org/wiki/Bicubic_interpolation)
- [Trilinear interpolation](https://en.wikipedia.org/wiki/Trilinear_interpolation)
- [Lanczos resampling](https://en.wikipedia.org/wiki/Lanczos_resampling)
- [Staristep interpolation](https://en.wikipedia.org/wiki/Stairstep_interpolation) 
- Other file formats (TBD)


## References ##

**BMP Specifications**

- [https://en.wikipedia.org/wiki/BMP_file_format](https://en.wikipedia.org/wiki/BMP_file_format)
- [https://web.archive.org/web/20080912171714/http://www.fortunecity.com/skyscraper/windows/364/bmpffrmt.html](https://web.archive.org/web/20080912171714/http://www.fortunecity.com/skyscraper/windows/364/bmpffrmt.html)
- [http://www.digicamsoft.com/bmp/bmp.html](http://www.digicamsoft.com/bmp/bmp.html)

**TGA Specifications**

- [https://en.wikipedia.org/wiki/Truevision_TGA](https://en.wikipedia.org/wiki/Truevision_TGA)
- [http://www.dca.fee.unicamp.br/~martino/disciplinas/ea978/tgaffs.pdf](http://www.dca.fee.unicamp.br/~martino/disciplinas/ea978/tgaffs.pdf)

**Mathematics/Interpolation**

- [https://en.wikipedia.org/wiki/Bilinear_interpolation](https://en.wikipedia.org/wiki/Bilinear_interpolation)
- [https://en.wikipedia.org/wiki/Linear_interpolation](https://en.wikipedia.org/wiki/Linear_interpolation)
