local game = Banshee.Game.Get()

Banshee.t = {}
local t = Banshee.t

game:SetAmbient({0.02, 0.02, 0.02})

-- материал кубика
local matCube = Banshee.Material()
t.matCube = matCube
matCube:SetDiffuse({1, 1, 1, 1})
matCube:SetSpecular({0.2, 0, 0, 0})

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
local shapeCube = game:CreatePhysicsBoxShape({ 1, 1, 1 })
game:AddStaticRigidBody(game:CreatePhysicsRigidBody(game:CreatePhysicsBoxShape({ 10, 10, 1 }), 0, { 11, 11, -1 }))
for i = 1, 10 do
	for j = 1, 10 do
		game:AddStaticModel(geoCube, matCube, { i * 2, j * 2, -1 })
	end
end
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
light2:SetPosition({30, -10, 20})
light2:SetTarget({11, 11, 0})
light2:SetProjection(45, 0.1, 100)
light2:SetShadow(true)

-- установка параметров

local bansheeMainGeometry = game:LoadGeometry("/banshee_main.geo")
local bansheeLeftWingGeometry = game:LoadGeometry("/banshee_left_wing.geo")
local bansheeRightWingGeometry = game:LoadGeometry("/banshee_right_wing.geo")
local bansheeRotor1Geometry = game:LoadGeometry("/banshee_rotor1.geo")
local bansheeRotor2Geometry = game:LoadGeometry("/banshee_rotor2.geo")
local bansheeMaterial = Banshee.Material()
--bansheeMaterial:SetDiffuseTexture(game:LoadTexture("/nescafe.png"))
bansheeMaterial:SetDiffuse({0.5, 0.5, 0.5, 1});
bansheeMaterial:SetSpecular({0.5, 0, 0, 0})
local bansheeShape = game:CreatePhysicsBoxShape({1,1,1})
game:SetBansheeParams(bansheeMainGeometry
	, bansheeLeftWingGeometry
	, bansheeRightWingGeometry
	, bansheeRotor1Geometry
	, bansheeRotor2Geometry
	, bansheeMaterial
	, bansheeShape
	, 9 -- mass
	, 1 -- rotorSpeed
	, 10 -- min rotor force
	, 10000 -- max rotor force
	, 2000 -- rotorForceChangeRate
	, 5 -- rotorPitchChangeRate
	, -1  -- rotorPitchControlMin
	, 1   -- rotorPitchControlMax
	, 1  -- rotorRollControlBound
	, 0.5 -- rotorPitchControlCoef,
	, 0.5 -- rotorRollControlCoef
)

game:PlaceHero({10, 10, 10})
game:PlaceCamera({ 20.0, 0.0, 10.0 }, 2.315, -0.625);
