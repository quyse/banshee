local game = Banshee.Game.Get()

Banshee.t = {}
local t = Banshee.t

game:SetAmbient({0.02, 0.02, 0.02})
game:SetBackgroundTexture(game:LoadTexture("/new_york.png"))

-- материал кубика
local matCube = Banshee.Material()
t.matCube = matCube
matCube:SetDiffuse({1, 1, 1, 1})
matCube:SetSpecular({0.2, 0, 0, 0})

-- debug material
local matDebug = Banshee.Material()
t.matDebug = matDebug
matDebug:SetDiffuse({1, 0, 0, 0.5})
game:SetDebugMaterial(matDebug)

local matBench = Banshee.Material()
t.matBench = matBench
matBench:SetDiffuse({1, 1, 1, 1})
matBench:SetSpecular({0.1, 0.1, 0.1, 0.2})
game:AddStaticModel(game:LoadGeometry("/bench.geo"), matBench, { 0, 2, 0 })

local matNescafe = Banshee.Material()
local texNescafe = game:LoadTexture("/nescafe.png")
matNescafe:SetDiffuseTexture(texNescafe)
matNescafe:SetSpecularTexture(texNescafe)
game:AddStaticModel(game:LoadGeometry("/nescafe.geo"), matNescafe, { 2, 0, 0 })

local geoCube = game:LoadGeometry("/box.geo")
game:SetCubeGeometry(geoCube)
local shapeCube = game:CreatePhysicsBoxShape({ 1, 1, 1 })
game:AddStaticRigidBody(game:CreatePhysicsRigidBody(game:CreatePhysicsBoxShape({ 10, 10, 1 }), 0, { 11, 11, -1 }))
for i = 1, 10 do
	for j = 1, 10 do
		game:AddStaticModel(geoCube, matCube, { i * 2, j * 2, -1 })
	end
end

local matFloor = Banshee.Material()
matFloor:SetDiffuseTexture(game:LoadTexture("/floor.png"))
game:AddStaticRigidBody(game:CreatePhysicsRigidBody(game:CreatePhysicsBoxShape({ 100, 100, 1 }), 0, { 0, 0, 0}))
game:AddStaticModelWithScale(geoCube, matFloor, { 0, 0, 0 }, { 100, 100, 1 })
--[[ 8x8x4 (256) ok, 9x9x4 (324) not, 10x10x3 (300) ok, 7x7x5 (245) ok, 8x8x5 (320) not
for i = 1, 4 do
for j = 1, 4 do
	for k = 1, 5 do
		game:AddRigidModel(geoCube, matCube, game:CreatePhysicsRigidBody(shapeCube, 100, { i * 2 + k, j * 2 + k, k * 4 + 20 }))
		--game:AddRigidModel(geoCube, matCube, game:CreatePhysicsRigidBody(shapeCube, 100, { i * 2, i * 2 + 1, 13 }))
		--game:AddRigidModel(geoCube, matCube, game:CreatePhysicsRigidBody(shapeCube, 100, { i * 2, i * 2 - 1, 13 }))
	end
end
end
--]]
--[[
local light1 = game:AddStaticLight()
light1:SetPosition(10, 10, 5)
light1:SetTarget(9, 9, 1)
light1:SetShadow(true)
--]]
local light2 = game:AddStaticLight()
light2:SetColor({1,1,1})
light2:SetPosition({30, -10, 20})
light2:SetTarget({11, 11, 0})
light2:SetProjection(45, 0.1, 100)
light2:SetShadow(true)

-- установка параметров

function addThing(x, y, z)
	--[[
	local light = game:AddStaticLight()
	light:SetPosition({x, y, z + 100})
	light:SetTarget({x, y, z})
	light:SetProjection(45, 0.1, 100)
	light:SetShadow(false)
	--]]

	local shape = game:CreatePhysicsBoxShape({ 1, 1, 1 })

	for i = -4, 4 do
		for j = -4, 4 do
			local p = {x + j * 20, y + i * 20, z}
			game:AddStaticRigidBody(game:CreatePhysicsRigidBody(shape, 0, p))
			game:AddStaticModel(geoCube, matCube, p)
		end
	end

	for k = 0, 20 do
		local p = {x, y, z + k * 2}
		game:AddStaticRigidBody(game:CreatePhysicsRigidBody(shape, 0, p))
		game:AddStaticModel(geoCube, matCube, p)
	end
end

addThing(0, 0, 2)
addThing(200, 0, 2)
addThing(0, 200, 2)
addThing(200, 200, 2)

local bansheeMainGeometry = game:LoadGeometry("/banshee_main.geo")
local bansheeLeftWingGeometry = game:LoadGeometry("/banshee_left_wing.geo")
local bansheeRightWingGeometry = game:LoadGeometry("/banshee_right_wing.geo")
local bansheeRotor1Geometry = game:LoadGeometry("/banshee_rotor1.geo")
local bansheeRotor2Geometry = game:LoadGeometry("/banshee_rotor2.geo")
local bansheeMaterial = Banshee.Material()
bansheeMaterial:SetDiffuseTexture(game:LoadTexture("/banshee.png"))
-- bansheeMaterial:SetDiffuse({0.5, 0.5, 0.5, 1});
bansheeMaterial:SetSpecular({1, 0, 0, 0})
local bansheeCubeSize = {1,1,1}
local bansheeShape = game:CreatePhysicsCompoundShape({
	{ 0, 0, 0, game:CreatePhysicsBoxShape({1,1,1}) },
	{ 1, 0, 0, game:CreatePhysicsBoxShape({1,1,0.5}) },
	{ -1, 0, 0, game:CreatePhysicsBoxShape({1,1,0.5}) }
	})
-- local bansheeShape = game:CreatePhysicsSphereShape(1)
game:SetBansheeParams(bansheeMainGeometry
	, bansheeLeftWingGeometry
	, bansheeRightWingGeometry
	, bansheeRotor1Geometry
	, bansheeRotor2Geometry
	, bansheeMaterial
	, bansheeShape
	, 9 -- mass
	, 0.2 -- linear damping
	, 0.9 -- angular damping
	, 1 -- rotorSpeed
	, 30 -- min rotor force
	, 44 -- normal rotor force
	, 90 -- max rotor force
	, 20 -- rotorForceChangeRate
	, 0.5 -- rotorPitchChangeRate
	, -2  -- rotorPitchControlMin
	, 2   -- rotorPitchControlMax
	, 2  -- rotorRollControlBound
	, 0.5 -- rotorPitchControlCoef
	, 0.5 -- rotorRollControlCoef
	, {0, 0, -1} -- look offset
	, {-1, -10, 0} -- cameraOffset
	, -5 -- cameraSpeedCoef
	, bansheeCubeSize
)

game:PlaceHero({10, 10, 10})
game:PlaceCamera({ 20.0, 0.0, 10.0 }, 2.315, -0.625);
