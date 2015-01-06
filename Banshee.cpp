#include <iostream>
#include <iomanip>
#include <sstream>

#include "Banshee.hpp"
#include "Painter.hpp"
#include "Geometry.hpp"
#include "GeometryFormats.hpp"
#include "Material.hpp"


const float pi = 3.1415926535897932f;

std::string Game::Banshee::bansheeDebug;

const mat4x4 Game::Banshee::mainTransform =
	CreateScalingMatrix(vec3(0.02f, 0.02f, 0.02f)) *
	QuaternionToMatrix(axis_rotation(vec3(0, 0, 1), pi));

const mat4x4 Game::Banshee::leftRotorTransform =
	CreateTranslationMatrix(vec3(96.0f * 0.02f, 0, 12.5f * 0.02f)) *
	QuaternionToMatrix(axis_rotation(vec3(0, 1, 0), -pi * 10.0f / 180.0f)) *
	CreateScalingMatrix(vec3(0.02f, 0.02f, 0.02f)) *
	QuaternionToMatrix(axis_rotation(vec3(0, 0, 1), pi));

const mat4x4 Game::Banshee::rightRotorTransform =
	CreateTranslationMatrix(vec3(-96.0f * 0.02f, 0, 12.5f * 0.02f)) *
	QuaternionToMatrix(axis_rotation(vec3(0, 1, 0), pi * 10.0f / 180.0f)) *
	CreateScalingMatrix(vec3(0.02f, 0.02f, 0.02f)) *
	QuaternionToMatrix(axis_rotation(vec3(0, 0, 1), pi));

// const mat4x4 Game::Banshee::mainTransform =
// 	CreateTranslationMatrix(vec3(0, + 01.f * 0.02f, + 50.f * 0.02f)) *
// 	CreateScalingMatrix(vec3(0.02f, 0.02f, 0.02f)) *
// 	QuaternionToMatrix(axis_rotation(vec3(0, 0, 1), pi));
// const mat4x4 Game::Banshee::leftRotorTransform =
// 	CreateTranslationMatrix(vec3(96.0f * 0.02f, + 01.f * 0.02f, (12.5f + 50.f)* 0.02f)) *
// 	QuaternionToMatrix(axis_rotation(vec3(0, 1, 0), -pi * 10.0f / 180.0f)) *
// 	CreateScalingMatrix(vec3(0.02f, 0.02f, 0.02f)) *
// 	QuaternionToMatrix(axis_rotation(vec3(0, 0, 1), pi));
// const mat4x4 Game::Banshee::rightRotorTransform =
// 	CreateTranslationMatrix(vec3(-96.0f * 0.02f, + 01.f * 0.02f, (12.5f + 50.f)* 0.02f)) *
// 	QuaternionToMatrix(axis_rotation(vec3(0, 1, 0), pi * 10.0f / 180.0f)) *
// 	CreateScalingMatrix(vec3(0.02f, 0.02f, 0.02f)) *
// 	QuaternionToMatrix(axis_rotation(vec3(0, 0, 1), pi));


Game::Banshee::Banshee(Game* game, const mat4x4& initialTransform) :
	game(game),
	rotorForce(game->bansheeParams.minRotorForce),
	leftRotorAngle(0),
	rightRotorAngle(0),
	controlPitch(0),
	controlRoll(0)
{
	rigidBody = game->physicsWorld->CreateRigidBody(game->bansheeParams.shape, game->bansheeParams.mass, initialTransform);
	rigidBody->DisableDeactivation();
	rigidBody.StaticCast<Physics::BtRigidBody>()->GetInternalObject()->setDamping(game->bansheeParams.linearDamping, game->bansheeParams.angularDamping);
	game->banshees.insert(this);
}


Game::Banshee::~Banshee()
{
	game->banshees.erase(this);
}


void Game::Banshee::stepRotorForce(const BansheeStepParams &step_params)
{
	rotorForce = relax(
		rotorForce,
		step_params.desired_force,
		game->bansheeParams.rotorForceChangeRate * step_params.frame_time);
	rotorForce = clamp(
		rotorForce,
		game->bansheeParams.minRotorForce,
		game->bansheeParams.maxRotorForce);	
}

void Game::Banshee::stepControls(const BansheeStepParams &step_params)
{
	const float left_pitch = getLeftPitch(controlPitch, controlRoll);
	const float right_pitch = getRightPitch(controlPitch, controlRoll);

	const float desired_control_pitch = clamp(
		controlPitch + step_params.pitch_delta,
		game->bansheeParams.rotorPitchControlMin,
		game->bansheeParams.rotorPitchControlMax);

	const float desired_control_roll = clamp(
		controlRoll + step_params.roll_delta,
		-game->bansheeParams.rotorRollControlBound,
		game->bansheeParams.rotorRollControlBound);

	const float desired_left_pitch =  getLeftPitch(desired_control_pitch, desired_control_roll);
	const float desired_right_pitch =  getRightPitch(desired_control_pitch, desired_control_roll);

	const float max_pitch_change = step_params.frame_time * game->bansheeParams.rotorPitchChangeRate;

	const float apply_coef = std::min<float>(1.f,
		std::min<float>(
			max_pitch_change / fabs(desired_left_pitch - left_pitch),
			max_pitch_change / fabs(desired_right_pitch - right_pitch)
			)
		);

	controlPitch += (desired_control_pitch - controlPitch) * apply_coef;
	controlRoll += (desired_control_roll - controlRoll) * apply_coef;
}


void Game::Banshee::Step(const BansheeStepParams step_params)
{
	stepRotorForce(step_params);

	stepControls(step_params);


	const float left_pitch = getLeftPitch(controlPitch, controlRoll);
	const float right_pitch = getRightPitch(controlPitch, controlRoll);

	mat4x4 transform = GetTransform();

	leftActualRotorTransform = transform * leftRotorTransform * QuaternionToMatrix(axis_rotation(vec3(1, 0, 0), left_pitch));
	vec4 leftRotorDir = leftActualRotorTransform * vec4(0, 0, 1, 0);
	leftRotorDir = normalize(leftRotorDir);

	rightActualRotorTransform = transform * rightRotorTransform * QuaternionToMatrix(axis_rotation(vec3(1, 0, 0), right_pitch));
	vec4 rightRotorDir = rightActualRotorTransform * vec4(0, 0, 1, 0);
	rightRotorDir = normalize(rightRotorDir);


	const float left_rotor_force = rotorForce;
	const float right_rotor_force = rotorForce;

	vec3 leftForce = vec3(leftRotorDir.x, leftRotorDir.y, leftRotorDir.z) * left_rotor_force;
	leftForcePoint = vec3(leftActualRotorTransform(0, 3), leftActualRotorTransform(1, 3), leftActualRotorTransform(2, 3));

	vec3 rightForce = vec3(rightRotorDir.x, rightRotorDir.y, rightRotorDir.z) * right_rotor_force;
	rightForcePoint = vec3(rightActualRotorTransform(0, 3), rightActualRotorTransform(1, 3), rightActualRotorTransform(2, 3));

	rigidBody->ApplyForce(leftForce, leftForcePoint);
	rigidBody->ApplyForce(rightForce, rightForcePoint);

	std::ostringstream s;
	s << std::fixed << std::setprecision(3)
		<< "left_pitch: " << left_pitch << " "
		<< "right_pitch: " << right_pitch << " "
		<< "(" << leftRotorDir << ") "
		<< "(" << rightRotorDir << ") "
		<< "(" << leftForce << ") "
		<< "(" << rightForce << ")";
	bansheeDebug = s.str();


	leftRotorAngle += left_rotor_force * game->bansheeParams.rotorSpeed * step_params.frame_time;
	rightRotorAngle += right_rotor_force * game->bansheeParams.rotorSpeed * step_params.frame_time;
	while(leftRotorAngle > pi * 2)
		leftRotorAngle -= pi * 2;
	while(rightRotorAngle > pi * 2)
		rightRotorAngle -= pi * 2;
}


void Game::Banshee::Paint(Painter* painter)
{
	mat4x4 transform = GetTransform();
	painter->AddModel(game->bansheeParams.material, game->bansheeParams.mainGeometry, transform * mainTransform);

	// left wing
	const float left_pitch = getLeftPitch(controlPitch, controlRoll);

	painter->AddModel(game->bansheeParams.material, game->bansheeParams.leftWingGeometry,
		transform *
		leftRotorTransform *
		QuaternionToMatrix(axis_rotation(vec3(1, 0, 0), left_pitch)));
	painter->AddModel(game->bansheeParams.material, game->bansheeParams.rotor1Geometry,
		transform *
		leftRotorTransform *
		QuaternionToMatrix(axis_rotation(vec3(1, 0, 0), left_pitch)) *
		QuaternionToMatrix(axis_rotation(vec3(0, 0, 1), leftRotorAngle)));
	painter->AddModel(game->bansheeParams.material, game->bansheeParams.rotor2Geometry,
		transform *
		leftRotorTransform *
		QuaternionToMatrix(axis_rotation(vec3(1, 0, 0), left_pitch)) *
		QuaternionToMatrix(axis_rotation(vec3(0, 0, 1), -leftRotorAngle)));

	// right wing
	const float right_pitch = getRightPitch(controlPitch, controlRoll);

	painter->AddModel(game->bansheeParams.material, game->bansheeParams.rightWingGeometry,
		transform *
		rightRotorTransform *
		QuaternionToMatrix(axis_rotation(vec3(1, 0, 0), right_pitch)));
	painter->AddModel(game->bansheeParams.material, game->bansheeParams.rotor1Geometry,
		transform *
		rightRotorTransform *
		QuaternionToMatrix(axis_rotation(vec3(1, 0, 0), right_pitch)) *
		QuaternionToMatrix(axis_rotation(vec3(0, 0, 1), rightRotorAngle)));
	painter->AddModel(game->bansheeParams.material, game->bansheeParams.rotor2Geometry,
		transform *
		rightRotorTransform *
		QuaternionToMatrix(axis_rotation(vec3(1, 0, 0), right_pitch)) *
		QuaternionToMatrix(axis_rotation(vec3(0, 0, 1), -rightRotorAngle)));

	// TEST: look
	painter->AddModel(game->bansheeParams.material, game->bansheeParams.mainGeometry,
		transform *
		CreateTranslationMatrix(game->bansheeParams.lookOffset) *
		CreateTranslationMatrix(vec3(0.0f, 50.0f, 0.0f)) *
		CreateScalingMatrix(vec3(0.1f, 100.0f, 0.1f)) *
		mainTransform);

	// TEST: left force
	painter->AddModel(game->bansheeParams.material, game->bansheeParams.mainGeometry,
		leftActualRotorTransform *
		CreateTranslationMatrix(vec3(0.0f, 0.0f, 60.0f)) *
		CreateScalingMatrix(vec3(0.5f, 0.5f, 50.0f)) *
		mainTransform);
	// TEST: right force
	painter->AddModel(game->bansheeParams.material, game->bansheeParams.mainGeometry,
		rightActualRotorTransform *
		CreateTranslationMatrix(vec3(0.0f, 0.0f, 60.0f)) *
		CreateScalingMatrix(vec3(0.5f, 0.5f, 50.0f)) *
		mainTransform);

	// debug cube
	painter->AddTransparentModel(game->debugMaterial, game->cubeGeometry, transform * CreateScalingMatrix(game->bansheeParams.cubeSize));
}