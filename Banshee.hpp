#ifndef ___BANSHEE_BANSHEE_HPP___
#define ___BANSHEE_BANSHEE_HPP___

#include "general.hpp"
#include "Game.hpp"

class Game::Banshee : public Object
{

public:
	static std::string bansheeDebug;

	Banshee(Game* game, const mat4x4& initialTransform);

	~Banshee();

	inline mat4x4 GetTransform() const
	{
		return rigidBody->GetTransform();
	}

	static inline float relax(float current, float target, float maxStep)
	{
		if(target > current)
			return std::min(current + maxStep, target);
		else
			return std::max(current - maxStep, target);
	}

	struct BansheeStepParams
	{
		float frame_time;
		float pitch_delta;
		float roll_delta;
		float desired_force;

		BansheeStepParams() :
			frame_time(0),
			pitch_delta(0),
			roll_delta(0),
			desired_force(0)
		{
			// nothing
		}
	};

	void Step(const BansheeStepParams step_params);

	void Paint(Painter* painter);

	inline float getControlPitch() const
	{
		return controlPitch;
	}

	inline float getControlRoll() const
	{
		return controlRoll;
	}

private:
	Game* game;
	ptr<Physics::RigidBody> rigidBody;

	float rotorForce;

	// Rotation angles of rotors.
	float leftRotorAngle, rightRotorAngle;

	// Control for rotors.
	float controlPitch, controlRoll;

	vec3 leftForcePoint, rightForcePoint;  // just for debug draw
	mat4x4 leftActualRotorTransform, rightActualRotorTransform;  // just for debug draw

	void stepRotorForce(const BansheeStepParams&);
	void stepControls(const BansheeStepParams&);

	static inline float getLeftPitch(const float control_pitch, const float control_roll)
	{
		return control_pitch + control_roll;
	}

	static inline float getRightPitch(const float control_pitch, const float control_roll)
	{
		return control_pitch - control_roll;
	}

	static const mat4x4 mainTransform;
	static const mat4x4 leftRotorTransform;
	static const mat4x4 rightRotorTransform;
};


#endif  // ___BANSHEE_BANSHEE_HPP___