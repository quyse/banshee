#ifndef ___BANSHEE_GAME_HPP___
#define ___BANSHEE_GAME_HPP___

#include "general.hpp"
#include <unordered_set>

class Geometry;
class GeometryFormats;
struct Material;
class Skeleton;
class BoneAnimation;
class BoneAnimationFrame;
class Painter;

struct StaticLight : public Object
{
	vec3 position;
	vec3 target;
	float angle;
	float nearPlane, farPlane;
	vec3 color;
	bool shadow;
	mat4x4 transform;

	StaticLight();

	void UpdateTransform();

	//*** Методы для скрипта.
	void SetPosition(const vec3& position);
	void SetTarget(const vec3& target);
	void SetProjection(float angle, float nearPlane, float farPlane);
	void SetColor(const vec3& color);
	void SetShadow(bool shadow);

	META_DECLARE_CLASS(StaticLight);
};

/// Класс игры.
class Game : public Object
{
private:
	ptr<Platform::Window> window;
	ptr<Device> device;
	ptr<Context> context;
	ptr<Presenter> presenter;

	ptr<GeometryFormats> geometryFormats;

	ptr<Painter> painter;

	ptr<FileSystem> fileSystem;

	ptr<Input::Manager> inputManager;

	ptr<TextureManager> textureManager;
	ptr<Gui::GrCanvas> canvas;
	ptr<Gui::Font> font;

	vec3 cameraPosition;
	float alpha;

	ptr<Material> decalMaterial;

	struct StaticModel
	{
		ptr<Geometry> geometry;
		ptr<Material> material;
		mat4x4 transform;
	};
	std::vector<StaticModel> staticModels;

	struct RigidModel
	{
		ptr<Geometry> geometry;
		ptr<Material> material;
		ptr<Physics::RigidBody> rigidBody;
	};
	std::vector<RigidModel> rigidModels;

	std::vector<ptr<Physics::RigidBody> > staticRigidBodies;

	std::vector<ptr<StaticLight> > staticLights;

	Ticker ticker;

	float cameraAlpha, cameraBeta;

	ptr<Physics::World> physicsWorld;
	struct Cube
	{
		ptr<Physics::RigidBody> rigidBody;
		vec3 scale;
		Cube(ptr<Physics::RigidBody> rigidBody, const vec3& scale = vec3(1, 1, 1))
		: rigidBody(rigidBody), scale(scale) {}
	};
	std::vector<Cube> cubes;

	struct BansheeParams
	{
		ptr<Geometry> mainGeometry;
		ptr<Geometry> leftWingGeometry;
		ptr<Geometry> rightWingGeometry;
		ptr<Geometry> rotor1Geometry;
		ptr<Geometry> rotor2Geometry;
		ptr<Material> material;
		ptr<Physics::Shape> shape;
		float mass;
		float rotorSpeed;
		float minRotorForce, maxRotorForce;
		float rotorForceChangeRate;
		float rotorPitchChangeRate;
		float rotorPitchControlMin;
		float rotorPitchControlMax;
		float rotorRollControlBound;
		float rotorPitchControlCoef;
		float rotorRollControlCoef;
	} bansheeParams;

	class Banshee;

	ptr<Banshee> hero;
	std::vector<ptr<Banshee> > enemies;
	typedef std::unordered_set<Banshee*> Banshees;
	Banshees banshees;

	vec3 ambientColor;

	float bloomLimit, toneLuminanceKey, toneMaxLuminance;

	/// Скрипт.
	ptr<Script::State> scriptState;
	/// Единственный экземпляр для игры.
	static Game* singleGame;

public:
	Game();
	~Game();

	void Run();
	void Tick();

	//******* Методы, доступные из скрипта.

	static ptr<Game> Get();

	ptr<Texture> LoadTexture(const String& fileName);
	ptr<Geometry> LoadGeometry(const String& fileName);
	ptr<Geometry> LoadSkinnedGeometry(const String& fileName);
	ptr<Skeleton> LoadSkeleton(const String& fileName);
	ptr<BoneAnimation> LoadBoneAnimation(const String& fileName, ptr<Skeleton> skeleton);
	ptr<Physics::Shape> CreatePhysicsBoxShape(const vec3& halfSize);
	ptr<Physics::Shape> CreatePhysicsSphereShape(float radius);
	ptr<Physics::RigidBody> CreatePhysicsRigidBody(ptr<Physics::Shape> physicsShape, float mass, const vec3& position);
	void AddStaticModel(ptr<Geometry> geometry, ptr<Material> material, const vec3& position);
	void AddRigidModel(ptr<Geometry> geometry, ptr<Material> material, ptr<Physics::RigidBody> physicsRigidBody);
	void AddStaticRigidBody(ptr<Physics::RigidBody> rigidBody);
	ptr<StaticLight> AddStaticLight();

	void SetAmbient(const vec3& color);

	void SetBansheeParams(
		ptr<Geometry> mainGeometry,
		ptr<Geometry> leftWingGeometry,
		ptr<Geometry> rightWingGeometry,
		ptr<Geometry> rotor1Geometry,
		ptr<Geometry> rotor2Geometry,
		ptr<Material> material,
		ptr<Physics::Shape> shape,
		float mass,
		float rotorSpeed,
		float minRotorForce,
		float maxRotorForce,
		float rotorForceChangeRate,
		float rotorPitchChangeRate,
		float rotorPitchControlMin,
		float rotorPitchControlMax,
		float rotorRollControlBound,
		float rotorPitchControlCoef,
		float rotorRollControlCoef
	);
	ptr<Banshee> CreateBanshee(const mat4x4& initialTransform);

	void PlaceHero(const vec3& position);
	void PlaceCamera(const vec3& position, float alpha, float beta);

	META_DECLARE_CLASS(Game);
};

#endif
