#pragma once
#include <obs-module.h>
#include "settings.hpp"
#include "alpha-analyzer.hpp"
#include "transform-engine.hpp"

enum class TimingPhase { IDLE, IN, HOLD, OUT };

struct AutoSFFilter {
	obs_source_t *context;
	FilterSettings settings;
	obs_weak_source_t *sf_source_ref;
	BoundingBox bbox;
	TransformResult transform;
	uint64_t last_recalc_time;
	bool needs_recalc;
	TimingPhase phase;
	uint64_t phase_start_time;
	uint64_t activate_time;
	gs_texrender_t *texrender;
	gs_effect_t *combined_effect;
	bool in_render;
	bool media_has_reset;
	int startup_frames_logged;
	bool calibrated_this_hold;

	AutoSFFilter()
		: context(nullptr), sf_source_ref(nullptr),
		  last_recalc_time(0), needs_recalc(true),
		  phase(TimingPhase::IDLE), phase_start_time(0),
		  activate_time(0),
		  texrender(nullptr), combined_effect(nullptr),
		  in_render(false), media_has_reset(false),
		  startup_frames_logged(0), calibrated_this_hold(false) {}


	~AutoSFFilter() {
		if (sf_source_ref) obs_weak_source_release(sf_source_ref);
		if (texrender || combined_effect) {
			obs_enter_graphics();
			if (texrender) gs_texrender_destroy(texrender);
			if (combined_effect) gs_effect_destroy(combined_effect);
			obs_leave_graphics();
		}
	}
};

void register_auto_sf_fit_filter();
