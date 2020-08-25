/* APNG Disassembler 2.8
 *
 * Deconstructs APNG files into individual frames.
 * http://apngdis.sourceforge.net
 * Copyright (c) 2010-2015 Max Stepin
 * maxst at users.sourceforge.net
 *
 * zlib license
 * ------------
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "libpng\png.h" // original (unpatched) libpng is ok

#define notabc(c) ((c) < 65 || (c) > 122 || ((c) > 90 && (c) < 97))

#define id_IHDR 0x52444849
#define id_acTL 0x4C546361
#define id_fcTL 0x4C546366
#define id_IDAT 0x54414449
#define id_fdAT 0x54416466
#define id_IEND 0x444E4549

struct CHUNK { unsigned char * p; unsigned int size; };
struct APNGFrame { unsigned char * p, ** rows; unsigned int w, h, delay_num, delay_den; };

const unsigned long cMaxPNGSize = 1000000UL;

void info_fn(png_structp png_ptr, png_infop info_ptr);
void row_fn(png_structp png_ptr, png_bytep new_row, png_uint_32 row_num, int pass);
void compose_frame(unsigned char ** rows_dst, unsigned char ** rows_src, unsigned char bop, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
inline unsigned int read_chunk(FILE * f, CHUNK * pChunk);
int processing_start(png_structp & png_ptr, png_infop & info_ptr, void * frame_ptr, bool hasInfo, CHUNK & chunkIHDR, std::vector<CHUNK>& chunksInfo);
int processing_data(png_structp png_ptr, png_infop info_ptr, unsigned char * p, unsigned int size);
int processing_finish(png_structp png_ptr, png_infop info_ptr);
int load_apng(char * szIn, std::vector<APNGFrame>& frames);
void save_png(char * szOut, APNGFrame * frame);
void save_txt(char * szOut, APNGFrame * frame);
