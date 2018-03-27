-- kgsws' Lua Doom exports
-- player definition

function playerSpawn(pl)
	local mo
	mo = pl.mo
	-- give all ammo, even empty amount
	mo.InventoryGive(MT_CLIP, 50)
	mo.InventoryGive(MT_SHELL, 0)
	mo.InventoryGive(MT_ROCKETAMMO, 0)
	mo.InventoryGive(MT_CELL, 0)
	-- give default weapons
	mo.InventoryGive(MT_FIST)
	mo.InventoryGive(MT_PISTOL)
	-- set pistol
	pl.SetWeapon(MT_PISTOL, true)
end

function playerCrash(mobj)
	-- you can add fall damage or trampoline effect here
	if mobj.momz < mobj.gravity * -8 then
		mobj.SoundBody("dsoof")
		mobj.player.deltaviewheight = mobj.momz / 8
	end
end

a.PlayerDeath =
function(mobj)
	if game.map == "E1M8" and mobj.sector.special == 11 then
		-- Doom1 episode 1 level ending
		game.DoomExit()
	end
end

function eggPain(mobj)
	-- easter egg
	local attacker
	attacker = mobj.attacker
	if attacker and attacker ~= mobj and mobj.InventoryCheck(MT_REVENGERUNE) > 0 then
		attacker.Damage(true, 0)
		for i=0,64 do
			attacker.SpawnMissile(MT_EGGPARTBNC, doomRandom(0, 8191), doomRandom(-10, 10) * 0.1, doomRandom(100 - attacker.info.shootz*10, attacker.height*9) * 0.1, doomRandom(-240, 240) * 0.1, doomRandom(-240, 241) * 0.1)
		end
	end
end

mtype = {
	painSound = "dsplpain",
	deathSound = "dspldeth",
	xdeathSound = "dsslop",
	activeSound = "dsnoway",
	health = 100,
	radius = 16,
	height = 56,
	mass = 100,
	painChance = 256,
	shootz = 32,
	viewz = 41,
	bobz = 16,
	pass = 2,
	action_crash = playerCrash,
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
		{"PLAYG", 0},
		{"PLAYG", 4, eggPain},
		{"PLAYG", 4, a.SoundPain},
		"_spawn"
	},
	_death = {
		{"PLAYH", 10, a.PlayerDeath},
		{"PLAYI", 10, a.SoundDeath},
		{"PLAYJ", 10, a.Fall},
		{"PLAYK", 10},
		{"PLAYL", 10},
		{"PLAYM", 10},
		{"PLAYN", -1}
	},
	_xdeath = {
		{"PLAYO", 5, a.PlayerDeath},
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
MT_PLAYER = createMobjType(mtype)
setPlayerType(MT_PLAYER)

