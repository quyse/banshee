#include "Camera.hpp"

const float Camera::infinity = 1e9;

Camera::Camera(
	const mat4x4 start_position,
	const float half_life,
	const float back_range) :
_half_life(half_life),
_back_range(back_range)
{
	const vec3 banshee_position(
		start_position(0, 3),
		start_position(1, 3),
		start_position(2, 3));

	const vec3 banshee_y(
		start_position(0, 1),
		start_position(1, 1),
		start_position(2, 1));

	const vec3 banshee_z(
		start_position(0, 2),
		start_position(1, 2),
		start_position(2, 2));

	_view_mat = CreateLookAtMatrix(
		_position,
		_position + banshee_y,
		banshee_z);

	_smooth_banshee_position = banshee_position;
	_smooth_banshee_y = banshee_y;
	_smooth_banshee_z = banshee_z;

	compute(
		banshee_position + banshee_y * infinity,
		0,
		0);
}


void Camera::compute(
	const vec3 target,
	const float x_view_angle,
	const float y_view_angle)
{
	const vec3 y = _smooth_banshee_y;
	const vec3 z = _smooth_banshee_z;
	const vec3 x = cross(y, z);

	// camera position offset in banshee coord system
	const vec3 offset(
		sin(x_view_angle),
		-cos(x_view_angle) * cos(y_view_angle),
		cos(x_view_angle) * sin(y_view_angle) );

	// set camera position
	_position =
		_smooth_banshee_position +
		x * offset.x * _back_range +
		y * offset.y * _back_range +
		z * offset.z * _back_range;

	_view_mat = CreateLookAtMatrix(_position, target, z);
}


vec3 Camera::getCameraPosition() const
{
	return _position;
}

mat4x4 Camera::getViewMat() const
{
	return _view_mat;
}


void Camera::newTick(
	const mat4x4 new_position,
	const float time,
	const float x_view_angle,
	const float y_view_angle)
{
	const vec3 banshee_position(
		new_position(0, 3),
		new_position(1, 3),
		new_position(2, 3));

	const vec3 banshee_y(
		new_position(0, 1),
		new_position(1, 1),
		new_position(2, 1));

	const vec3 banshee_z(
		new_position(0, 2),
		new_position(1, 2),
		new_position(2, 2));

	#if 0  // no smoothing
	_smooth_banshee_position = banshee_position;
	_smooth_banshee_y = banshee_y;
	_smooth_banshee_z = banshee_z;

	#else  // some smothing

	const float last_coef = exp(time * log(0.5f) / _half_life);
	const float new_coef = 1.f - last_coef;

	_smooth_banshee_position =
		_smooth_banshee_position * last_coef +
		banshee_position * new_coef;

	_smooth_banshee_z =
		_smooth_banshee_z * last_coef +
		banshee_z * new_coef;
	_smooth_banshee_z = normalize(_smooth_banshee_z);

	_smooth_banshee_y =
		_smooth_banshee_y * last_coef +
		banshee_y * new_coef;

	const vec3 smooth_x = cross(_smooth_banshee_y, _smooth_banshee_z);

	_smooth_banshee_y = normalize(cross(_smooth_banshee_z, smooth_x));
	#endif

	compute(
		banshee_position + banshee_y * infinity,
		x_view_angle,
		y_view_angle);
}

