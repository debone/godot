/**************************************************************************/
/*  point_transform_gizmo_2d.h                                            */
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

#pragma once

#include "core/math/transform_2d.h"
#include "core/math/vector2.h"
#include "core/templates/vector.h"

class Control;

// Editor-agnostic helpers for transforming a multi-point selection with
// Move/Rotate/Scale gizmos. Shared by the 2D point editors (polygon, path, ...)
// so they reproduce the CanvasItemEditor gizmo look and pivot/local-space math
// without depending on its per-CanvasItem state.
namespace PointTransformGizmo2D {

// Which gizmo to draw / how to interact. Maps from CanvasItemEditor::get_current_tool().
enum class Mode {
	NONE,
	MOVE,
	ROTATE,
	SCALE,
};

// Which part of the gizmo a press landed on.
enum class HitType {
	NONE,
	AXIS_X, // Constrain to the gizmo's local X axis.
	AXIS_Y, // Constrain to the gizmo's local Y axis.
	PLANE, // Free move / uniform handle (gizmo body).
};

// Average of a set of points (their geometric mean). Returns zero for an empty set.
Vector2 selection_center(const Vector<Vector2> &p_points);

// Screen-space rect (already normalized) vs. screen-space point test, for box selection.
bool rect_contains_point(const Rect2 &p_screen_rect, const Vector2 &p_screen_point);

// Draw the Move/Rotate/Scale gizmo on the canvas overlay.
// p_pivot_screen is the pivot in overlay (screen) pixels; p_basis_rotation is the
// on-screen rotation of the gizmo axes (0 for global space, the node's screen
// rotation for local space). p_active_hit highlights the grabbed axis while dragging.
// p_scale_preview offsets the scale handles to preview the current drag.
void draw_gizmo(Control *p_overlay, Mode p_mode, const Vector2 &p_pivot_screen, real_t p_basis_rotation, HitType p_active_hit = HitType::NONE, const Vector2 &p_scale_preview = Vector2());

// Hit-test the gizmo handles. p_mouse_screen and p_pivot_screen are in overlay pixels.
// For MOVE/SCALE this distinguishes axis handles from the body (PLANE). For ROTATE
// any press returns PLANE (rotation is driven from the angle around the pivot).
HitType hit_test(Mode p_mode, const Vector2 &p_pivot_screen, real_t p_basis_rotation, const Vector2 &p_mouse_screen);

// Apply a transform to a single point about a pivot, all in the same (local) space.
// MOVE adds p_translate; ROTATE rotates about the pivot by p_rotate; SCALE scales the
// offset from the pivot by p_scale (component-wise).
Vector2 apply_transform(const Vector2 &p_point, const Vector2 &p_pivot, Mode p_mode, const Vector2 &p_translate, real_t p_rotate, const Vector2 &p_scale);

// Blender-style numeric entry for a transform: accumulates typed digits, an optional
// decimal point, a sign, and an axis constraint, so a group transform can be given an
// exact value (e.g. rotate by typing "45", scale by "2", move "10" on X).
struct NumericInput {
	bool active = false;
	double value = 0.0;
	int next_decimal = 0; // 0 = integer part, negative = fractional digit position
	bool negate = false;
	int axis = -1; // -1 = none/uniform, 0 = X, 1 = Y

	void reset();
	void add_digit(uint32_t p_digit);
	void start_decimal();
	bool has_input() const { return value != 0.0 || next_decimal != 0; }
	double signed_value() const { return negate ? -value : value; }
};

} //namespace PointTransformGizmo2D
