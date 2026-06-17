#pragma once

struct BoundingBox;

void debug_overlay_draw(int source_width, int source_height, const BoundingBox &bbox,
			float scale_x, float scale_y, float offset_x,
			float offset_y, bool bbox_valid);
