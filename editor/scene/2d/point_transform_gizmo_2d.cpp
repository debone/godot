/**************************************************************************/
/*  point_transform_gizmo_2d.cpp                                          */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "point_transform_gizmo_2d.h"

#include "editor/editor_string_names.h"
#include "editor/themes/editor_scale.h"
#include "scene/gui/control.h"

// Pixel distance from the pivot to the tip of the move/scale gizmo arms. Matches
// MOVE_HANDLE_DISTANCE / SCALE_HANDLE_DISTANCE in canvas_item_editor_plugin.cpp.
static constexpr real_t HANDLE_DISTANCE = 25;

namespace PointTransformGizmo2D {

Vector2 selection_center(const Vector<Vector2> &p_points) {
	const int n = p_points.size();
	if (n == 0) {
		return Vector2();
	}
	Vector2 sum;
	for (int i = 0; i < n; i++) {
		sum += p_points[i];
	}
	return sum / n;
}

bool rect_contains_point(const Rect2 &p_screen_rect, const Vector2 &p_screen_point) {
	return p_screen_rect.has_point(p_screen_point);
}

void draw_gizmo(Control *p_overlay, Mode p_mode, const Vector2 &p_pivot_screen, real_t p_basis_rotation, HitType p_active_hit, const Vector2 &p_scale_preview) {
	if (p_mode == Mode::NONE) {
		return;
	}

	const Color axis_x_color = p_overlay->get_theme_color(SNAME("axis_x_color"), EditorStringName(Editor));
	const Color axis_y_color = p_overlay->get_theme_color(SNAME("axis_y_color"), EditorStringName(Editor));
	const Color accent = p_overlay->get_theme_color(SNAME("accent_color"), EditorStringName(Editor));

	// Maps gizmo-local pixels (where +X/+Y are the gizmo axes) to overlay pixels.
	const Transform2D gizmo_xform = Transform2D(p_basis_rotation, p_pivot_screen);

	const Color x_col = (p_active_hit == HitType::AXIS_X) ? accent : axis_x_color;
	const Color y_col = (p_active_hit == HitType::AXIS_Y) ? accent : axis_y_color;

	switch (p_mode) {
		case Mode::MOVE: {
			const real_t arm = HANDLE_DISTANCE * EDSCALE;

			// X arrow.
			Vector<Vector2> x_head = {
				gizmo_xform.xform(Vector2(arm, 5 * EDSCALE)),
				gizmo_xform.xform(Vector2(arm, -5 * EDSCALE)),
				gizmo_xform.xform(Vector2(arm + 10 * EDSCALE, 0)),
			};
			p_overlay->draw_line(gizmo_xform.xform(Vector2()), gizmo_xform.xform(Vector2(arm, 0)), x_col, Math::round(EDSCALE));
			p_overlay->draw_colored_polygon(x_head, x_col);

			// Y arrow.
			Vector<Vector2> y_head = {
				gizmo_xform.xform(Vector2(5 * EDSCALE, arm)),
				gizmo_xform.xform(Vector2(-5 * EDSCALE, arm)),
				gizmo_xform.xform(Vector2(0, arm + 10 * EDSCALE)),
			};
			p_overlay->draw_line(gizmo_xform.xform(Vector2()), gizmo_xform.xform(Vector2(0, arm)), y_col, Math::round(EDSCALE));
			p_overlay->draw_colored_polygon(y_head, y_col);
		} break;

		case Mode::SCALE: {
			const real_t arm_x = (HANDLE_DISTANCE + p_scale_preview.x) * EDSCALE;
			const real_t arm_y = (HANDLE_DISTANCE + p_scale_preview.y) * EDSCALE;

			p_overlay->draw_line(gizmo_xform.xform(Vector2()), gizmo_xform.xform(Vector2(arm_x, 0)), x_col, Math::round(EDSCALE));
			Vector<Vector2> x_rect = {
				gizmo_xform.xform(Vector2(arm_x, -5 * EDSCALE)),
				gizmo_xform.xform(Vector2(arm_x + 10 * EDSCALE, -5 * EDSCALE)),
				gizmo_xform.xform(Vector2(arm_x + 10 * EDSCALE, 5 * EDSCALE)),
				gizmo_xform.xform(Vector2(arm_x, 5 * EDSCALE)),
			};
			p_overlay->draw_colored_polygon(x_rect, x_col);

			p_overlay->draw_line(gizmo_xform.xform(Vector2()), gizmo_xform.xform(Vector2(0, arm_y)), y_col, Math::round(EDSCALE));
			Vector<Vector2> y_rect = {
				gizmo_xform.xform(Vector2(-5 * EDSCALE, arm_y)),
				gizmo_xform.xform(Vector2(5 * EDSCALE, arm_y)),
				gizmo_xform.xform(Vector2(5 * EDSCALE, arm_y + 10 * EDSCALE)),
				gizmo_xform.xform(Vector2(-5 * EDSCALE, arm_y + 10 * EDSCALE)),
			};
			p_overlay->draw_colored_polygon(y_rect, y_col);
		} break;

		case Mode::ROTATE: {
			// A ring centered on the pivot; the drag itself is angle-driven.
			const real_t radius = HANDLE_DISTANCE * EDSCALE;
			const Color ring_col = (p_active_hit == HitType::PLANE) ? accent : accent * Color(1, 1, 1, 0.6);
			p_overlay->draw_arc(p_pivot_screen, radius, 0, Math::TAU, 32, ring_col, Math::round(2 * EDSCALE));
		} break;

		case Mode::NONE:
			break;
	}
}

HitType hit_test(Mode p_mode, const Vector2 &p_pivot_screen, real_t p_basis_rotation, const Vector2 &p_mouse_screen) {
	if (p_mode == Mode::NONE) {
		return HitType::NONE;
	}

	// Mouse position in gizmo-local pixels.
	const Transform2D gizmo_xform = Transform2D(p_basis_rotation, p_pivot_screen);
	const Vector2 local = gizmo_xform.affine_inverse().xform(p_mouse_screen);

	switch (p_mode) {
		case Mode::MOVE:
		case Mode::SCALE: {
			const real_t arm = HANDLE_DISTANCE * EDSCALE;
			const Rect2 x_handle_rect = Rect2(arm, -5 * EDSCALE, 15 * EDSCALE, 10 * EDSCALE);
			const Rect2 y_handle_rect = Rect2(-5 * EDSCALE, arm, 10 * EDSCALE, 15 * EDSCALE);
			if (x_handle_rect.has_point(local)) {
				return HitType::AXIS_X;
			}
			if (y_handle_rect.has_point(local)) {
				return HitType::AXIS_Y;
			}
			// For Move, grabbing near the pivot does a free (planar) move. Scale needs an axis
			// handle (a near-pivot scale grab would divide by a near-zero distance).
			if (p_mode == Mode::MOVE && local.length() < HANDLE_DISTANCE * EDSCALE) {
				return HitType::PLANE;
			}
			return HitType::NONE;
		} break;

		case Mode::ROTATE: {
			// Only grab when near the ring, so clicks elsewhere can still select points.
			const real_t radius = HANDLE_DISTANCE * EDSCALE;
			if (Math::abs(local.length() - radius) < 8 * EDSCALE) {
				return HitType::PLANE;
			}
			return HitType::NONE;
		} break;

		case Mode::NONE:
			break;
	}
	return HitType::NONE;
}

Vector2 apply_transform(const Vector2 &p_point, const Vector2 &p_pivot, Mode p_mode, const Vector2 &p_translate, real_t p_rotate, const Vector2 &p_scale) {
	switch (p_mode) {
		case Mode::MOVE:
			return p_point + p_translate;
		case Mode::ROTATE:
			return p_pivot + (p_point - p_pivot).rotated(p_rotate);
		case Mode::SCALE:
			return p_pivot + (p_point - p_pivot) * p_scale;
		case Mode::NONE:
			break;
	}
	return p_point;
}

void NumericInput::reset() {
	active = false;
	value = 0.0;
	next_decimal = 0;
	negate = false;
	axis = -1;
}

void NumericInput::add_digit(uint32_t p_digit) {
	if (next_decimal < 0) {
		value = value + p_digit * Math::pow(10.0, (double)next_decimal--);
	} else {
		value = value * 10 + p_digit;
	}
}

void NumericInput::start_decimal() {
	if (next_decimal == 0) {
		next_decimal = -1;
	}
}

} //namespace PointTransformGizmo2D
