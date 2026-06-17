#pragma once
#include <obs-module.h>

struct BoundingBox {
	int min_x;
	int min_y;
	int max_x;
	int max_y;
	bool valid;

	BoundingBox()
		: min_x(0),
		  min_y(0),
		  max_x(0),
		  max_y(0),
		  valid(false)
	{
	}

	int width() const { return max_x - min_x; }
	int height() const { return max_y - min_y; }
};

BoundingBox alpha_analyze(const uint8_t *rgba_data, int width, int height,
			  int linesize, int threshold);
