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

for i = -10, 10 do
	for j = -10, 10 do
		game:AddStaticModel(geoCube, matCube, { i * 20, j * 20, -1 })
	end
end

local matFloor = Banshee.Material()
matFloor:SetDiffuseTexture(game:LoadTexture("/floor.png"))

-- floor
game:AddStaticRigidBody(game:CreatePhysicsRigidBody(game:CreatePhysicsBoxShape({ 10000, 10000, 1 }), 0, { 0, 0, -1}))
game:AddStaticModelWithScale(geoCube, matFloor, { 0, 0, -1 }, { 10000, 10000, 1 })

-- -- ceil
-- game:AddStaticRigidBody(game:CreatePhysicsRigidBody(game:CreatePhysicsBoxShape({ 10000, 10000, 1 }), 0, { 0, 0, 101 }))
-- game:AddStaticModelWithScale(geoCube, matFloor, { 0, 0, 101 }, { 10000, 10000, 1 })

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

local light3 = game:AddStaticLight()
light3:SetPosition({-300, -100, 200})
light3:SetTarget({11, 11, 0})
light3:SetProjection(180, 0.1, 100)
light3:SetShadow(true)

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

local s = 0.02
local s2 = s / 2;

local banshee_mass_center_x = 0;
local banshee_mass_center_y = 10 * s;
local banshee_mass_center_z = -60 * s;

local bansheeShape = game:CreatePhysicsCompoundShape({
	{       0 - banshee_mass_center_x,   -177 * s - banshee_mass_center_y,    -13 * s - banshee_mass_center_z, game:CreatePhysicsBoxShape({    120 * s2,     37 * s2,     55 * s2}) },
	{       0 - banshee_mass_center_x,   -128 * s - banshee_mass_center_y,    -13 * s - banshee_mass_center_z, game:CreatePhysicsBoxShape({     30 * s2,    127 * s2,     17 * s2}) },
	{       0 - banshee_mass_center_x,     -2 * s - banshee_mass_center_y,    -46 * s - banshee_mass_center_z, game:CreatePhysicsBoxShape({    200 * s2,     48 * s2,     40 * s2}) },
	{       0 - banshee_mass_center_x,     50 * s - banshee_mass_center_y,    -62 * s - banshee_mass_center_z, game:CreatePhysicsBoxShape({    112 * s2,     66 * s2,     30 * s2}) },
	{       0 - banshee_mass_center_x,    -16 * s - banshee_mass_center_y,    -23 * s - banshee_mass_center_z, game:CreatePhysicsBoxShape({     75 * s2,    144 * s2,     55 * s2}) },
	{  98 * s - banshee_mass_center_x,          0 - banshee_mass_center_y,     12 * s - banshee_mass_center_z, game:CreatePhysicsBoxShape({     74 * s2,     83 * s2,     35 * s2}) },
	{ -98 * s - banshee_mass_center_x,          0 - banshee_mass_center_y,     12 * s - banshee_mass_center_z, game:CreatePhysicsBoxShape({     74 * s2,     83 * s2,     35 * s2}) }
	})

-- local bansheeShape = game:CreatePhysicsCompoundShape({
-- 	{ 10, 0, -1, game:CreatePhysicsBoxShape({4,4,4}) },
-- 	{ 1, 0, 0, game:CreatePhysicsBoxShape({1,1,0.5}) },
-- 	{ -1, 0, 0, game:CreatePhysicsBoxShape({1,1,0.5}) }
-- 	})

-- local bansheeShape = game:CreatePhysicsSphereShape(3)

-- local bansheeShape = game:CreatePhysicsBoxShape({5, 5, 0.0000005})

game:SetBansheeParams(bansheeMainGeometry
	, bansheeLeftWingGeometry
	, bansheeRightWingGeometry
	, bansheeRotor1Geometry
	, bansheeRotor2Geometry
	, bansheeMaterial
	, bansheeShape
	, {banshee_mass_center_x, banshee_mass_center_y, banshee_mass_center_z} -- banshee mass center
	, 9 -- mass
	, 0.2 -- linear damping
	, 0.2 -- angular damping
	, 1 -- rotorSpeed
	, 30 -- min rotor force
	, 40 -- normal rotor force
	, 70 -- max rotor force
	, 20 -- rotorForceChangeRate
	, 5.5 -- rotorPitchChangeRate
	, -0.3  -- rotorPitchControlMin
	, 0.3   -- rotorPitchControlMax
	, 1.3  -- rotorRollControlBound
	, 0.0005 -- rotorPitchControlCoef
	, 0.0005 -- rotorRollControlCoef
	, {0, 0, -1} -- look offset
	, {-1, -10, 0} -- cameraOffset
	, -5 -- cameraSpeedCoef
	, bansheeCubeSize
)

game:PlaceHero({10, 10, 10})
game:PlaceCamera({ 20.0, 0.0, 10.0 }, 2.315, -0.625);
