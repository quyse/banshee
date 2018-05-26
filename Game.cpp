#include "Game.hpp"
#include "Painter.hpp"
#include "Geometry.hpp"
#include "GeometryFormats.hpp"
#include "Material.hpp"
#include "Camera.hpp"
#include "Skeleton.hpp"
#include "BoneAnimation.hpp"
#include "Banshee.hpp"
#include "../inanity/script/lua/State.hpp"
#ifndef ___INANITY_PLATFORM_EMSCRIPTEN
#include "../inanity/inanity-sqlitefs.hpp"
#endif
#include <iostream>
#include <iomanip>
#include <sstream>

Game* Game::singleGame = 0;

const float pi = 3.1415926535897932f;
const float gravity = -9.8f;

Game::Game() :
	bloomLimit(10.0f), toneLuminanceKey(0.12f), toneMaxLuminance(3.1f)
{
	singleGame = this;
}

Game::~Game()
{
	hero = nullptr;
	enemies.clear();
}

void Game::Run()
{
	try
	{
		ptr<Graphics::System> system = Inanity::Platform::Game::CreateDefaultGraphicsSystem();

		ptr<Graphics::Adapter> adapter = system->GetAdapters()[0];
		device = system->CreateDevice(adapter);
		ptr<Graphics::Monitor> monitor = adapter->GetMonitors()[0];

		int screenWidth = 800;
		int screenHeight = 600;
		bool fullscreen = false;

		ptr<Platform::Window> window = monitor->CreateDefaultWindow(
			"Banshee Project", screenWidth, screenHeight);
		this->window = window;

		inputManager = Inanity::Platform::Game::CreateInputManager(window);

		ptr<Graphics::MonitorMode> monitorMode;
		if(fullscreen)
			monitorMode = monitor->TryCreateMode(screenWidth, screenHeight);
		presenter = device->CreateWindowPresenter(window, monitorMode);

		context = system->CreateContext(device);

#ifdef ___INANITY_PLATFORM_EMSCRIPTEN
		ptr<FileSystem> shaderCacheFileSystem = NEW(Data::TempFileSystem());
#else
		const char* shadersCacheFileName =
#ifdef _DEBUG
			"shaders_debug"
#else
			"shaders"
#endif
			;
		ptr<FileSystem> shaderCacheFileSystem = NEW(Data::SQLiteFileSystem(shadersCacheFileName));
#endif
			;

		ptr<ShaderCache> shaderCache = NEW(ShaderCache(shaderCacheFileSystem, device,
			device->CreateShaderCompiler(), device->CreateShaderGenerator(), NEW(Crypto::WhirlpoolStream())));

		fileSystem =
#ifdef PRODUCTION
			NEW(Data::BlobFileSystem(Platform::FileSystem::GetNativeFileSystem()->LoadFile("data")))
#else
			NEW(Data::BufferedFileSystem(NEW(Platform::FileSystem("assets"))))
#endif
		;

		geometryFormats = NEW(GeometryFormats());

		painter = NEW(Painter(device, context, presenter, shaderCache, geometryFormats));

		{
			SamplerSettings samplerSettings;
			samplerSettings.SetFilter(SamplerSettings::filterLinear);
			samplerSettings.SetWrap(SamplerSettings::wrapRepeat);
			textureManager = NEW(TextureManager(fileSystem, device, samplerSettings));
		}

		// GUI canvas and fonts
		canvas = Gui::GrCanvas::Create(device, shaderCache);
		{
			ptr<Gui::FontEngine> fontEngine = NEW(Gui::FtEngine());
			ptr<Gui::FontFace> fontFace = fontEngine->LoadFontFace(fileSystem->LoadFile("/DejaVuSans.ttf"));
			const int fontSize = 13;
			ptr<Gui::FontShape> fontShape = fontFace->CreateShape(fontSize);
			ptr<Gui::FontGlyphs> fontGlyphs = fontFace->CreateGlyphs(canvas, fontSize, {});
			font = NEW(Gui::Font(fontShape, fontGlyphs));
		}

		physicsWorld = NEW(Physics::BtWorld());

		// запустить стартовый скрипт
		ptr<Script::Lua::State> luaState = NEW(Script::Lua::State());
		luaState->Register<Game>();
		luaState->Register<Material>();
		scriptState = luaState;

		ptr<Script::Function> mainScript = scriptState->LoadScript(fileSystem->LoadFile(
#ifdef PRODUCTION
			"/main.luab"
#else
			"/main.lua"
#endif
		));
		mainScript->Run();

		window->SetMouseLock(true);
		window->SetCursorVisible(false);

		try
		{
			window->Run(Handler::Bind(MakePointer(this), &Game::Tick));

			scriptState = 0;
		}
		catch(Exception* exception)
		{
			scriptState = 0;
			THROW_SECONDARY("Error while running game", exception);
		}
	}
	catch(Exception* exception)
	{
		THROW_SECONDARY("Can't initialize game", exception);
	}
}

void Game::Tick()
{
	float frameTime = ticker.Tick();

	static bool cameraMode = false;

	Banshee::BansheeStepParams hero_step_params;
	hero_step_params.frame_time = frameTime;
	hero_step_params.desired_force = bansheeParams.normalRotorForce;

	ptr<Input::Frame> inputFrame = inputManager->GetCurrentFrame();
	while(inputFrame->NextEvent())
	{
		const Input::Event& inputEvent = inputFrame->GetCurrentEvent();

		//std::cout << inputEvent << "[" << inputFrame->GetCurrentState().cursorX << ' ' << inputFrame->GetCurrentState().cursorY << "] ";

		switch(inputEvent.device)
		{
		case Input::Event::deviceKeyboard:
			if(inputEvent.keyboard.type == Input::Event::Keyboard::typeKeyDown)
			{
				switch(inputEvent.keyboard.key)
				{
				case Input::Keys::Escape: // escape
					window->Close();
					return;
				case Input::Keys::Space:
					//physicsCharacter.FastCast<Physics::BtCharacter>()->GetInternalController()->jump();
					break;
#ifndef PRODUCTION
				case Input::Keys::M:
					try
					{
						scriptState->LoadScript(fileSystem->LoadFile("/console.lua"))->Run();
						std::cout << "console.lua successfully executed.\n";
					}
					catch(Exception* exception)
					{
						std::ostringstream s;
						MakePointer(exception)->PrintStack(s);
						std::cout << s.str() << '\n';
					}
					break;

				case Input::Keys::C:
					cameraMode = !cameraMode;
					break;

				case Input::Keys::_1:
					bloomLimit -= 0.1f;
					printf("bloomLimit: %f\n", bloomLimit);
					break;
				case Input::Keys::_2:
					bloomLimit += 0.1f;
					printf("bloomLimit: %f\n", bloomLimit);
					break;
				case Input::Keys::_3:
					toneLuminanceKey -= 0.01f;
					printf("toneLuminanceKey: %f\n", toneLuminanceKey);
					break;
				case Input::Keys::_4:
					toneLuminanceKey += 0.01f;
					printf("toneLuminanceKey: %f\n", toneLuminanceKey);
					break;
				case Input::Keys::_5:
					toneMaxLuminance -= 0.1f;
					printf("toneMaxLuminance: %f\n", toneMaxLuminance);
					break;
				case Input::Keys::_6:
					toneMaxLuminance += 0.1f;
					printf("toneMaxLuminance: %f\n", toneMaxLuminance);
					break;

				case Input::Keys::L:
					{
						static bool mouseLock = true;
						mouseLock = !mouseLock;
						window->SetMouseLock(mouseLock);
					}
					break;
				case Input::Keys::V:
					{
						static bool cursorVisible = false;
						cursorVisible = !cursorVisible;
						window->SetCursorVisible(cursorVisible);
					}
					break;
				default: break;
#endif
				}
			}
			break;
		case Input::Event::deviceMouse:
			switch(inputEvent.mouse.type)
			{
			case Input::Event::Mouse::typeButtonUp:
				break;
			case Input::Event::Mouse::typeRawMove:
				if(cameraMode)
				{
					const float maxAngleChange = frameTime * 50;
					cameraAlpha -= clamp(inputEvent.mouse.rawMoveX * 0.005f, -maxAngleChange, maxAngleChange);
					cameraBeta -= clamp(inputEvent.mouse.rawMoveY * 0.005f, -maxAngleChange, maxAngleChange);
					cameraAlpha -= clamp(inputEvent.mouse.rawMoveZ * 0.005f, -maxAngleChange, maxAngleChange);
				}
				else
				{
					hero_step_params.pitch_delta = inputEvent.mouse.rawMoveY * bansheeParams.rotorPitchControlCoef;
					hero_step_params.roll_delta = inputEvent.mouse.rawMoveX * bansheeParams.rotorRollControlCoef;
				}
				break;
			default: break;
			}
			break;
		default:
			break;
		}
	}


	vec3 cameraDirection = vec3(cos(cameraAlpha) * cos(cameraBeta), sin(cameraAlpha) * cos(cameraBeta), sin(cameraBeta));
	//vec3 cameraRightDirection = normalize(cross(cameraDirection, vec3(0, 0, 1)));
	//vec3 cameraUpDirection = cross(cameraRightDirection, cameraDirection);

	const Input::State& inputState = inputFrame->GetCurrentState();
	/*
	left up right down Q E
	37 38 39 40
	65 87 68 83 81 69
	*/
	float cameraStep = 5;
	vec3 cameraMove(0, 0, 0);
	vec3 cameraMoveDirectionFront(cos(cameraAlpha), sin(cameraAlpha), 0);
	vec3 cameraMoveDirectionUp(0, 0, 1);
	vec3 cameraMoveDirectionRight = cross(cameraMoveDirectionFront, cameraMoveDirectionUp);
	if(inputState.keyboard[37] || inputState.keyboard[65])
		cameraMove -= cameraMoveDirectionRight * cameraStep;
	if(inputState.keyboard[38] || inputState.keyboard[87])
	{
		cameraMove += cameraMoveDirectionFront * cameraStep;
		if(!cameraMode)
		{
			// controlMaxForce = true;
			hero_step_params.desired_force = bansheeParams.maxRotorForce;
		}
	}
	if(inputState.keyboard[39] || inputState.keyboard[68])
		cameraMove += cameraMoveDirectionRight * cameraStep;
	if(inputState.keyboard[40] || inputState.keyboard[83])
	{
		cameraMove -= cameraMoveDirectionFront * cameraStep;
		if(!cameraMode)
		{
			// controlMinForce = true;
			hero_step_params.desired_force = bansheeParams.minRotorForce;
		}
	}
	if(inputState.keyboard[81])
		cameraMove -= cameraMoveDirectionUp * cameraStep;
	if(inputState.keyboard[69])
		cameraMove += cameraMoveDirectionUp * cameraStep;


	physicsWorld->Simulate(frameTime);


	// if(controlMaxForce)
	// 	hero->SetControlMaxForce(true);
	// if(controlMinForce)
	// 	hero->SetControlMinForce(true);
	// hero->SetControlPitch(controlPitch);
	// hero->SetControlRoll(controlRoll);

	// for(Banshees::iterator i = banshees.begin(); i != banshees.end(); ++i)
	// 	(*i)->Step(frameTime);

	hero->Step(hero_step_params);


	mat4x4 viewMatrix;
	if(cameraMode)
	{
		cameraPosition += cameraMove * frameTime;
		viewMatrix = CreateLookAtMatrix(cameraPosition, cameraPosition + cameraDirection, vec3(0, 0, 1));
	}
	else
	{

		mat4x4 heroTransform = hero->GetTransform();

		if(!_camera)
		{
			_camera = NEW(Camera(heroTransform, 0.1f, 8.f));
		}

		const float a = hero->getControlRoll() / bansheeParams.rotorRollControlBound;
		const float b =
			(hero->getControlPitch() - bansheeParams.rotorPitchControlMin) /
			(bansheeParams.rotorPitchControlMax - bansheeParams.rotorPitchControlMin);

		_camera->newTick(
			heroTransform,
			frameTime,
			a * pi / 6,
			b * pi / 20 + pi/30);

		cameraPosition = _camera->getCameraPosition();
		viewMatrix = _camera->getViewMat();
	}

	alpha += frameTime;

	int screenWidth = presenter->GetWidth();
	int screenHeight = presenter->GetHeight();
	painter->Resize(screenWidth, screenHeight);

	mat4x4 projMatrix = CreateProjectionPerspectiveFovMatrix(pi / 4, float(screenWidth) / float(screenHeight), 0.1f, 10000.0f);

	// зарегистрировать все объекты
	painter->BeginFrame(frameTime);
	painter->SetCamera(projMatrix * viewMatrix, cameraPosition);
	painter->SetAmbientColor(ambientColor);

	for(size_t i = 0; i < staticModels.size(); ++i)
	{
		const StaticModel& model = staticModels[i];
		painter->AddModel(model.material, model.geometry, model.transform);
	}

	for(size_t i = 0; i < rigidModels.size(); ++i)
	{
		const RigidModel& model = rigidModels[i];
		painter->AddModel(model.material, model.geometry, model.rigidBody->GetTransform());
	}

	for(size_t i = 0; i < staticLights.size(); ++i)
	{
		ptr<StaticLight> light = staticLights[i];
		if(light->shadow)
			painter->AddShadowLight(light->position, light->color, light->transform);
		else
			painter->AddBasicLight(light->position, light->color);
	}

	for(Banshees::const_iterator i = banshees.begin(); i != banshees.end(); ++i)
		(*i)->Paint(painter);

	painter->SetupPostprocess(bloomLimit, toneLuminanceKey, toneMaxLuminance);

	painter->Draw();

	canvas->SetContext(context);

	// fps
	{
		Context::LetFrameBuffer lfb(context, presenter->GetFrameBuffer());
		Context::LetViewport lv(context, screenWidth, screenHeight);

		static int tickCount = 0;
		static const int needTickCount = 100;
		static float allTicksTime = 0;
		allTicksTime += frameTime;
		static float lastAllTicksTime = 0;
		if(++tickCount >= needTickCount)
		{
			lastAllTicksTime = allTicksTime;
			allTicksTime = 0;
			tickCount = 0;
		}
		char fpsString[64];
		sprintf(fpsString, "frameTime: %.6f sec, FPS: %.6f", lastAllTicksTime / needTickCount, needTickCount / lastAllTicksTime);

		font->DrawString(canvas, fpsString, (uint32_t)'Zyyy', vec2(20.0f, (float)screenHeight - 20.0f), vec4(1, 0, 0, 1));
		font->DrawString(canvas, Banshee::bansheeDebug, (uint32_t)'Zyyy', vec2(20.0f, (float)screenHeight - 40.0f), vec4(0, 1, 0, 1));

		#if 0
		// normal control

		font->DrawString(canvas, "XXX", vec2(
			(float)screenWidth * 0.5f,
			(float)screenHeight * (1 - (0 - bansheeParams.rotorPitchControlMin) / (bansheeParams.rotorPitchControlMax - bansheeParams.rotorPitchControlMin)) - 1.0f
			), vec4(0, 0, 0, 1), Gui::Font::textOriginCenter | Gui::Font::textOriginMiddle);
		font->DrawString(canvas, "XXX", vec2(
			(float)screenWidth * 0.5f,
			(float)screenHeight * (1 - (0 - bansheeParams.rotorPitchControlMin) / (bansheeParams.rotorPitchControlMax - bansheeParams.rotorPitchControlMin))
			), vec4(1, 1, 1, 1), Gui::Font::textOriginCenter | Gui::Font::textOriginMiddle);
		// actual control
		font->DrawString(canvas, "XXX", vec2(
			(float)screenWidth * (1 + hero->getControlRoll() / bansheeParams.rotorRollControlBound) * 0.5f,
			(float)screenHeight * (1 - (hero->getControlPitch() - bansheeParams.rotorPitchControlMin) / (bansheeParams.rotorPitchControlMax - bansheeParams.rotorPitchControlMin)) - 1.0f
			), vec4(0, 0, 0, 1), Gui::Font::textOriginCenter | Gui::Font::textOriginMiddle);
		font->DrawString(canvas, "XXX", vec2(
			(float)screenWidth * (1 + hero->getControlRoll() / bansheeParams.rotorRollControlBound) * 0.5f,
			(float)screenHeight * (1 - (hero->getControlPitch() - bansheeParams.rotorPitchControlMin) / (bansheeParams.rotorPitchControlMax - bansheeParams.rotorPitchControlMin))
			), vec4(1, 1, 1, 1), Gui::Font::textOriginCenter | Gui::Font::textOriginMiddle);
		#endif

		canvas->Flush();
	}

	presenter->Present();
}

ptr<Game> Game::Get()
{
	return singleGame;
}

ptr<Texture> Game::LoadTexture(const String& fileName)
{
	return textureManager->Get(fileName);
}

ptr<Geometry> Game::LoadGeometry(const String& fileName)
{
	return NEW(Geometry(
		device->CreateStaticVertexBuffer(fileSystem->LoadFile(fileName + ".vertices"), geometryFormats->vl),
		device->CreateStaticIndexBuffer(fileSystem->LoadFile(fileName + ".indices"), sizeof(short))
	));
}

ptr<Geometry> Game::LoadSkinnedGeometry(const String& fileName)
{
	return NEW(Geometry(
		device->CreateStaticVertexBuffer(fileSystem->LoadFile(fileName + ".vertices"), geometryFormats->vlSkinned),
		device->CreateStaticIndexBuffer(fileSystem->LoadFile(fileName + ".indices"), sizeof(short))
	));
}

ptr<Skeleton> Game::LoadSkeleton(const String& fileName)
{
	return Skeleton::Deserialize(fileSystem->LoadStream(fileName));
}

ptr<BoneAnimation> Game::LoadBoneAnimation(const String& fileName, ptr<Skeleton> skeleton)
{
	if(!skeleton)
	{
		std::vector<Skeleton::Bone> bones(1);
		bones[0].originalWorldPosition = vec3(0, 0, 0);
		bones[0].originalRelativePosition = vec3(0, 0, 0);
		bones[0].parent = 0;
		skeleton = NEW(Skeleton(bones));
	}
	return BoneAnimation::Deserialize(fileSystem->LoadStream(fileName), skeleton);
}

ptr<Physics::Shape> Game::CreatePhysicsBoxShape(const vec3& halfSize)
{
	return physicsWorld->CreateBoxShape(halfSize);
}

ptr<Physics::Shape> Game::CreatePhysicsSphereShape(float radius)
{
	return physicsWorld->CreateSphereShape(radius);
}

ptr<Physics::Shape> Game::CreatePhysicsCompoundShape(ptr<Script::Any> anyShapes)
{
	int shapesCount = anyShapes->GetLength();
	std::vector<std::pair<mat4x4, ptr<Physics::Shape> > > desc(shapesCount);
	for(int i = 0; i < shapesCount; ++i)
	{
		ptr<Script::Any> offsetAndShape = anyShapes->Get(i);
		float x = offsetAndShape->Get(0)->AsFloat();
		float y = offsetAndShape->Get(1)->AsFloat();
		float z = offsetAndShape->Get(2)->AsFloat();
		ptr<Physics::Shape> shape = offsetAndShape->Get(3)->AsObject().FastCast<Physics::Shape>();
		desc[i].first = CreateTranslationMatrix(vec3(x, y, z));
		desc[i].second = shape;
	}
	return physicsWorld->CreateCompoundShape(desc);
}

ptr<Physics::RigidBody> Game::CreatePhysicsRigidBody(ptr<Physics::Shape> physicsShape, float mass, const vec3& position)
{
	Eigen::Affine3f startTransform = Eigen::Affine3f::Identity();
	startTransform.translate(toEigen(position));
	return physicsWorld->CreateRigidBody(physicsShape, mass, fromEigen(startTransform.matrix()));
}

void Game::AddStaticModel(ptr<Geometry> geometry, ptr<Material> material, const vec3& position)
{
	AddStaticModelWithScale(geometry, material, position, vec3(1, 1, 1));
}

void Game::AddStaticModelWithScale(ptr<Geometry> geometry, ptr<Material> material, const vec3& position, const vec3& scale)
{
	StaticModel model;
	model.geometry = geometry;
	model.material = material;
	Eigen::Affine3f transform = Eigen::Affine3f::Identity();
	transform.translate(toEigen(position));
	transform.scale(toEigen(scale));
	model.transform = fromEigen(transform.matrix());
	staticModels.push_back(model);
}

void Game::AddRigidModel(ptr<Geometry> geometry, ptr<Material> material, ptr<Physics::RigidBody> physicsRigidBody)
{
	RigidModel model;
	model.geometry = geometry;
	model.material = material;
	model.rigidBody = physicsRigidBody;
	rigidModels.push_back(model);
}

void Game::AddStaticRigidBody(ptr<Physics::RigidBody> rigidBody)
{
	staticRigidBodies.push_back(rigidBody);
}

ptr<StaticLight> Game::AddStaticLight()
{
	ptr<StaticLight> light = NEW(StaticLight());
	staticLights.push_back(light);
	return light;
}

void Game::SetAmbient(const vec3& color)
{
	this->ambientColor = color;
}

void Game::SetBackgroundTexture(ptr<Texture> texture)
{
	painter->SetBackgroundTexture(texture);
}

void Game::SetBansheeParams(
	ptr<Geometry> mainGeometry,
	ptr<Geometry> leftWingGeometry,
	ptr<Geometry> rightWingGeometry,
	ptr<Geometry> rotor1Geometry,
	ptr<Geometry> rotor2Geometry,
	ptr<Material> material,
	ptr<Physics::Shape> shape,
	const vec3& bansheeMassCenter,
	float mass,
	float linearDamping,
	float angularDamping,
	float rotorSpeed,
	float minRotorForce,
	float normalRotorForce,
	float maxRotorForce,
	float rotorForceChangeRate,
	float rotorPitchChangeRate,
	float rotorPitchControlMin,
	float rotorPitchControlMax,
	float rotorRollControlBound,
	float rotorPitchControlCoef,
	float rotorRollControlCoef,
	const vec3& lookOffset,
	const vec3& cameraOffset,
	float cameraSpeedCoef,
	const vec3& cubeSize
)
{
	bansheeParams.mainGeometry = mainGeometry;
	bansheeParams.leftWingGeometry = leftWingGeometry;
	bansheeParams.rightWingGeometry = rightWingGeometry;
	bansheeParams.rotor1Geometry = rotor1Geometry;
	bansheeParams.rotor2Geometry = rotor2Geometry;
	bansheeParams.material = material;
	bansheeParams.shape = shape;

	Banshee::moveMassCenter(bansheeMassCenter);

	bansheeParams.mass = mass;
	bansheeParams.linearDamping = linearDamping;
	bansheeParams.angularDamping = angularDamping;
	bansheeParams.rotorSpeed = rotorSpeed;
	bansheeParams.minRotorForce = minRotorForce;
	bansheeParams.normalRotorForce = normalRotorForce;
	bansheeParams.maxRotorForce = maxRotorForce;
	bansheeParams.rotorForceChangeRate = rotorForceChangeRate;
	bansheeParams.rotorPitchChangeRate = rotorPitchChangeRate;
	bansheeParams.rotorPitchControlMin = rotorPitchControlMin;
	bansheeParams.rotorPitchControlMax = rotorPitchControlMax;
	bansheeParams.rotorRollControlBound = rotorRollControlBound;
	bansheeParams.rotorPitchControlCoef = rotorPitchControlCoef;
	bansheeParams.rotorRollControlCoef = rotorRollControlCoef;
	bansheeParams.lookOffset = lookOffset;
	bansheeParams.cameraOffset = cameraOffset;
	bansheeParams.cameraSpeedCoef = cameraSpeedCoef;
	bansheeParams.cubeSize = cubeSize;
}

ptr<Game::Banshee> Game::CreateBanshee(const mat4x4& initialTransform)
{
	return NEW(Banshee(this, initialTransform));
}

void Game::PlaceHero(const vec3& position)
{
	Eigen::Affine3f startTransform = Eigen::Affine3f::Identity();
	startTransform.translate(toEigen(position));
	mat4x4 initialTransform = fromEigen(startTransform.matrix());

	hero = CreateBanshee(initialTransform);
}

void Game::PlaceCamera(const vec3& position, float alpha, float beta)
{
	this->cameraPosition = position;
	this->cameraAlpha = alpha;
	this->cameraBeta = beta;
}

void Game::SetCubeGeometry(ptr<Geometry> cubeGeometry)
{
	this->cubeGeometry = cubeGeometry;
}

void Game::SetDebugMaterial(ptr<Material> debugMaterial)
{
	this->debugMaterial = debugMaterial;
}

//******* Game::StaticLight

StaticLight::StaticLight() :
	position(-1, 0, 0), target(0, 0, 0), angle(pi / 4), nearPlane(0.1f), farPlane(100.0f), color(1, 1, 1), shadow(false)
{
	UpdateTransform();
}

void StaticLight::UpdateTransform()
{
	transform =
		CreateProjectionPerspectiveFovMatrix(angle, 1.0f, nearPlane, farPlane) *
		CreateLookAtMatrix(position, target, vec3(0, 0, 1));
}

void StaticLight::SetPosition(const vec3& position)
{
	this->position = position;
	UpdateTransform();
}

void StaticLight::SetTarget(const vec3& target)
{
	this->target = target;
	UpdateTransform();
}

void StaticLight::SetProjection(float angle, float nearPlane, float farPlane)
{
	this->angle = angle * 3.1415926535897932f / 180;
	this->nearPlane = nearPlane;
	this->farPlane = farPlane;
	UpdateTransform();
}

void StaticLight::SetColor(const vec3& color)
{
	this->color = color;
}

void StaticLight::SetShadow(bool shadow)
{
	this->shadow = shadow;
}
