#pragma once
#include <obs-module.h>
#include <string>

enum class FitMode {
	CONTAIN,
	COVER,
	STRETCH
};

enum class RecalcMode {
	MANUAL,
	ON_ACTIVATE,
	PERIODIC
};

struct FilterSettings {
	std::string sf_source_name;
	FitMode fit_mode;
	int alpha_threshold;
	int padding;
	RecalcMode recalc_mode;
	int recalc_interval_ms;
	bool debug_enabled;
	bool enable_timing;
	int in_duration_ms;
	int hold_duration_ms;
	int out_duration_ms;
	bool render_sf_overlay;

	int cached_bbox_min_x;
	int cached_bbox_min_y;
	int cached_bbox_max_x;
	int cached_bbox_max_y;
	bool cached_bbox_valid;

	FilterSettings()
		: fit_mode(FitMode::CONTAIN),
		  alpha_threshold(16),
		  padding(0),
		  recalc_mode(RecalcMode::ON_ACTIVATE),
		  recalc_interval_ms(1000),
		  debug_enabled(false),
		  enable_timing(false),
		  in_duration_ms(1000),
		  hold_duration_ms(8000),
		  out_duration_ms(1000),
		  render_sf_overlay(true),
		  cached_bbox_min_x(0),
		  cached_bbox_min_y(0),
		  cached_bbox_max_x(0),
		  cached_bbox_max_y(0),
		  cached_bbox_valid(false)
	{
	}
};

void settings_load(FilterSettings *s, obs_data_t *data);
void settings_save(FilterSettings *s, obs_data_t *data);
void settings_defaults(obs_data_t *data);
obs_properties_t *settings_properties(FilterSettings *s);
