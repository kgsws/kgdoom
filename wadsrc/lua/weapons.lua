-- kgsws' Lua Doom exports
-- Doom weapons

a.Explode =
function(mobj)
	mobj.RadiusDamage(128, 128, 0, mobj.source, true)
end

a.FirePunch =
function(mobj)
	local an
	local sl
	local damage
	if mobj.InventoryCheck(MT_BERSERK) > 0 then
		damage = doomRandom(1, 10) * 20
	else
		damage = doomRandom(1, 10) * 2
	end
	an, sl = mobj.AttackAim(true)
	an, sl = mobj.LineAttack(MT_PUFF_MELEE, damage, an + (doomRandom() - doomRandom()) / 2, sl, 0, 0, 64)
	if sl then
		mobj.SoundWeapon("dspunch")
		mobj.Face(sl)
	end
end

a.FirePistol =
function(mobj)
	a.WeaponFlash(mobj)
	local an
	local sl
	an, sl = mobj.AttackAim(true)
	mobj.SoundWeapon("dspistol")
	if mobj.player.refire > 0 then
		an = an + (doomRandom() - doomRandom()) / 2
	end
	mobj.LineAttack(MT_PUFF, doomRandom(1, 3) * 5, an, sl)
end

a.FireShotgun =
function(mobj)
	a.WeaponFlash(mobj)
	local an
	local sl
	an, sl = mobj.AttackAim(true)
	mobj.SoundWeapon("dsshotgn")
	for i=1,7 do
		mobj.LineAttack(MT_PUFF, doomRandom(1, 3) * 5, an + (doomRandom() - doomRandom()) / 2, sl)
	end
end

a.FireShotgun2 =
function(mobj)
	a.WeaponFlash(mobj)
	local an
	local sl
	an, sl = mobj.AttackAim(true)
	mobj.SoundWeapon("dsdshtgn")
	for i=1,20 do
		mobj.LineAttack(MT_PUFF, doomRandom(1, 3) * 5, an + (doomRandom() - doomRandom()), sl + (doomRandom() - doomRandom()) / 2048)
	end
end

a.SgnCheck =
function(mobj)
	print("check for ammo")
end

a.SgnSound0 =
function(mobj)
	mobj.SoundWeapon("dsdbopn")
end

a.SgnSound1 =
function(mobj)
	mobj.SoundWeapon("dsdbload")
end

a.SgnSound2 =
function(mobj)
	mobj.SoundWeapon("dsdbcls")
end

a.FireChaingun0 =
function(mobj, flash)
	mobj.player.WeaponFlash(flash)
	local an
	local sl
	an, sl = mobj.AttackAim(true)
	mobj.SoundWeapon("dspistol")
	if mobj.player.refire > 0 then
		an = an + (doomRandom() - doomRandom()) / 2
	end
	mobj.LineAttack(MT_PUFF, doomRandom(1, 3) * 5, an, sl)
end

a.FireChaingun1 =
function(mobj)
	a.FireChaingun0(mobj, 1)
end

a.FireMissile =
function(mobj)
	local an
	local sl
	an, sl = mobj.AttackAim(false)
	mobj.SpawnMissile(MT_ROCKET, an, sl)
end

a.FirePlasma =
function(mobj)
	mobj.player.WeaponFlash(doomRandom(1))
	local an
	local sl
	an, sl = mobj.AttackAim(false)
	mobj.SpawnMissile(MT_PLASMA, an, sl)
end

a.BfgSound =
function(mobj)
	a.NoiseAlert(mobj)
	mobj.SoundWeapon("dsbfg")
end

a.FireBfg =
function(mobj)
	local an
	local sl
	an, sl = mobj.AttackAim(false)
	mobj.SpawnMissile(MT_BFG, an, sl)
end

a.BfgSpray =
function(mobj)
	if mobj.source ~= nil then
		local source
		local angle
		source = mobj.source
		angle = mobj.angle - 2048
		for i=0,39 do
			local target
			target = source.LineTarget(angle)
			if target ~= nil then
				spawnMobj(MT_EXTRABFG, target.x, target.y, target.z + target.info.height / 4)
				target.Damage(doomRandom(15, 120), 0, source, source)
			end
			angle = angle + 102.4
		end
	end
end

a.FireSaw =
function(mobj)
	local an
	local sl
	an, sl = mobj.AttackAim(true)
	an, sl = mobj.LineAttack(MT_PUFF, -12, an + (doomRandom() - doomRandom()) / 2, sl, 0, 0, 64)
	if sl then
		mobj.SoundWeapon("dssawhit")
		mobj.Face(sl)
	else
		mobj.SoundWeapon("dssawful")
	end
end

a.SawIdle =
function(mobj)
	mobj.SoundWeapon("dssawidl")
end

a.SawUp =
function(mobj)
	mobj.SoundWeapon("dssawup")
end

function pickupWeapon(mobj, spec, arg)
	local left
	if arg[1] ~= nil then
		left = mobj.InventoryGive(arg[1], arg[2])
	else
		left = 0
	end
	if mobj.InventoryCheck(spec.info) > 0 then
		if left >= arg[2] then
			return pickup.doNotPickup
		end
	else
		mobj.InventoryGive(spec.info)
		mobj.player.SetWeapon(spec.info)
	end
	mobj.player.Message(arg[3])
	return pickup.weapon
end

--
-- PROJECTILES
--

-- MT_ROCKET
mtype = {
	seeSound = "dsrlaunc",
	deathSound = "dsbarexp",
	speed = 20,
	radius = 11,
	height = 8,
	damage = -20,
	flags = mf.Projectile,
	_spawn = {
		{"*MISLA", -1}
	},
	_death = {
		{"*MISLB", 8, a.Explode},
		{"*MISLC", 6},
		{"*MISLD", 4}
	}
}
MT_ROCKET = createMobjType(mtype)

-- MT_PLASMA
mtype = {
	seeSound = "dsplasma",
	deathSound = "dsfirxpl",
	speed = 25,
	radius = 13,
	height = 8,
	damage = -5,
	flags = mf.Projectile,
	_spawn = {
		{"*PLSSA", 6},
		{"*PLSSB", 6},
		"loop"
	},
	_death = {
		{"*PLSEA", 4},
		{"*PLSEB", 4},
		{"*PLSEC", 4},
		{"*PLSED", 4},
		{"*PLSEE", 4}
	}
}
MT_PLASMA = createMobjType(mtype)

-- MT_BFG
mtype = {
	deathSound = "dsrxplod",
	speed = 25,
	radius = 13,
	height = 8,
	damage = -100,
	flags = mf.Projectile,
	_spawn = {
		{"*BFS1A", 4},
		{"*BFS1B", 4},
		"loop"
	},
	_death = {
		{"*BFE1A", 8},
		{"*BFE1B", 8},
		{"*BFE1C", 8, a.BfgSpray},
		{"*BFE1D", 8},
		{"*BFE1E", 8},
		{"*BFE1F", 8}
	}
}
MT_BFG = createMobjType(mtype)

-- MT_EXTRABFG
mtype = {
	radius = 20,
	height = 16,
	flags = mf.noBlockmap | mf.noGravity,
	_spawn = {
		{"*BFE2A", 8},
		{"*BFE2B", 8},
		{"*BFE2C", 8},
		{"*BFE2D", 8}
	}
}
MT_EXTRABFG = createMobjType(mtype)

--
-- WEAPONS
--

-- MT_FIST (custom type)
mtype = {
	maxcount = 1,
	radius = 20,
	height = 16,
	_spawn = {
		{"FISTA", -1}
	},
	_wRaise = {
		{"PUNGA", 1, a.WeaponRaise},
		"loop"
	},
	_wReady = {
		{"PUNGA", 1, a.WeaponReady},
		"loop"
	},
	_wLower = {
		{"PUNGA", 1, a.WeaponLower},
		"loop"
	},
	_wFireMain = {
		{"PUNGB", 4, a.NoiseAlert},
		{"PUNGC", 4, a.FirePunch},
		{"PUNGD", 5},
		{"PUNGC", 4},
		{"PUNGB", 5, a.WeaponRefire},
		"_wReady"
	}
}
MT_FIST = createMobjType(mtype)

-- MT_PISTOL (custom type)
mtype = {
	activeSound = "dswpnup",
	action = pickupWeapon,
	arg = {MT_CLIP, 20, "You got the pistol!"},
	maxcount = 1,
	radius = 20,
	height = 16,
	flags = mf.special,
	_spawn = {
		{"PISTA", -1}
	},
	_wRaise = {
		{"PISGA", 1, a.WeaponRaise},
		"loop"
	},
	_wReady = {
		{"PISGA", 1, a.WeaponReady},
		"loop"
	},
	_wLower = {
		{"PISGA", 1, a.WeaponLower},
		"loop"
	},
	_wFireMain = {
		{"PISGA", 4, a.NoiseAlert},
		{"PISGB", 6, a.FirePistol},
		{"PISGC", 4},
		{"PISGA", 5, a.WeaponRefire},
		"_wReady"
	},
	_wFlashMain = {
		{"*PISFA", 7}
	}
}
MT_PISTOL = createMobjType(mtype)

-- MT_SHOTGUN
mtype = {
	activeSound = "dswpnup",
	action = pickupWeapon,
	arg = {MT_SHELL, 8, "You got the shotgun!"},
	maxcount = 1,
	ednum = 2001,
	radius = 20,
	height = 16,
	flags = mf.special,
	_spawn = {
		{"SHOTA", -1}
	},
	_wRaise = {
		{"SHTGA", 1, a.WeaponRaise},
		"loop"
	},
	_wReady = {
		{"SHTGA", 1, a.WeaponReady},
		"loop"
	},
	_wLower = {
		{"SHTGA", 1, a.WeaponLower},
		"loop"
	},
	_wFireMain = {
		{"SHTGA", 3, a.NoiseAlert},
		{"SHTGA", 7, a.FireShotgun},
		{"SHTGB", 5},
		{"SHTGC", 5},
		{"SHTGD", 4},
		{"SHTGC", 5},
		{"SHTGB", 5},
		{"SHTGA", 3},
		{"SHTGA", 7, a.WeaponRefire},
		"_wReady"
	},
	_wFlashMain = {
		{"*SHTFA", 4},
		{"*SHTFB", 3}
	}
}
MT_SHOTGUN = createMobjType(mtype)

-- MT_SUPERSHOTGUN
mtype = {
	activeSound = "dswpnup",
	action = pickupWeapon,
	arg = {MT_SHELL, 8, "You got the super shotgun!"},
	maxcount = 1,
	ednum = 82,
	radius = 20,
	height = 16,
	flags = mf.special,
	_spawn = {
		{"SGN2A", -1}
	},
	_wRaise = {
		{"SHT2A", 1, a.WeaponRaise},
		"loop"
	},
	_wReady = {
		{"SHT2A", 1, a.WeaponReady},
		"loop"
	},
	_wLower = {
		{"SHT2A", 1, a.WeaponLower},
		"loop"
	},
	_wFireMain = {
		{"SHT2A", 3, a.NoiseAlert},
		{"SHT2A", 7, a.FireShotgun2},
		{"SHT2B", 7},
		{"SHT2C", 7, a.SgnCheck},
		{"SHT2D", 7, a.SgnSound0},
		{"SHT2E", 7},
		{"SHT2F", 7, a.SgnSound1},
		{"SHT2G", 6},
		{"SHT2H", 6, a.SgnSound2},
		{"SHT2A", 5, a.WeaponRefire},
		"_wReady"
	},
	_wFlashMain = {
		{"*SHT2I", 4},
		{"*SHT2J", 3}
	}
}
MT_SUPERSHOTGUN = createMobjType(mtype)

-- MT_CHAINGUN
mtype = {
	activeSound = "dswpnup",
	action = pickupWeapon,
	arg = {MT_CLIP, 20, "You got the chaingun!"},
	maxcount = 1,
	ednum = 2002,
	radius = 20,
	height = 16,
	flags = mf.special,
	_spawn = {
		{"MGUNA", -1}
	},
	_wRaise = {
		{"CHGGA", 1, a.WeaponRaise},
		"loop"
	},
	_wReady = {
		{"CHGGA", 1, a.WeaponReady},
		"loop"
	},
	_wLower = {
		{"CHGGA", 1, a.WeaponLower},
		"loop"
	},
	_wFireMain = {
		{"CHGGA", 0, a.NoiseAlert},
		{"CHGGA", 4, a.FireChaingun0},
		{"CHGGB", 4, a.FireChaingun1},
		{"CHGGB", 0, a.WeaponRefire},
		"_wReady"
	},
	_wFlashMain = {
		{"*CHGFA", 5},
		"stop",
		{"*CHGFB", 5}
	}
}
MT_CHAINGUN = createMobjType(mtype)

-- MT_LAUNCHER (renamed)
mtype = {
	activeSound = "dswpnup",
	action = pickupWeapon,
	arg = {MT_ROCKETAMMO, 2, "You got the rocket launcher!"},
	maxcount = 1,
	ednum = 2003,
	radius = 20,
	height = 16,
	flags = mf.special,
	_spawn = {
		{"LAUNA", -1}
	},
	_wRaise = {
		{"MISGA", 1, a.WeaponRaise},
		"loop"
	},
	_wReady = {
		{"MISGA", 1, a.WeaponReady},
		"loop"
	},
	_wLower = {
		{"MISGA", 1, a.WeaponLower},
		"loop"
	},
	_wFireMain = {
		{"MISGB", 0, a.NoiseAlert},
		{"MISGB", 8, a.WeaponFlash},
		{"MISGB", 12, a.FireMissile},
		{"MISGB", 0, a.WeaponRefire},
		"_wReady"
	},
	_wFlashMain = {
		{"*MISFA", 3},
		{"*MISFB", 4},
		{"*MISFC", 4},
		{"*MISFD", 4}
	}
}
MT_LAUNCHER = createMobjType(mtype)

-- MT_PLASMAGUN (renamed)
mtype = {
	activeSound = "dswpnup",
	action = pickupWeapon,
	arg = {MT_CELL, 40, "You got the plasma gun!"},
	maxcount = 1,
	ednum = 2004,
	radius = 20,
	height = 16,
	flags = mf.special,
	_spawn = {
		{"PLASA", -1}
	},
	_wRaise = {
		{"PLSGA", 1, a.WeaponRaise},
		"loop"
	},
	_wReady = {
		{"PLSGA", 1, a.WeaponReady},
		"loop"
	},
	_wLower = {
		{"PLSGA", 1, a.WeaponLower},
		"loop"
	},
	_wFireMain = {
		{"PLSGA", 0, a.NoiseAlert},
		{"PLSGA", 3, a.FirePlasma},
		{"PLSGB", 20, a.WeaponRefire},
		"_wReady"
	},
	_wFlashMain = {
		{"*PLSFA", 4},
		"stop",
		{"*PLSFB", 4}
	}
}
MT_PLASMAGUN = createMobjType(mtype)

-- MT_BFGW (renamed)
mtype = {
	activeSound = "dswpnup",
	action = pickupWeapon,
	arg = {MT_CELL, 40, "You got the BFG9000!  Oh, yes."},
	maxcount = 1,
	ednum = 2006,
	radius = 20,
	height = 16,
	flags = mf.special,
	_spawn = {
		{"BFUGA", -1}
	},
	_wRaise = {
		{"BFGGA", 1, a.WeaponRaise},
		"loop"
	},
	_wReady = {
		{"BFGGA", 1, a.WeaponReady},
		"loop"
	},
	_wLower = {
		{"BFGGA", 1, a.WeaponLower},
		"loop"
	},
	_wFireMain = {
		{"BFGGA", 20, a.BfgSound},
		{"BFGGB", 10, a.WeaponFlash},
		{"BFGGB", 10, a.FireBfg},
		{"BFGGB", 20, a.WeaponRefire},
		"_wReady"
	},
	_wFlashMain = {
		{"*BFGFA", 11},
		{"*BFGFB", 6}
	}
}
MT_BFGW = createMobjType(mtype)

-- MT_CHAINSAW (renamed)
mtype = {
	activeSound = "dswpnup",
	action = pickupWeapon,
	arg = {nil, 0, "A chainsaw!  Find some meat!"},
	maxcount = 1,
	ednum = 2005,
	radius = 20,
	height = 16,
	flags = mf.special,
	_spawn = {
		{"CSAWA", -1}
	},
	_wRaise = {
		{"SAWGC", 0, a.SawUp},
		{"SAWGC", 1, a.WeaponRaise},
		"_wRaise+1"
	},
	_wReady = {
		{"SAWGC", 0, a.SawIdle},
		{"SAWGC", 1, a.WeaponReady},
		{"SAWGC", 1, a.WeaponReady},
		{"SAWGC", 1, a.WeaponReady},
		{"SAWGC", 1, a.WeaponReady},
		{"SAWGD", 1, a.WeaponReady},
		{"SAWGD", 1, a.WeaponReady},
		{"SAWGD", 1, a.WeaponReady},
		{"SAWGD", 1, a.WeaponReady},
		"loop"
	},
	_wLower = {
		{"SAWGC", 1, a.WeaponLower},
		"loop"
	},
	_wFireMain = {
		{"SAWGA", 0, a.NoiseAlert},
		{"SAWGA", 4, a.FireSaw},
		{"SAWGB", 4, a.FireSaw},
		{"SAWGB", 0, a.WeaponRefire},
		"_wReady"
	}
}
MT_CHAINSAW = createMobjType(mtype)

