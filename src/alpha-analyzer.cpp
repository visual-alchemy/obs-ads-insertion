#include "alpha-analyzer.hpp"
#include <algorithm>

BoundingBox alpha_analyze(const uint8_t *rgba_data, int width, int height,
			  int linesize, int threshold)
{
	BoundingBox bbox;

	if (!rgba_data || width <= 0 || height <= 0)
		return bbox;

	int min_x = width, min_y = height, max_x = 0, max_y = 0;
	bool found = false;

	for (int y = 0; y < height; y++) {
		const uint8_t *row = rgba_data + (size_t)y * linesize;
		for (int x = 0; x < width; x++) {
			uint8_t a = row[x * 4 + 3];
			if (a <= threshold) {
				if (x < min_x)
					min_x = x;
				if (x > max_x)
					max_x = x;
				if (y < min_y)
					min_y = y;
				if (y > max_y)
					max_y = y;
				found = true;
			}
		}
	}

	if (found) {
		bbox.min_x = min_x;
		bbox.min_y = min_y;
		bbox.max_x = max_x + 1;
		bbox.max_y = max_y + 1;
		bbox.valid = true;
	}

	return bbox;
}
