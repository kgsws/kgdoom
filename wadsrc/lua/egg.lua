-- kgsws' Lua Doom exports
-- easter egg kgDoom map

egg_render_style = {"!SHADOW", "!HOLEY0", "!HOLEY1"}

function eggSpawnFloor(sector, line)
	sector.AddFloor(line.frontsector, line, line.arg1)
end

function eggAddFloor(line)
	sectorTagIterator(line.arg0, eggSpawnFloor, line)
end

function eggSpawnEffect(thing)
	return false, thing
end

function eggParticle(mobj)
	if egg_target ~= nil then
		local mt
		an, sl = mobj.AttackAim(false, egg_target)
		mobj.angle = an
		if mobj.health == 2991 then
			mt = MT_EGGPARTBIG
		else
			mt = MT_EGGPARTICLE
		end
		mobj.SpawnMissile(mt, an, sl, doomRandom(0, 80) * 0.1, doomRandom(-40, 40) * 0.1, doomRandom(-40, 41) * 0.1)
	end
end

function eggParticleOrange(mobj)
	local mt
	local mo
	local rnd
	rnd = doomRandom()
	if rnd < mobj.health then
		mt = MT_EGGPARTBIG
	else
		mt = MT_EGGPARTICLE
	end
	mo = spawnMobj(mt, mobj.x + doomRandom(-40, 40) * 0.1, mobj.y + doomRandom(-40, 41) * 0.1, mobj.z + 128)
	mo.translation = "LIGHTMAP"
	mo.momz = -2.7
	if rnd + 128 < mobj.health then
		mobj._pain()
		mobj.translation = "LIGHTMAP"
	else
		mobj.translation = "-"
	end
end

function eggParticleDisable(mobj)
	local spot
	mobj.armor = mobj.armor + 1
	spot = thingTagIterator(mobj.armor, eggSpawnEffect)
	spot._death()
	if mobj.armor > 2 then
		eggParticleBoom(mobj)
	end
end

function eggParticleBoom(mobj)
	local i
	for i=0,8 do
		mobj.SpawnMissile(MT_EGGPARTBNC, doomRandom(0, 8191), doomRandom(-10, 10) * 0.1, doomRandom(-160, 320) * 0.1, doomRandom(-160, 160) * 0.1, doomRandom(-160, 161) * 0.1)
	end
end

function eggParticleBang(mobj)
	local i
	for i=0,64 do
		mobj.SpawnMissile(MT_EGGPARTBNC, doomRandom(0, 8191), doomRandom(-10, 10) * 0.1, doomRandom(0, 640) * 0.1, doomRandom(-240, 240) * 0.1, doomRandom(-240, 241) * 0.1)
	end
	mobj.SpawnMissile(MT_REVENGERUNE, doomRandom(0, 8191), -0.1, 16)
end

function eggParticleSelf(mobj)
	an, sl = mobj.AttackAim(false, egg_target)
	mobj.Thrust(3)
	mobj.momz = sl * 3
end

function eggOrangify(mobj)
	if mobj.info == MT_EGGPARTICLE or mobj.info == MT_EGGEFFECT then
		mobj.translation = "LIGHTMAP"
		mobj.health = 2991
	end
end

function eggOrangeTakeover()
	globalThingsIterator(eggOrangify)
	egg_target._death()
end

function eggOrangeBigger()
	egg_target.health = egg_target.health + 2
	if egg_target.health < 384 then
		return true
	else
		egg_target.TickerSet(2991, 35, nil, eggOrangeTakeover)
	end
end

function eggOrangeStart()
	egg_target._see()
	egg_target.TickerSet(2990, 2, nil, eggOrangeBigger)
end

function eggNextSpawn()
	egg_target = spawnMobj(MT_EGGBIG, egg_target.x, egg_target.y, egg_target.z)
	egg_target.TickerSet(2991, 35, nil, eggOrangeStart)
end

function eggStartSpawn()
	egg_target = thingTagIterator(5, eggSpawnEffect)
	egg_target.TickerSet(2991, 42, nil, eggNextSpawn)
end

function eggSwitch(mobj, line)
	local spot
	local mo
	line.DoButton("dsswtchn", "dsswtchn")
	spot = thingTagIterator(line.arg0, eggSpawnEffect)
	mo = spawnMobj(MT_EGGEFFECT, spot.x, spot.y, spot.z)
	mo.tag = line.arg0
	spot.Remove()
	egg_count = egg_count + 1
	if egg_count == 4 then
		mobj.TickerSet(2991, 2*35, nil, eggStartSpawn)
	end
end

linefunc[253] = eggSwitch

mtype = {
	__noClip = true,
	__noBlockmap = true,
	__noGravity = true,
	_spawn = {
		{"*TFOGI", 3, eggParticle},
		{"*TFOGJ", 3, eggParticle},
		"loop"
	},
	_death = {
		{"*TFOGH", 0, eggParticleSelf},
		{"*TFOGI", 3},
		{"*TFOGJ", 3},
		{"*TFOGI", 3},
		{"*TFOGJ", 3},
		{"*TFOGI", 3},
		{"*TFOGJ", 3},
		{"*TFOGI", 3},
		{"*TFOGJ", 3},
		{"*TFOGI", 3},
		{"*TFOGJ", 3},
		{"*TFOGI", 3},
		{"*TFOGJ", 3},
		{"*TFOGI", 3}
	}
}
MT_EGGEFFECT = createMobjType(mtype)

mtype = {
	speed = 3,
	__Projectile = true,
	__noClip = true,
	_spawn = {
		{"*TFOGG", 40}
	}
}
MT_EGGPARTICLE = createMobjType(mtype)

mtype = {
	translation = "LIGHTMAP",
	speed = 3,
	__Projectile = true,
	__noClip = true,
	_spawn = {
		{"*TFOGH", 40}
	}
}
MT_EGGPARTBIG = createMobjType(mtype)

mtype = {
	__noClip = true,
	__noBlockmap = true,
	__noGravity = true,
	_spawn = {
		{"*TFOGF", 3},
		{"*TFOGE", 3},
		{"*TFOGD", 3},
		{"*TFOGC", 3},
		{"*TFOGB", 3},
		"_spawn+3"
	},
	_see = {
		{"*TFOGC", 3, eggParticleOrange},
		{"*TFOGB", 3, eggParticleOrange},
		"loop"
	},
	_pain = {
		{"*TFOGA", 2},
		"_see"
	},
	_death = {
		{"*TFOGA", 70},
		{"*TFOGA", 20, eggParticleDisable},
		{"*TFOGA", 20, eggParticleDisable},
		{"*TFOGA", 20, eggParticleDisable},
		{"*TFOGA", 20, eggParticleDisable},
		{"*TFOGA", 20, eggParticleBoom},
		{"*TFOGA", 3, eggParticleBang},
		{"*TFOGD", 3},
		{"*TFOGE", 3},
		{"*TFOGF", 3},
		{"*TFOGH", 3},
		{"*TFOGG", 3}
	}
}
MT_EGGBIG = createMobjType(mtype)

mtype = {
	translation = "LIGHTMAP",
	speed = 1,
	damage = 10000,
	radius = 8,
	height = 8,
	pass = 3,
	bounce = 0.6,
	__missile = true,
	__noBlockmap = true,
	__dropOff = true,
	__noZChange = true,
	__wallBounce = true,
	__mobjBounce = true,
	_spawn = {
		{"*SPRKA", 2},
		{"*SPRKB", 2},
		"loop"
	},
	_death = {
		{"*SPRKA", 1}
	}
}
MT_EGGPARTBNC = createMobjType(mtype)

-- easter egg item
function eggExit()
	game.DoomExit()
end

function pickupRune(mobj, spec, arg)
	mobj.InventoryGive(spec.info)
	mobj.player.Message(arg)
	mobj.TickerSet(2991, 2*35, nil, eggExit)
	return pickup.item
end

mtype = {
	activeSound = "dsslop",
	gravity = 0.01,
	speed = 1,
	maxcount = 1,
	action = pickupRune,
	arg = "Picked up a revenge rune.",
	radius = 16,
	height = 16,
	__special = true,
	_spawn = {
		{"REVEA", 2},
		{"REVEB", 2},
		"loop"
	}
}
MT_REVENGERUNE = createMobjType(mtype)

