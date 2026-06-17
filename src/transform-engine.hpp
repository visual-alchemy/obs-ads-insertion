#pragma once

enum class FitMode;

struct TransformResult {
	float scale_x;
	float scale_y;
	float offset_x;
	float offset_y;
};

TransformResult transform_calculate(int source_width, int source_height,
				    int bbox_x, int bbox_y, int bbox_w,
				    int bbox_h, FitMode fit_mode,
				    int padding);
