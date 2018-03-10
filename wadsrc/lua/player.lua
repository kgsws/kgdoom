-- kgsws' Lua Doom exports
-- player definition

function playerSpawn(pl)
	-- give all ammo, even empty amount
	pl.mo.InventoryGive(MT_CLIP, 50)
	pl.mo.InventoryGive(MT_SHELL, 0)
	pl.mo.InventoryGive(MT_ROCKETAMMO, 0)
	pl.mo.InventoryGive(MT_CELL, 0)
	-- give default weapons
	pl.mo.InventoryGive(MT_FIST)
	pl.mo.InventoryGive(MT_PISTOL)
	-- set pistol
	pl.SetWeapon(MT_PISTOL, true)
end

function pain(mobj)
	-- easter egg
	if mobj.attacker and mobj.attacker ~= mobj then
		mobj.attacker.Damage(true, 0)
	end
end

mtype = {
	painSound = "dsplpain",
	deathSound = "dspldeth",
	xdeathSound = "dsslop",
	health = 100,
	radius = 16,
	height = 56,
	mass = 100,
	painChance = 256,
	shootz = 32,
	viewz = 41,
	bobz = 16,
	__solid = true,
	__shootable = true,
	__dropOff = true,
	__pickup = true,
	__slide = true,
	_spawn = {
		{"PLAYA", -1}
	},
	_see = {
		{"PLAYA", 4},
		{"PLAYB", 4},
		{"PLAYC", 4},
		{"PLAYD", 4},
		"loop"
	},
	_melee = {
		{"PLAYE", 12},
		"_spawn"
	},
	_missile = {
		{"*PLAYF", 6},
		"_spawn"
	},
	_pain = {
		{"PLAYG", 4, pain},
		{"PLAYG", 4, a.SoundPain},
		"_spawn"
	},
	_death = {
		{"PLAYH", 10},
		{"PLAYI", 10, a.SoundDeath},
		{"PLAYJ", 10, a.Fall},
		{"PLAYK", 10},
		{"PLAYL", 10},
		{"PLAYM", 10},
		{"PLAYN", -1}
	},
	_xdeath = {
		{"PLAYO", 5},
		{"PLAYP", 5, a.SoundXDeath},
		{"PLAYQ", 5, a.Fall},
		{"PLAYR", 5},
		{"PLAYS", 5},
		{"PLAYT", 5},
		{"PLAYU", 5},
		{"PLAYV", 5},
		{"PLAYW", -1}
	},
	_crush = {
		{"POL5A0", -1, a.Crushed}
	}
}
setPlayerType(createMobjType(mtype))

