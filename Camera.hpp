#ifndef ___BANSHEE_CAMERA_HPP___
#define ___BANSHEE_CAMERA_HPP___

#include "general.hpp"

class Camera : public Object
{
public:

	Camera(
		const mat4x4 start_position,
		const float half_life,
		const float back_range);

	void newTick(
		const mat4x4 new_position,
		const float time,
		const float x_view_angle,
		const float y_view_angle);

	vec3 getCameraPosition() const;

	mat4x4 getViewMat() const;

private:

	// camera position and view matrix
	vec3 _position;
	mat4x4 _view_mat;


	// smothing parameter
	// (in seconds)
	const float _half_life;

	const float _back_range;

	// smoothed banshee transform params
	vec3 _smooth_banshee_position;
	vec3 _smooth_banshee_y;
	vec3 _smooth_banshee_z;

	// computes _position and _view_mat
	void compute(
		const vec3 target,
		const float x_view_direction,
		const float y_view_direction);

	static const float infinity;

	META_DECLARE_CLASS(Camera);
};

#endif  // ___BANSHEE_CAMERA_HPP___