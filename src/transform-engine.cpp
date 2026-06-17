#include "transform-engine.hpp"
#include "settings.hpp"
#include <algorithm>

TransformResult transform_calculate(int source_width, int source_height,
				    int bbox_x, int bbox_y, int bbox_w,
				    int bbox_h, FitMode fit_mode,
				    int padding)
{
	TransformResult result = {1.0f, 1.0f, 0.0f, 0.0f};

	if (source_width <= 0 || source_height <= 0 || bbox_w <= 0 || bbox_h <= 0)
		return result;

	int pad = padding;
	int inner_w = std::max(1, bbox_w - pad * 2);
	int inner_h = std::max(1, bbox_h - pad * 2);

	float source_aspect = (float)source_width / (float)source_height;
	float bbox_aspect = (float)inner_w / (float)inner_h;

	switch (fit_mode) {
	case FitMode::CONTAIN: {
		float scale = (source_aspect > bbox_aspect)
				      ? (float)inner_w / (float)source_width
				      : (float)inner_h / (float)source_height;
		result.scale_x = scale;
		result.scale_y = scale;
		result.offset_x = (float)bbox_x + (float)pad;
		result.offset_y = (float)bbox_y + (float)pad;
		break;
	}
	case FitMode::COVER: {
		float scale = (source_aspect > bbox_aspect)
				      ? (float)inner_h / (float)source_height
				      : (float)inner_w / (float)source_width;
		result.scale_x = scale;
		result.scale_y = scale;
		result.offset_x = (float)bbox_x - (source_width * scale - (float)inner_w) * 0.5f;
		result.offset_y = (float)bbox_y - (source_height * scale - (float)inner_h) * 0.5f;
		break;
	}
	case FitMode::STRETCH:
		result.scale_x = (float)inner_w / (float)source_width;
		result.scale_y = (float)inner_h / (float)source_height;
		result.offset_x = (float)bbox_x + (float)pad;
		result.offset_y = (float)bbox_y + (float)pad;
		break;
	}

	return result;
}
