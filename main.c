#include<stdio.h>
#include "oler_graphics.c"

int main() {
	oler_Bitmap bitmap = (oler_Bitmap) {.width = 100, .height = 100, .pixels=malloc(sizeof(oler_Color)*100*100)};
	for (int i = 0; i < 100*100; i++) {
		bitmap.pixels[i] = (oler_Color) 0xFF007FFFu;
	}
	oler_vline(bitmap, 50, 10, 40, (oler_Color) 0x00FF7FFFu);
	oler_hline(bitmap, 50, 10, 40, (oler_Color) 0x00FF7FFFu);
	oler_rect(bitmap, 80, 30, 10, 20, (oler_Color) 0x000000FFu);

	//oler_line(bitmap, 10, 10, 20, 50, (oler_Color) 0x00FFF7FFu);

	oler_line_width(bitmap, 50, 20, 99, 99, 5., (oler_Color) 0x00FFF7FFu);
	oler_line_width(bitmap, 20, 50, 100, 100, 5., (oler_Color) 0x00FFF7FFu);

	//oler_line_width(bitmap, 80, 50, 130, 130, 5.5, (oler_Color) 0x00FFF7FFu);

	oler_save_ppm("image3.ppm", bitmap);
}
