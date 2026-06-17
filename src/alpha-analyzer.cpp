#include "alpha-analyzer.hpp"
#include <algorithm>
#include <vector>

BoundingBox alpha_analyze(const uint8_t *rgba_data, int width, int height,
			  int linesize, int threshold)
{
	BoundingBox bbox;

	if (!rgba_data || width <= 0 || height <= 0)
		return bbox;

	// 1. Count transparent pixels in each column
	std::vector<int> trans_col(width, 0);
	for (int y = 0; y < height; y++) {
		const uint8_t *row = rgba_data + (size_t)y * linesize;
		for (int x = 0; x < width; x++) {
			uint8_t a = row[x * 4 + 3];
			if (a <= threshold) {
				trans_col[x]++;
			}
		}
	}

	// A column is part of the cutout if it has at least 10% of height as transparent pixels
	int min_trans_in_col = (int)(height * 0.1);
	std::vector<int> valid_cols;
	for (int x = 0; x < width; x++) {
		if (trans_col[x] >= min_trans_in_col) {
			valid_cols.push_back(x);
		}
	}

	// Find the largest contiguous interval of valid columns
	int min_x = width;
	int max_x = -1;
	if (!valid_cols.empty()) {
		int start = valid_cols[0];
		int prev = valid_cols[0];
		int best_start = 0;
		int best_end = -1;
		int best_len = 0;

		for (size_t i = 1; i < valid_cols.size(); i++) {
			int x = valid_cols[i];
			if (x == prev + 1) {
				prev = x;
			} else {
				int len = prev - start + 1;
				if (len > best_len) {
					best_len = len;
					best_start = start;
					best_end = prev;
				}
				start = x;
				prev = x;
			}
		}
		int len = prev - start + 1;
		if (len > best_len) {
			best_len = len;
			best_start = start;
			best_end = prev;
		}

		min_x = best_start;
		max_x = best_end;
	}

	if (max_x < 0) {
		return bbox;
	}

	// 2. Count transparent pixels in each row, but ONLY within the column range [min_x, max_x]
	int cutout_w = max_x - min_x + 1;
	std::vector<int> trans_row(height, 0);
	for (int y = 0; y < height; y++) {
		const uint8_t *row = rgba_data + (size_t)y * linesize;
		for (int x = min_x; x <= max_x; x++) {
			uint8_t a = row[x * 4 + 3];
			if (a <= threshold) {
				trans_row[y]++;
			}
		}
	}

	// A row is part of the cutout if at least 50% of the columns in that row are transparent
	int min_trans_in_row = (int)(cutout_w * 0.5);
	std::vector<int> valid_rows;
	for (int y = 0; y < height; y++) {
		if (trans_row[y] >= min_trans_in_row) {
			valid_rows.push_back(y);
		}
	}

	// Find the largest contiguous interval of valid rows
	int min_y = height;
	int max_y = -1;
	if (!valid_rows.empty()) {
		int start = valid_rows[0];
		int prev = valid_rows[0];
		int best_start = 0;
		int best_end = -1;
		int best_len = 0;

		for (size_t i = 1; i < valid_rows.size(); i++) {
			int y = valid_rows[i];
			if (y == prev + 1) {
				prev = y;
			} else {
				int len = prev - start + 1;
				if (len > best_len) {
					best_len = len;
					best_start = start;
					best_end = prev;
				}
				start = y;
				prev = y;
			}
		}
		int len = prev - start + 1;
		if (len > best_len) {
			best_len = len;
			best_start = start;
			best_end = prev;
		}

		min_y = best_start;
		max_y = best_end;
	}

	if (max_y < 0) {
		return bbox;
	}

	bbox.min_x = min_x;
	bbox.min_y = min_y;
	bbox.max_x = max_x + 1;
	bbox.max_y = max_y + 1;
	bbox.valid = true;

	return bbox;
}
