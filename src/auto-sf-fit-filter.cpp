#include "auto-sf-fit-filter.hpp"
#include "debug-overlay.hpp"
#include <graphics/matrix4.h>
#include <graphics/vec2.h>
#include <algorithm>
#include <cmath>

struct RecursionGuard {
	bool &flag;
	RecursionGuard(bool &f) : flag(f) { flag = true; }
	~RecursionGuard() { flag = false; }
};

static const char *FILTER_ID = "auto_sf_fit_filter";

static bool do_recalc(AutoSFFilter *f, bool force = false);

static const char *filter_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("AutoSFFitFilter");
}

static void resolve_sf(AutoSFFilter *f)
{
	if (f->sf_source_ref) {
		obs_weak_source_release(f->sf_source_ref);
		f->sf_source_ref = nullptr;
	}
	if (!f->settings.sf_source_name.empty()) {
		obs_source_t *sf = obs_get_source_by_name(
			f->settings.sf_source_name.c_str());
		if (sf) {
			f->sf_source_ref = obs_source_get_weak_source(sf);
			obs_source_release(sf);
		}
	}
}

static void *filter_create(obs_data_t *settings, obs_source_t *context)
{
	AutoSFFilter *f = new AutoSFFilter();
	f->context = context;
	settings_load(&f->settings, settings);
	resolve_sf(f);

	f->bbox.min_x = f->settings.cached_bbox_min_x;
	f->bbox.min_y = f->settings.cached_bbox_min_y;
	f->bbox.max_x = f->settings.cached_bbox_max_x;
	f->bbox.max_y = f->settings.cached_bbox_max_y;
	f->bbox.valid = f->settings.cached_bbox_valid;

	obs_enter_graphics();
	f->texrender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);

	char *error_str = nullptr;
	const char *effect_src = 
		"uniform float4x4 ViewProj;\n"
		"uniform texture2d image;\n"
		"uniform texture2d sf_image;\n"
		"uniform float2 scale;\n"
		"uniform float2 offset;\n"
		"uniform float2 sf_scale;\n"
		"uniform float2 sf_offset;\n"
		"uniform float sf_opacity;\n"
		"sampler_state def_sampler {\n"
		"    Filter   = Linear;\n"
		"    AddressU = Clamp;\n"
		"    AddressV = Clamp;\n"
		"};\n"
		"struct VertData {\n"
		"    float4 pos : POSITION;\n"
		"    float2 uv  : TEXCOORD0;\n"
		"};\n"
		"VertData VSDefault(VertData v_in) {\n"
		"    VertData v_out;\n"
		"    v_out.pos = mul(float4(v_in.pos.xyz, 1.0), ViewProj);\n"
		"    v_out.uv  = v_in.uv;\n"
		"    return v_out;\n"
		"}\n"
		"float4 PSDrawCombined(VertData v_in) : TARGET {\n"
		"    float2 parent_uv = (v_in.uv - offset) / scale;\n"
		"    float4 parent_col;\n"
		"    if (parent_uv.x >= 0.0 && parent_uv.x <= 1.0 && parent_uv.y >= 0.0 && parent_uv.y <= 1.0) {\n"
		"        parent_col = image.Sample(def_sampler, parent_uv);\n"
		"    } else {\n"
		"        parent_col = float4(0.0, 0.0, 0.0, 0.0);\n"
		"    }\n"
		"    float2 sf_uv = (v_in.uv - sf_offset) / sf_scale;\n"
		"    float4 sf_col;\n"
		"    if (sf_uv.x >= 0.0 && sf_uv.x <= 1.0 && sf_uv.y >= 0.0 && sf_uv.y <= 1.0) {\n"
		"        sf_col = sf_image.Sample(def_sampler, sf_uv);\n"
		"    } else {\n"
		"        sf_col = float4(0.0, 0.0, 0.0, 0.0);\n"
		"    }\n"
		"    sf_col.a *= sf_opacity;\n"
		"    sf_col.rgb *= sf_opacity;\n"
		"    float4 final_col;\n"
		"    final_col.rgb = parent_col.rgb * (1.0 - sf_col.a) + sf_col.rgb;\n"
		"    final_col.a = parent_col.a * (1.0 - sf_col.a) + sf_col.a;\n"
		"    return final_col;\n"
		"}\n"
		"technique Draw {\n"
		"    pass {\n"
		"        vertex_shader = VSDefault(v_in);\n"
		"        pixel_shader  = PSDrawCombined(v_in);\n"
		"    }\n"
		"}\n";
	f->combined_effect = gs_effect_create(effect_src, "combined_effect", &error_str);
	if (error_str) {
		blog(LOG_ERROR, "Failed to compile combined effect: %s", error_str);
		bfree(error_str);
	}
	obs_leave_graphics();

	return f;
}

static void filter_destroy(void *data)
{
	AutoSFFilter *f = static_cast<AutoSFFilter *>(data);
	if (obs_source_active(f->context) && f->sf_source_ref) {
		obs_source_t *sf = obs_weak_source_get_source(f->sf_source_ref);
		if (sf) {
			obs_source_dec_showing(sf);
			obs_source_dec_active(sf);
			obs_source_release(sf);
		}
	}
	delete f;
}

static void filter_update(void *data, obs_data_t *settings)
{
	AutoSFFilter *f = static_cast<AutoSFFilter *>(data);
	obs_source_t *old_sf = f->sf_source_ref ? obs_weak_source_get_source(f->sf_source_ref) : nullptr;

	settings_load(&f->settings, settings);
	resolve_sf(f);

	obs_source_t *new_sf = f->sf_source_ref ? obs_weak_source_get_source(f->sf_source_ref) : nullptr;

	if (obs_source_active(f->context)) {
		if (old_sf && old_sf != new_sf) {
			obs_source_dec_showing(old_sf);
			obs_source_dec_active(old_sf);
		}
		if (new_sf && old_sf != new_sf) {
			obs_source_inc_showing(new_sf);
			obs_source_inc_active(new_sf);
		}
	}

	if (old_sf) obs_source_release(old_sf);
	if (new_sf) obs_source_release(new_sf);

	bool sf_changed = (old_sf != new_sf);
	if (sf_changed) {
		f->bbox.valid = false;
		f->settings.cached_bbox_valid = false;
		obs_data_set_bool(settings, "bbox_valid", false);
	} else {
		f->bbox.min_x = f->settings.cached_bbox_min_x;
		f->bbox.min_y = f->settings.cached_bbox_min_y;
		f->bbox.max_x = f->settings.cached_bbox_max_x;
		f->bbox.max_y = f->settings.cached_bbox_max_y;
		f->bbox.valid = f->settings.cached_bbox_valid;
	}

	f->needs_recalc = true;
}

static obs_properties_t *filter_get_properties(void *data)
{
	AutoSFFilter *f = static_cast<AutoSFFilter *>(data);
	FilterSettings s;
	if (f) s = f->settings;
	obs_properties_t *p = settings_properties(&s);
	obs_properties_add_button(p, "recalc_now",
				  obs_module_text("RecalculateNow"),
				  [](obs_properties_t *, obs_property_t *,
				     void *d) {
					  auto *fl = static_cast<AutoSFFilter *>(d);
					  if (fl) {
						  do_recalc(fl, true);
						  fl->needs_recalc = !fl->bbox.valid;
					  }
					  return true;
				  });

	std::string info_text = "Make with ❤️ by <img src=\"data:image/png;base64,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\" width=\"14\" height=\"14\"> <a href=\"https://github.com/visual-alchemy/obs-ads-insertion\">visual-alchemy</a>";

	obs_properties_add_text(p, "credit_info", info_text.c_str(), OBS_TEXT_INFO);

	return p;
}

static void filter_defaults(obs_data_t *s) { settings_defaults(s); }

static void filter_activate(void *data)
{
	AutoSFFilter *f = static_cast<AutoSFFilter *>(data);
	f->needs_recalc = true;
	f->media_has_reset = false;
	f->startup_frames_logged = 0;
	f->activate_time = obs_get_video_frame_time();
	if (f->settings.enable_timing) {
		f->phase = TimingPhase::IN;
		f->phase_start_time = 0;
	}

	if (f->sf_source_ref) {
		obs_source_t *sf = obs_weak_source_get_source(f->sf_source_ref);
		if (sf) {
			obs_source_inc_showing(sf);
			obs_source_inc_active(sf);
			obs_source_media_restart(sf);
			obs_source_release(sf);
		}
	}
}

static void filter_deactivate(void *data)
{
	AutoSFFilter *f = static_cast<AutoSFFilter *>(data);
	f->phase = TimingPhase::IDLE;

	if (f->sf_source_ref) {
		obs_source_t *sf = obs_weak_source_get_source(f->sf_source_ref);
		if (sf) {
			obs_source_dec_showing(sf);
			obs_source_dec_active(sf);
			obs_source_release(sf);
		}
	}
}

static bool do_recalc(AutoSFFilter *f, bool force)
{
	if (!force && f->settings.enable_timing && f->phase != TimingPhase::HOLD && f->bbox.valid) {
		return false;
	}

	if (!f->sf_source_ref) resolve_sf(f);
	if (!f->sf_source_ref) return false;

	obs_source_t *sf = obs_weak_source_get_source(f->sf_source_ref);
	if (!sf) return false;

	obs_source_frame *frame = obs_source_get_frame(sf);
	if (!frame || !frame->data[0]) {
		obs_source_release(sf);
		return false;
	}

	int w = (int)frame->width, h = (int)frame->height;
	BoundingBox bb = alpha_analyze(frame->data[0], w, h,
				       (int)frame->linesize[0],
				       f->settings.alpha_threshold);
	obs_source_release_frame(sf, frame);
	obs_source_release(sf);

	if (!bb.valid) return true;
	f->bbox = bb;

	// Save the detected bounding box to the settings so it persists and loads on activation
	obs_data_t *settings = obs_source_get_settings(f->context);
	if (settings) {
		obs_data_set_int(settings, "bbox_min_x", f->bbox.min_x);
		obs_data_set_int(settings, "bbox_min_y", f->bbox.min_y);
		obs_data_set_int(settings, "bbox_max_x", f->bbox.max_x);
		obs_data_set_int(settings, "bbox_max_y", f->bbox.max_y);
		obs_data_set_bool(settings, "bbox_valid", true);
		obs_data_release(settings);
	}

	obs_source_t *p = obs_filter_get_parent(f->context);
	uint32_t pw = p ? obs_source_get_base_width(p) : 0;
	uint32_t ph = p ? obs_source_get_base_height(p) : 0;
	if (!pw) pw = (uint32_t)w;
	if (!ph) ph = (uint32_t)h;

	f->transform = transform_calculate((int)pw, (int)ph,
					   f->bbox.min_x, f->bbox.min_y,
					   f->bbox.width(), f->bbox.height(),
					   f->settings.fit_mode,
					   f->settings.padding);
	return true;
}

static float lin(float a, float b, float t) { return a + (b - a) * t; }

static float ease_out(float t)
{
	t = std::max(0.0f, std::min(1.0f, t));
	float v = 1.0f - t;
	return 1.0f - v * v * v;
}

static float ease_in(float t)
{
	t = std::max(0.0f, std::min(1.0f, t));
	return t * t * t;
}

static void advance_phase(AutoSFFilter *f)
{
	if (!f->settings.enable_timing || f->phase == TimingPhase::IDLE) return;
	if (f->phase_start_time == 0) return;

	uint64_t now = obs_get_video_frame_time();
	float t = (float)(now - f->phase_start_time) / 1000000.0f;
	TimingPhase old_phase = f->phase;

	switch (f->phase) {
	case TimingPhase::IN:
		if (t >= (float)f->settings.in_duration_ms) {
			f->phase = TimingPhase::HOLD;
			f->phase_start_time = now;
		}
		break;
	case TimingPhase::HOLD:
		if (t >= (float)f->settings.hold_duration_ms) {
			f->phase = TimingPhase::OUT;
			f->phase_start_time = now;
		}
		break;
	case TimingPhase::OUT:
		if (t >= (float)f->settings.out_duration_ms) {
			f->phase = TimingPhase::IDLE;
			f->phase_start_time = 0;
		}
		break;
	default: break;
	}

	if (f->phase != old_phase) {
		blog(LOG_INFO, "[AutoSFFit] Phase transitioned from %d to %d at t=%.2f ms", (int)old_phase, (int)f->phase, t);
	}
}

static void get_transform(AutoSFFilter *f, float cx, float cy, 
                          int64_t media_time, int64_t duration,
                          float &ox, float &oy, float &sx, float &sy, 
                          float &sf_ox, float &sf_oy, float &sf_sx, float &sf_sy,
                          float &sf_opacity)
{
	sx = sy = 1.0f;
	ox = oy = 0.0f;
	sf_ox = sf_oy = 0.0f;
	sf_sx = sf_sy = 1.0f;
	sf_opacity = 1.0f;

	if (!f->bbox.valid) {
		sf_opacity = 0.0f;
		return;
	}

	if (!f->settings.enable_timing) {
		sx = f->transform.scale_x;
		sy = f->transform.scale_y;
		ox = f->transform.offset_x;
		oy = f->transform.offset_y;
		return;
	}

	float tx = f->transform.scale_x, ty = f->transform.scale_y;
	float tox = f->transform.offset_x, toy = f->transform.offset_y;

	float S_start_x = (tx > 0.001f) ? (1.0f / tx) : 1.0f;
	float S_start_y = (ty > 0.001f) ? (1.0f / ty) : 1.0f;
	float O_start_x = -tox * S_start_x;
	float O_start_y = -toy * S_start_y;

	// Use media sync if valid media duration is present and we've reset
	if (duration > 0 && media_time >= 0 && f->media_has_reset) {
		float in_dur = (float)f->settings.in_duration_ms;
		float out_dur = (float)f->settings.out_duration_ms;
		float total_dur = (float)duration;

		if (in_dur + out_dur > total_dur) {
			float sum = in_dur + out_dur;
			if (sum > 0.0f) {
				float scale = total_dur / sum;
				in_dur *= scale;
				out_dur *= scale;
			} else {
				in_dur = 0.0f;
				out_dur = 0.0f;
			}
		}

		float r = 0.0f;
		bool is_out = false;
		TimingPhase current_phase = TimingPhase::IDLE;

		if ((float)media_time <= in_dur) {
			current_phase = TimingPhase::IN;
			float t = (in_dur > 0.0f) ? ((float)media_time / in_dur) : 1.0f;
			r = ease_out(t);
			is_out = false;
			sf_opacity = 1.0f;
		} else if ((float)media_time <= total_dur - out_dur) {
			current_phase = TimingPhase::HOLD;
			r = 1.0f;
			is_out = false;
			sf_opacity = 1.0f;
		} else if ((float)media_time < total_dur) {
			current_phase = TimingPhase::OUT;
			float elapsed = (float)media_time - (total_dur - out_dur);
			float t = (out_dur > 0.0f) ? (elapsed / out_dur) : 1.0f;
			r = ease_in(t);
			is_out = true;
			sf_opacity = 1.0f;
		} else {
			current_phase = TimingPhase::IDLE;
			r = 1.0f;
			is_out = true;
			sf_opacity = 0.0f;
		}

		if (current_phase != f->phase) {
			blog(LOG_INFO, "[AutoSFFit] Media-sync Phase transitioned from %d to %d at media_time=%lld ms (dur=%lld)", (int)f->phase, (int)current_phase, media_time, duration);
			f->phase = current_phase;
		}

		if (current_phase == TimingPhase::HOLD) {
			sx = tx; sy = ty; ox = tox; oy = toy;
			sf_ox = 0.0f; sf_oy = 0.0f;
			sf_sx = 1.0f; sf_sy = 1.0f;
		} else {
			float S_x = is_out ? lin(1.0f, S_start_x, r) : lin(S_start_x, 1.0f, r);
			float S_y = is_out ? lin(1.0f, S_start_y, r) : lin(S_start_y, 1.0f, r);
			float O_x = is_out ? lin(0.0f, O_start_x, r) : lin(O_start_x, 0.0f, r);
			float O_y = is_out ? lin(0.0f, O_start_y, r) : lin(O_start_y, 0.0f, r);

			sf_sx = S_x; sf_sy = S_y;
			sf_ox = O_x; sf_oy = O_y;
			sx = S_x * tx; sy = S_y * ty;
			ox = O_x + tox * S_x; oy = O_y + toy * S_y;
		}
		return;
	}

	// Fallback to system-clock sync
	if (f->phase == TimingPhase::IDLE || f->phase_start_time == 0) {
		sx = sy = 1.0f;
		ox = oy = 0.0f;
		sf_opacity = 0.0f;
		sf_sx = S_start_x;
		sf_sy = S_start_y;
		sf_ox = O_start_x;
		sf_oy = O_start_y;
		return;
	}

	advance_phase(f);
	if (f->phase == TimingPhase::IDLE) {
		sf_opacity = 0.0f;
		sf_sx = S_start_x;
		sf_sy = S_start_y;
		sf_ox = O_start_x;
		sf_oy = O_start_y;
		return;
	}

	float in = (float)f->settings.in_duration_ms;
	float out = (float)f->settings.out_duration_ms;

	uint64_t now = obs_get_video_frame_time();
	float el = (float)(now - f->phase_start_time) / 1000000.0f;

	float r = 0.0f;
	bool is_out = false;

	switch (f->phase) {
	case TimingPhase::IN:
		r = (in > 0) ? ease_out(el / in) : 1.0f;
		is_out = false;
		sf_opacity = 1.0f;
		break;
	case TimingPhase::HOLD:
		r = 1.0f;
		is_out = false;
		sf_opacity = 1.0f;
		break;
	case TimingPhase::OUT:
		r = (out > 0) ? ease_in(el / out) : 1.0f;
		is_out = true;
		sf_opacity = 1.0f;
		break;
	default: break;
	}

	if (f->phase == TimingPhase::HOLD) {
		sx = tx; sy = ty; ox = tox; oy = toy;
		sf_ox = 0.0f; sf_oy = 0.0f;
		sf_sx = 1.0f; sf_sy = 1.0f;
	} else {
		float S_x = is_out ? lin(1.0f, S_start_x, r) : lin(S_start_x, 1.0f, r);
		float S_y = is_out ? lin(1.0f, S_start_y, r) : lin(S_start_y, 1.0f, r);
		float O_x = is_out ? lin(0.0f, O_start_x, r) : lin(O_start_x, 0.0f, r);
		float O_y = is_out ? lin(0.0f, O_start_y, r) : lin(O_start_y, 0.0f, r);

		sf_sx = S_x; sf_sy = S_y;
		sf_ox = O_x; sf_oy = O_y;
		sx = S_x * tx; sy = S_y * ty;
		ox = O_x + tox * S_x; oy = O_y + toy * S_y;
	}
}

static void filter_video_render(void *data, gs_effect_t *effect)
{
	AutoSFFilter *f = static_cast<AutoSFFilter *>(data);
	if (f->in_render) return;
	RecursionGuard recursion_guard(f->in_render);

	obs_source_t *parent = obs_filter_get_parent(f->context);
	if (!parent) return;

	uint32_t cx = obs_source_get_base_width(parent);
	uint32_t cy = obs_source_get_base_height(parent);
	if (!cx || !cy) return;

	if (f->bbox.valid) {
		f->transform = transform_calculate((int)cx, (int)cy,
						   f->bbox.min_x, f->bbox.min_y,
						   f->bbox.width(), f->bbox.height(),
						   f->settings.fit_mode,
						   f->settings.padding);
	}

	if (f->needs_recalc ||
	    (f->settings.recalc_mode == RecalcMode::PERIODIC &&
	     obs_get_video_frame_time() - f->last_recalc_time >=
		     (uint64_t)(f->settings.recalc_interval_ms * 1000000))) {
		bool should_recalc = true;
		if (f->settings.enable_timing && f->phase != TimingPhase::HOLD && f->bbox.valid) {
			should_recalc = false;
		}
		if (should_recalc) {
			if (do_recalc(f)) {
				f->needs_recalc = !f->bbox.valid;
				f->last_recalc_time = obs_get_video_frame_time();
			}
		}
	}

	// Check if SF source exists
	obs_source_t *sf = f->sf_source_ref ? obs_weak_source_get_source(f->sf_source_ref) : nullptr;

	int64_t media_time = -1;
	int64_t duration = -1;
	bool is_media = false;

	if (sf) {
		uint32_t flags = obs_source_get_output_flags(sf);
		if (flags & OBS_SOURCE_CONTROLLABLE_MEDIA) {
			is_media = true;
			duration = obs_source_media_get_duration(sf);
			if (duration > 0) {
				media_time = obs_source_media_get_time(sf);
			}
		}
	}

	uint64_t now = obs_get_video_frame_time();

	if (f->settings.enable_timing) {
		if (sf && is_media) {
			if (!f->media_has_reset) {
				if (media_time >= 0 && media_time < 200) {
					f->media_has_reset = true;
				} else {
					uint64_t diff = now - f->activate_time;
					if (diff > 1000000000ULL) { // 1.0s in ns
						f->media_has_reset = true;
						blog(LOG_WARNING, "[AutoSFFit] Media reset detection timed out. Forcing reset.");
					}
				}
			}
		} else {
			f->media_has_reset = true;
		}

		if (f->media_has_reset && f->phase_start_time == 0) {
			f->phase_start_time = now;
		}
	}



	if (!obs_source_process_filter_begin(f->context, GS_RGBA, OBS_NO_DIRECT_RENDERING)) {
		if (sf) obs_source_release(sf);
		return;
	}

	float sx, sy, ox, oy;
	float sf_ox = 0.0f;
	float sf_oy = 0.0f;
	float sf_sx = 1.0f;
	float sf_sy = 1.0f;
	float sf_opacity = 0.0f;
	get_transform(f, (float)cx, (float)cy, media_time, duration, ox, oy, sx, sy, sf_ox, sf_oy, sf_sx, sf_sy, sf_opacity);

	if (f->settings.enable_timing && f->startup_frames_logged < 120) {
		uint32_t sf_w = sf ? obs_source_get_base_width(sf) : 0;
		uint32_t sf_h = sf ? obs_source_get_base_height(sf) : 0;
		blog(LOG_INFO, "[AutoSFFit] Frame %d: sf_w=%u, sf_h=%u, dur=%lld, media_time=%lld, has_reset=%d, phase=%d, sx=%.3f, sy=%.3f, ox=%.3f, oy=%.3f, sf_ox=%.3f, sf_oy=%.3f, sf_sx=%.3f, sf_sy=%.3f, sf_opacity=%.3f",
			f->startup_frames_logged, sf_w, sf_h, duration, media_time, (int)f->media_has_reset, (int)f->phase, sx, sy, ox, oy, sf_ox, sf_oy, sf_sx, sf_sy, sf_opacity);
		f->startup_frames_logged++;
	}

	gs_texture_t *sf_tex = nullptr;

	if (sf && sf != parent && sf != f->context) {
		if (f->settings.render_sf_overlay) {
			uint32_t sf_w = obs_source_get_base_width(sf);
			uint32_t sf_h = obs_source_get_base_height(sf);
			if (sf_w > 0 && sf_h > 0 && f->texrender) {

				gs_texrender_reset(f->texrender);
				if (gs_texrender_begin(f->texrender, sf_w, sf_h)) {
					struct vec4 clear_color;
					vec4_zero(&clear_color);
					gs_clear(GS_CLEAR_COLOR, &clear_color, 0.0f, 0);

					obs_source_video_render(sf);

					gs_texrender_end(f->texrender);

					sf_tex = gs_texrender_get_texture(f->texrender);
				}
			}
		}
		obs_source_release(sf);
	}

	if (!sf_tex) {
		sf_opacity = 0.0f;
		sf_ox = 0.0f;
		sf_oy = 0.0f;
		sf_sx = 1.0f;
		sf_sy = 1.0f;
	}

	if (f->combined_effect) {
		gs_eparam_t *param_sf_image = gs_effect_get_param_by_name(f->combined_effect, "sf_image");
		gs_eparam_t *param_scale = gs_effect_get_param_by_name(f->combined_effect, "scale");
		gs_eparam_t *param_offset = gs_effect_get_param_by_name(f->combined_effect, "offset");
		gs_eparam_t *param_sf_scale = gs_effect_get_param_by_name(f->combined_effect, "sf_scale");
		gs_eparam_t *param_sf_offset = gs_effect_get_param_by_name(f->combined_effect, "sf_offset");
		gs_eparam_t *param_sf_opacity = gs_effect_get_param_by_name(f->combined_effect, "sf_opacity");

		struct vec2 v_scale;
		v_scale.x = std::max(sx, 0.001f);
		v_scale.y = std::max(sy, 0.001f);

		struct vec2 v_offset;
		v_offset.x = (cx > 0) ? (ox / (float)cx) : 0.0f;
		v_offset.y = (cy > 0) ? (oy / (float)cy) : 0.0f;

		struct vec2 v_sf_scale;
		v_sf_scale.x = std::max(sf_sx, 0.001f);
		v_sf_scale.y = std::max(sf_sy, 0.001f);

		struct vec2 v_sf_offset;
		v_sf_offset.x = (cx > 0) ? (sf_ox / (float)cx) : 0.0f;
		v_sf_offset.y = (cy > 0) ? (sf_oy / (float)cy) : 0.0f;

		gs_effect_set_vec2(param_scale, &v_scale);
		gs_effect_set_vec2(param_offset, &v_offset);
		gs_effect_set_vec2(param_sf_scale, &v_sf_scale);
		gs_effect_set_vec2(param_sf_offset, &v_sf_offset);
		gs_effect_set_float(param_sf_opacity, sf_opacity);
		if (sf_tex) {
			gs_effect_set_texture(param_sf_image, sf_tex);
		} else {
			gs_effect_set_texture(param_sf_image, nullptr);
		}
	}

	obs_source_process_filter_end(f->context, f->combined_effect, cx, cy);

	if (f->settings.debug_enabled) {
		debug_overlay_draw((int)cx, (int)cy, f->bbox, sx, sy, ox, oy, f->bbox.valid);
	}
}

void register_auto_sf_fit_filter()
{
	obs_source_info i = {};
	i.id = FILTER_ID;
	i.type = OBS_SOURCE_TYPE_FILTER;
	i.output_flags = OBS_SOURCE_VIDEO;
	i.get_name = filter_get_name;
	i.create = filter_create;
	i.destroy = filter_destroy;
	i.update = filter_update;
	i.get_properties = filter_get_properties;
	i.get_defaults = filter_defaults;
	i.activate = filter_activate;
	i.deactivate = filter_deactivate;
	i.video_render = filter_video_render;
	obs_register_source(&i);
}
