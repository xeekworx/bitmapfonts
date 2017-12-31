// xwfont_library.cpp : Defines the exported functions for the DLL application.
//

#include <SDKDDKVer.h>		// Including SDKDDKVer.h defines the highest available Windows platform.
#define WIN32_LEAN_AND_MEAN	// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include <stdio.h>
#include <math.h>

#include "xwfont_api.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#define WIDTH   640
#define HEIGHT  51

unsigned char image[HEIGHT][WIDTH];						/* origin is the upper left corner */

static void draw_bitmap(FT_Bitmap*  bitmap, FT_Int x, FT_Int y);
static void show_image(FILE* stream);

XWFONTAPI void xeekworx::test_xwfontapi(void)
{
	FT_Library		library;
	FT_Face			face;

	FT_GlyphSlot	slot;
	//FT_Matrix		matrix;								/* transformation matrix */
	FT_Vector		pen;								/* untransformed origin  */
	FT_Error		error;

	char			filename[] = "C:\\Windows\\Fonts\\arial.ttf";
	char			text[] = "Hello World!";

	//double			angle;
	int				target_height;
	size_t			n, num_chars;

	num_chars = strlen(text);
	//angle = (25.0 / 360) * 3.14159 * 2;					/* use 25 degrees     */
	target_height = HEIGHT;

	error = FT_Init_FreeType(&library);					/* initialize library */
														/* error handling omitted */

	error = FT_New_Face(library, filename, 0, &face);	/* create face object */
														/* error handling omitted */

	error = FT_Set_Char_Size(face, 50 * 64, 0,			/* use 50pt at 100dpi */
		100, 0);										/* set character size */
														/* error handling omitted */

	slot = face->glyph;

	/* set up matrix */
	//matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
	//matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
	//matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
	//matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);

	/* the pen position in 26.6 cartesian space coordinates; */
	/* start at (300,200) relative to the upper left corner  */
	pen.x = 1;// 300 * 64;
	pen.y = 1;// (target_height - 200) * 64;

	for (n = 0; n < num_chars; n++)
	{
		/* set transformation */
		FT_Set_Transform(face, NULL, &pen);

		/* load glyph image into the slot (erase previous one) */
		error = FT_Load_Char(face, text[n], FT_LOAD_RENDER);
		if (error)
			continue;                 /* ignore errors */

									  /* now, draw to our target surface (convert position) */
		draw_bitmap(&slot->bitmap,
			slot->bitmap_left,
			target_height - slot->bitmap_top);

		/* increment pen position */
		pen.x += slot->advance.x;
		pen.y += slot->advance.y;
	}

	FILE* f = NULL;
	fopen_s(&f, "output.txt", "w");
	if (f) {
		show_image(f);
		fclose(f);
	}

	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

static void
draw_bitmap(FT_Bitmap*  bitmap,
	FT_Int      x,
	FT_Int      y)
{
	FT_Int  i, j, p, q;
	FT_Int  x_max = x + bitmap->width;
	FT_Int  y_max = y + bitmap->rows;


	for (i = x, p = 0; i < x_max; i++, p++)
	{
		for (j = y, q = 0; j < y_max; j++, q++)
		{
			if (i < 0 || j < 0 ||
				i >= WIDTH || j >= HEIGHT)
				continue;

			image[j][i] |= bitmap->buffer[q * bitmap->width + p];
		}
	}
}

static void
show_image(FILE* stream)
{
	int  i, j;


	for (i = 0; i < HEIGHT; i++)
	{
		for (j = 0; j < WIDTH; j++)
			fputc(image[i][j] == 0 ? ' '
				: image[i][j] < 128 ? '+'
				: '*',
				stream);
		fputc('\n', stream);
	}
}