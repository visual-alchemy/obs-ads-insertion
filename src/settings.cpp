#include "settings.hpp"

void settings_load(FilterSettings *s, obs_data_t *data)
{
	s->sf_source_name = obs_data_get_string(data, "sf_source");
	s->fit_mode = static_cast<FitMode>(obs_data_get_int(data, "fit_mode"));
	s->alpha_threshold = (int)obs_data_get_int(data, "alpha_threshold");
	s->padding = (int)obs_data_get_int(data, "padding");
	s->recalc_mode = static_cast<RecalcMode>(obs_data_get_int(data, "recalc_mode"));
	s->recalc_interval_ms = (int)obs_data_get_int(data, "recalc_interval_ms");
	s->debug_enabled = obs_data_get_bool(data, "debug_enabled");
	s->enable_timing = obs_data_get_bool(data, "enable_timing");
	s->in_duration_ms = (int)obs_data_get_int(data, "in_duration_ms");
	s->hold_duration_ms = (int)obs_data_get_int(data, "hold_duration_ms");
	s->out_duration_ms = (int)obs_data_get_int(data, "out_duration_ms");
	s->render_sf_overlay = obs_data_get_bool(data, "render_sf_overlay");
	s->preview_squeezed = obs_data_get_bool(data, "preview_squeezed");
	s->sf_anim_mode = static_cast<SFAnimMode>(obs_data_get_int(data, "sf_anim_mode"));
	s->slide_dir = static_cast<SlideDir>(obs_data_get_int(data, "slide_dir"));

	s->cached_bbox_min_x = (int)obs_data_get_int(data, "bbox_min_x");
	s->cached_bbox_min_y = (int)obs_data_get_int(data, "bbox_min_y");
	s->cached_bbox_max_x = (int)obs_data_get_int(data, "bbox_max_x");
	s->cached_bbox_max_y = (int)obs_data_get_int(data, "bbox_max_y");
	s->cached_bbox_valid = obs_data_get_bool(data, "bbox_valid");
}

void settings_save(FilterSettings *s, obs_data_t *data)
{
	obs_data_set_string(data, "sf_source", s->sf_source_name.c_str());
	obs_data_set_int(data, "fit_mode", static_cast<long long>(s->fit_mode));
	obs_data_set_int(data, "alpha_threshold", s->alpha_threshold);
	obs_data_set_int(data, "padding", s->padding);
	obs_data_set_int(data, "recalc_mode", static_cast<long long>(s->recalc_mode));
	obs_data_set_int(data, "recalc_interval_ms", s->recalc_interval_ms);
	obs_data_set_bool(data, "debug_enabled", s->debug_enabled);
	obs_data_set_bool(data, "enable_timing", s->enable_timing);
	obs_data_set_int(data, "in_duration_ms", s->in_duration_ms);
	obs_data_set_int(data, "hold_duration_ms", s->hold_duration_ms);
	obs_data_set_int(data, "out_duration_ms", s->out_duration_ms);
	obs_data_set_bool(data, "render_sf_overlay", s->render_sf_overlay);
	obs_data_set_bool(data, "preview_squeezed", s->preview_squeezed);
	obs_data_set_int(data, "sf_anim_mode", static_cast<long long>(s->sf_anim_mode));
	obs_data_set_int(data, "slide_dir", static_cast<long long>(s->slide_dir));

	obs_data_set_int(data, "bbox_min_x", s->cached_bbox_min_x);
	obs_data_set_int(data, "bbox_min_y", s->cached_bbox_min_y);
	obs_data_set_int(data, "bbox_max_x", s->cached_bbox_max_x);
	obs_data_set_int(data, "bbox_max_y", s->cached_bbox_max_y);
	obs_data_set_bool(data, "bbox_valid", s->cached_bbox_valid);
}

void settings_defaults(obs_data_t *data)
{
	obs_data_set_default_string(data, "sf_source", "");
	obs_data_set_default_int(data, "fit_mode", static_cast<long long>(FitMode::CONTAIN));
	obs_data_set_default_int(data, "alpha_threshold", 16);
	obs_data_set_default_int(data, "padding", 0);
	obs_data_set_default_int(data, "recalc_mode",
				 static_cast<long long>(RecalcMode::ON_ACTIVATE));
	obs_data_set_default_int(data, "recalc_interval_ms", 1000);
	obs_data_set_default_bool(data, "debug_enabled", false);
	obs_data_set_default_bool(data, "enable_timing", false);
	obs_data_set_default_int(data, "in_duration_ms", 1000);
	obs_data_set_default_int(data, "hold_duration_ms", 8000);
	obs_data_set_default_int(data, "out_duration_ms", 1000);
	obs_data_set_default_bool(data, "render_sf_overlay", true);
	obs_data_set_default_bool(data, "preview_squeezed", false);
	obs_data_set_default_int(data, "sf_anim_mode", static_cast<long long>(SFAnimMode::SCALE_SLIDE));
	obs_data_set_default_int(data, "slide_dir", static_cast<long long>(SlideDir::LEFT));

	obs_data_set_default_int(data, "bbox_min_x", 0);
	obs_data_set_default_int(data, "bbox_min_y", 0);
	obs_data_set_default_int(data, "bbox_max_x", 0);
	obs_data_set_default_int(data, "bbox_max_y", 0);
	obs_data_set_default_bool(data, "bbox_valid", false);
}

static bool prop_source_filter(void *data, obs_source_t *source)
{
	const char *id = obs_source_get_id(source);
	const char *name = obs_source_get_name(source);
	if (!id || !name)
		return true;

	obs_property_t *p = static_cast<obs_property_t *>(data);
	obs_property_list_add_string(p, name, name);
	return true;
}

obs_properties_t *settings_properties(FilterSettings *s)
{
	obs_properties_t *props = obs_properties_create();

	obs_property_t *p = obs_properties_add_list(props, "sf_source",
						    obs_module_text("SfSource"),
						    OBS_COMBO_TYPE_LIST,
						    OBS_COMBO_FORMAT_STRING);
	obs_enum_sources(prop_source_filter, p);

	obs_property_t *fit = obs_properties_add_list(props, "fit_mode",
						       obs_module_text("FitMode"),
						       OBS_COMBO_TYPE_LIST,
						       OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(fit, obs_module_text("FitMode.Contain"),
				  static_cast<long long>(FitMode::CONTAIN));
	obs_property_list_add_int(fit, obs_module_text("FitMode.Cover"),
				  static_cast<long long>(FitMode::COVER));
	obs_property_list_add_int(fit, obs_module_text("FitMode.Stretch"),
				  static_cast<long long>(FitMode::STRETCH));

	obs_properties_add_int_slider(props, "alpha_threshold",
				      obs_module_text("AlphaThreshold"), 0, 255, 1);

	obs_properties_add_int_slider(props, "padding", obs_module_text("Padding"), 0,
				      200, 1);

	obs_property_t *recalc = obs_properties_add_list(props, "recalc_mode",
							 obs_module_text("RecalcMode"),
							 OBS_COMBO_TYPE_LIST,
							 OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(recalc, obs_module_text("RecalcMode.Manual"),
				  static_cast<long long>(RecalcMode::MANUAL));
	obs_property_list_add_int(recalc, obs_module_text("RecalcMode.OnActivate"),
				  static_cast<long long>(RecalcMode::ON_ACTIVATE));
	obs_property_list_add_int(recalc, obs_module_text("RecalcMode.Periodic"),
				  static_cast<long long>(RecalcMode::PERIODIC));

	obs_properties_add_int(props, "recalc_interval_ms",
			       obs_module_text("RecalcIntervalMs"), 100, 60000, 100);

	obs_properties_add_bool(props, "enable_timing", obs_module_text("EnableTiming"));

	obs_properties_add_int(props, "in_duration_ms", obs_module_text("InDurationMs"),
			       0, 10000, 100);
	obs_properties_add_int(props, "hold_duration_ms",
			       obs_module_text("HoldDurationMs"), 0, 60000, 100);
	obs_properties_add_int(props, "out_duration_ms",
			       obs_module_text("OutDurationMs"), 0, 10000, 100);

	obs_property_t *anim_mode = obs_properties_add_list(props, "sf_anim_mode",
							     obs_module_text("SFAnimMode"),
							     OBS_COMBO_TYPE_LIST,
							     OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(anim_mode, obs_module_text("SFAnimMode.Static"),
				  static_cast<long long>(SFAnimMode::STATIC));
	obs_property_list_add_int(anim_mode, obs_module_text("SFAnimMode.Slide"),
				  static_cast<long long>(SFAnimMode::SLIDE));
	obs_property_list_add_int(anim_mode, obs_module_text("SFAnimMode.ScaleSlide"),
				  static_cast<long long>(SFAnimMode::SCALE_SLIDE));

	obs_property_t *slide_dir = obs_properties_add_list(props, "slide_dir",
							     obs_module_text("SlideDir"),
							     OBS_COMBO_TYPE_LIST,
							     OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(slide_dir, obs_module_text("SlideDir.Left"),
				  static_cast<long long>(SlideDir::LEFT));
	obs_property_list_add_int(slide_dir, obs_module_text("SlideDir.Right"),
				  static_cast<long long>(SlideDir::RIGHT));
	obs_property_list_add_int(slide_dir, obs_module_text("SlideDir.Top"),
				  static_cast<long long>(SlideDir::TOP));
	obs_property_list_add_int(slide_dir, obs_module_text("SlideDir.Bottom"),
				  static_cast<long long>(SlideDir::BOTTOM));

	obs_property_set_modified_callback(anim_mode, [](obs_properties_t *props, obs_property_t *, obs_data_t *data) {
		SFAnimMode mode = static_cast<SFAnimMode>(obs_data_get_int(data, "sf_anim_mode"));
		obs_property_t *p_slide_dir = obs_properties_get(props, "slide_dir");
		if (p_slide_dir) {
			obs_property_set_visible(p_slide_dir, mode == SFAnimMode::SLIDE);
		}
		return true;
	});

	if (s) {
		obs_property_t *p_slide_dir = obs_properties_get(props, "slide_dir");
		if (p_slide_dir) {
			obs_property_set_visible(p_slide_dir, s->sf_anim_mode == SFAnimMode::SLIDE);
		}
	}

	obs_properties_add_bool(props, "debug_enabled",
				obs_module_text("DebugEnabled"));

	obs_properties_add_bool(props, "render_sf_overlay",
				obs_module_text("RenderSfOverlay"));
	obs_properties_add_bool(props, "preview_squeezed",
				obs_module_text("PreviewSqueezed"));

	return props;
}

