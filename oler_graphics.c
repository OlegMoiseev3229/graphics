#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<math.h>

typedef union {
	uint32_t rgba;
	struct {
		uint8_t a;
		uint8_t b;
		uint8_t g;
		uint8_t r;
	};
} oler_Color;

typedef struct {
	size_t width;
	size_t height;
	oler_Color *pixels;
} oler_Bitmap;

float oler_distance_from_point_to_line(float x_start, float y_start, float x_end, float y_end, float x_point, float y_point) {
	float denom = fabsf((y_end - y_start)*x_point - (x_end - x_start)*y_point + x_end*y_start - y_end*x_start);
	float numer = sqrtf((y_end - y_start)*(y_end - y_start) + (x_end - x_start)*(x_end - x_start));
	return denom/numer;
}

float oler_point_distance(float x1, float y1, float x2, float y2) {
	return sqrtf((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
}

float oler_point_project_to_line_segment(float x_start, float y_start, float x_end, float y_end, float x_point, float y_point) { // returns a number from 0 to 1
	float apx = x_point - x_start;
	float apy = y_point - y_start;

	float t = (apx * (x_end - x_start) + apy * (y_end - y_start))/((x_end - x_start)*(x_end - x_start) + (y_end - y_start)*(y_end - y_start));
	if (t < 0) {
		t = 0;
	}
	if (t > 1) {
		t = 1;
	}
	return t;
}

size_t oler_load_ppm(const char *filepath, oler_Bitmap *bitmap) {
	FILE *file = fopen(filepath, "rb");
	size_t res = 0xCE;
	if (file == NULL) {
		fprintf(stderr, "Error reading a file %s at source line %d\n", filepath, __LINE__);
		res =  0; goto end;
	}
	char magic[2];
	if (fscanf(file, "%c", &magic[0]) != 1) {
		fprintf(stderr, "Error reading a file %s at source line %d\n", filepath, __LINE__);
		res =  0; goto end;
	}
	if (fscanf(file, "%c", &magic[1]) != 1) {
		fprintf(stderr, "Error reading a file %s at source line %d\n", filepath, __LINE__); res =  0; goto end;
	}

	if (magic[0] != 'P') {
		fprintf(stderr, "Error reading a file %s at source line %d, incorrect format\n", filepath, __LINE__);
		res =  0; goto end;
	}

	if (magic[1] != '6') {
		fprintf(stderr, "Error reading a file %s at source line %d, incorrect format\n", filepath, __LINE__);
		res =  0; goto end;
	}

	int width, height;
	if (fscanf(file, "%d", &width) != 1) {
		fprintf(stderr, "Error reading a file %s at source line %d, incorrect format\n", filepath, __LINE__);
		res =  0; goto end;
	}

	if (fscanf(file, "%d", &height) != 1) {
		fprintf(stderr, "Error reading a file %s at source line %d, incorrect format\n", filepath, __LINE__);
		res =  0; goto end;
	}

	if (!bitmap) {
		res =  width * height * sizeof(oler_Color); goto end;
	}
	bitmap->width = width;
	bitmap->height = height;
	if (!bitmap->pixels) {
		res =  width * height * sizeof(oler_Color); goto end;
	}

	int depth;
	if (fscanf(file, "%d\n", &depth) != 1) {
		fprintf(stderr, "Error reading a file %s at source line %d, incorrect format\n", filepath, __LINE__);
		res =  0; goto end;
	}
	if (depth != 255) {
		fprintf(stderr, "Error reading a file %s at source line %d, incorrect format\n", filepath, __LINE__);
		res =  0; goto end;
	}

	for (int y = 0; y < height; y++) {
		oler_Color *row = bitmap->pixels + y*width; //arena_get_memory(arena, sizeof(uint32_t)*width);
		for (int x = 0; x < width; x++) {
			uint8_t rgb[3];
			if (fread(rgb, sizeof(uint8_t), 3, file) != 3) {
					fprintf(stderr, "Error reading a file %s at source line %d on iteration x=%d y=%d, incorrect format\n", filepath, __LINE__, x, y);
					res =  0; goto end;
			}
			row[x] = (oler_Color) {.r=rgb[0], .g=rgb[1], .b=rgb[2], .a=0xFF}; //(0xFF | ((uint32_t)rgb[0] << 24) | ((uint32_t)rgb[1]<<16) | ((uint32_t)rgb[2]<<8));
		}
	}
	res =  1; goto end;
end:
	fclose(file);
	return res;
}

int oler_save_ppm(const char *filepath, oler_Bitmap bitmap) {
	FILE *file = fopen(filepath, "wb");
	int res = 0xCE;
	if (file == NULL) {
		fprintf(stderr, "Error writing to a file %s at source line %d\n", filepath, __LINE__);
		res =  0; goto end;
	}

	char magic[2] = {'P', '6'};
	int write_res = fwrite(magic, sizeof(magic[0]), sizeof(magic)/sizeof(magic[0]), file);
	if (write_res != 2) {
		fprintf(stderr, "Error writing to a file %s at source line %d\n", filepath, __LINE__);
		res =  0; goto end;
	}

	write_res = fprintf(file, "\n%ld %ld\n", bitmap.width, bitmap.height);
	if (write_res <= 0) {
		fprintf(stderr, "Error writing to a file %s at source line %d\n", filepath, __LINE__);
		res =  0; goto end;
	}

	write_res = fprintf(file, "255\n");
	if (write_res < 0) {
		fprintf(stderr, "Error writing to a file %s at source line %d\n", filepath, __LINE__);
		res =  0; goto end;
	}

	for (int y = 0; y < bitmap.height; y++) {
		for (int x = 0; x < bitmap.width; x++) {
			oler_Color pixel = bitmap.pixels[x + bitmap.width*y];
			fwrite(&pixel.r, 1, sizeof(pixel.r), file);
			fwrite(&pixel.g, 1, sizeof(pixel.g), file);
			fwrite(&pixel.b, 1, sizeof(pixel.b), file);
		}
	}
	res = 1;
end:
	fclose(file);
	return res;
}

void oler_vline(oler_Bitmap bitmap, int x, int y_start, int y_end, oler_Color color) {
	if (x < 0) {
		return;
	}
	if (x > bitmap.width) {
		return;
	}
	if (y_start > y_end) {
		int temp = y_start;
		y_start = y_end;
		y_end = temp;
	}
	for (int y = y_start; y <= y_end; y++) {
		if (y < 0) {
			continue;
		}
		if (y >= bitmap.height) {
			return;
		}
		bitmap.pixels[x + bitmap.width * y] = color;
	}
}

void oler_hline(oler_Bitmap bitmap, int y, int x_start, int x_end, oler_Color color) {
	if (y < 0) {
		return;
	}
	if (y >= bitmap.height) {
		return;
	}
	if (x_start > x_end) {
		int temp = x_start;
		x_start = x_end;
		x_end = temp;
	}

	for (int x = x_start; x <= x_end; x++) {
		if (x < 0) {
			continue;
		}
		if (x >= bitmap.width) {
			return;
		}
		bitmap.pixels[x + bitmap.width * y] = color;
	}
}

void oler_line(oler_Bitmap bitmap, float x_start, float y_start, float x_end, float y_end, oler_Color color) {
	if (x_start > x_end) {
		float temp = x_start;
		x_start = x_end;
		x_end = temp;
	}
	if (y_start > y_end) {
		float temp = y_start;
		y_start = y_end;
		y_end = temp;
	}
	if (y_end - y_start > x_end - x_start) {
		float slope = (x_end - x_start)/(y_end - y_start);
		for (float y = y_start; y < y_end; y++) {
			if (roundf(y) >= bitmap.height) {
				return;
			}
			float x = slope * (y - y_start) + x_start;
			int x_int = (int) roundf(x);
			if (x_int >= bitmap.width) {
				return;
			}
			bitmap.pixels[x_int + (int)roundf(y)*bitmap.width] = color;
	       	}
	} else {
		float slope = (y_end - y_start)/(x_end - x_start);
		for (float x = x_start; x < x_end; x++) {
			if (roundf(x) >= bitmap.width) {
				return;
			}
			float y = slope * (x - x_start) + y_start;
			int y_int = (int) roundf(y);
			if (y_int >= bitmap.height) {
				return;
			}
			bitmap.pixels[(int)roundf(x) + y_int*bitmap.width] = color;
		}
	}
}

void oler_line_width(oler_Bitmap bitmap, float x_start, float y_start, float x_end, float y_end, float width, oler_Color color) {
	if (x_start > x_end) {
		float temp = x_start;
		x_start = x_end;
		x_end = temp;
	}
	if (y_start > y_end) {
		float temp = y_start;
		y_start = y_end;
		y_end = temp;
	}
	float x_left = x_start - width;
	float x_right = x_end + width;
	float y_top = y_start - width;
	float y_bottom = y_end + width;

	for (int y = floorf(y_top); y < ceilf(y_bottom); y++) {
		if (y < 0) {
			continue;
		}
		if (y >= bitmap.height) {
			break;
		}
		for (int x = floorf(x_left); x < ceilf(x_right); x++) {
			if (x < 0) {
				continue;
			}
			if (x >= bitmap.width) {
				break;
			}
			float t = oler_point_project_to_line_segment(x_start, y_start, x_end, y_end, x + 0.5, y + 0.5);
			float x_proj = x_start + t * (x_end - x_start);
			float y_proj = y_start + t * (y_end - y_start);
			float distance = oler_point_distance(x + 0.5, y + 0.5, x_proj, y_proj);
			if (distance < width) {
				bitmap.pixels[x + y*bitmap.width] = color;
			}
		}
	}
}

void oler_rect(oler_Bitmap bitmap, int x_left, int y_top, int width, int height, oler_Color color) {
	for (int y = y_top; y < y_top + height; y++) {
		if (y < 0) {
			continue;
		}
		if (y > bitmap.height) {
			return;
		}
		for (int x = x_left; x < x_left + width; x++) {
			if (x < 0) {
				continue;
			}
			if (x > bitmap.width) {
				break;
			}
			bitmap.pixels[x + bitmap.width * y] = color;
		}
	}
}

