#include "debug-overlay.hpp"
#include "alpha-analyzer.hpp"
#include <graphics/matrix4.h>

void debug_overlay_draw(int source_width, int source_height, const BoundingBox &bbox,
			float scale_x, float scale_y, float offset_x,
			float offset_y, bool bbox_valid)
{
	UNUSED_PARAMETER(source_width);
	UNUSED_PARAMETER(source_height);

	if (!bbox_valid)
		return;

	gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
	if (!solid)
		return;

	gs_technique_t *tech = gs_effect_get_technique(solid, "Solid");
	if (!tech)
		return;

	gs_eparam_t *color_param = gs_effect_get_param_by_name(solid, "color");
	if (!color_param)
		return;

	gs_technique_begin(tech);
	gs_technique_begin_pass(tech, 0);

	gs_effect_set_color(color_param, 0xFF00FF00);

	gs_matrix_push();

	struct matrix4 transform;
	matrix4_identity(&transform);
	matrix4_translate3f(&transform, &transform, offset_x, offset_y, 0.0f);
	matrix4_scale3f(&transform, &transform,
			(float)bbox.width() * scale_x,
			(float)bbox.height() * scale_y, 1.0f);
	gs_matrix_mul(&transform);

	gs_render_start(false);

	gs_vertex2f(0.0f, 0.0f);
	gs_vertex2f(1.0f, 0.0f);
	gs_vertex2f(1.0f, 1.0f);
	gs_vertex2f(0.0f, 1.0f);
	gs_vertex2f(0.0f, 0.0f);

	gs_render_stop(GS_LINESTRIP);

	gs_matrix_pop();

	gs_technique_end_pass(tech);
	gs_technique_end(tech);
}
