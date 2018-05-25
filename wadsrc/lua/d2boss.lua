-- kgsws' Lua Doom exports
-- Doom2 main boss

PIT_BrainTarget =
function(thing, list)
	if thing.info == MT_BOSSTARGET then
		table.insert(list, thing)
	end
end

a.BrainSpit =
function(mobj)
	local an
	local sl
	local targets
	targets = {}
	globalThingsIterator(PIT_BrainTarget, targets)
	if #targets > 0 then
		local mo
		targets = targets[doomRandom(1, #targets)]
		an, sl = mobj.AttackAim(false, targets)
		mobj.angle = an
		mo = mobj.SpawnMissile(MT_SPAWNSHOT, an, sl)
		mo.target = targets
	end
end

a.BrainScream =
function(mobj)
	local i
	local s
	local e
	local y
	a.SoundDeath(mobj)
	s = mobj.x - 196
	e = mobj.x + 312
	y = mobj.y - 320
	for i=s,e,8 do
		spawnMobj(MT_EXPLOSION, i, y, 128 + doomRandom() * 2)
	end
end

a.BrainExplode =
function(mobj)
	spawnMobj(MT_EXPLOSION, mobj.x + (doomRandom() - doomRandom()) * 0.03125, mobj.y, 128 + doomRandom() * 2)
end

a.BrainDie =
function(mobj)
	if game.map == "MAP30" then
		game.Exit("D2FINALE")
	else
		game.DoomExit()
	end
end

a.SpawnFly =
function(mobj)
	if mobj.Distance(mobj.target, false) < 24 then
		local rnd
		local type
		spawnMobj(MT_SPAWNFIRE, mobj.target.x, mobj.target.y)
		rnd = doomRandom()
		if rnd < 50 then
			type = MT_TROOP
		elseif rnd < 90 then
			type = MT_SERGEANT
		elseif rnd < 120 then
			type = MT_SHADOWS
		elseif rnd < 130 then
			type = MT_PAIN
		elseif rnd < 160 then
			type = MT_HEAD
		elseif rnd < 162 then
			type = MT_VILE
		elseif rnd < 172 then
			type = MT_UNDEAD
		elseif rnd < 192 then
			type = MT_BABY
		elseif rnd < 222 then
			type = MT_FATSO
		elseif rnd < 246 then
			type = MT_KNIGHT
		else
			type = MT_BRUISER
		end
		rnd = spawnMobj(type, mobj.target.x, mobj.target.y)
		mobj.Remove()
		blockThingsIterator(rnd, PIT_StompThing, rnd)
	end
end

-- MT_EXPLOSION (custom type)
mtype = {
	deathSound = "dsbarexp",
	gravity = 0,
	__noBlockmap = true,
	_spawn = {
		{"*MISLB", 10, a.SoundDeath},
		{"*MISLC", 10},
		{"*MISLD", 10, a.BrainExplode}
	}
}
MT_EXPLOSION = createMobjType(mtype)

-- MT_BOSSBRAIN
mtype = {
	painSound = "dsbospn",
	deathSound = "dsbosdth",
	ednum = 88,
	health = 250,
	radius = 16,
	height = 16,
	reactionTime = 8,
	painChance = 256,
	damageScale = {0},
	__solid = true,
	__shootable = true,
	__fullVolume = true,
	__noRadiusDmgZ = true,
	_spawn = {
		{"BBRNA", -1}
	},
	_pain = {
		{"BBRNB", 36, a.SoundPain},
		"_spawn"
	},
	_death = {
		{"BBRNA", 120, a.BrainScream},
		{"BBRNA", -1, a.BrainDie}
	}
}
createMobjType(mtype)

-- MT_BOSSSPIT
mtype = {
	seeSound = "dsbossit",
	ednum = 89,
	height = 32,
	shootz = 10,
	__noBlockmap = true,
	__noSector = true,
	__fullVolume = true,
	_spawn = {
		{"BBRNA", 10, a.Look},
		"loop"
	},
	_see = {
		{"BBRNA", 181},
		{"BBRNA", 150, a.BrainSpit},
		"_see+1"
	}
}
createMobjType(mtype)

-- MT_SPAWNSHOT
mtype = {
	seeSound = "dsbospit",
	speed = 10,
	gravity = 0,
	__Projectile = true,
	__noClip = true,
	__fullVolume = true,
	_spawn = {
		{"*BOSFA", 3, a.SpawnFly},
		{"*BOSFB", 3, a.SpawnFly},
		{"*BOSFC", 3, a.SpawnFly},
		{"*BOSFD", 3, a.SpawnFly},
		"loop"
	}
}
MT_SPAWNSHOT = createMobjType(mtype)

-- MT_BOSSTARGET
mtype = {
	ednum = 87,
	__noBlockmap = true,
	__noSector = true,
	_spawn = {
		{"BOSFA", -1}
	}
}
MT_BOSSTARGET = createMobjType(mtype)

-- MT_SPAWNFIRE
mtype = {
	seeSound = "dsflame",
	gravity = 0,
	__noBlockmap = true,
	_spawn = {
		{"*FIREA", 4, a.SoundSee},
		{"*FIREB", 4},
		{"*FIREC", 4},
		{"*FIRED", 4},
		{"*FIREE", 4},
		{"*FIREF", 4},
		{"*FIREG", 4},
		{"*FIREH", 4}
	}
}
MT_SPAWNFIRE = createMobjType(mtype)

--
-- custom Doom2 finale
--

function finalGetSpawnType(info)
	if info == MT_POSSESSED then
		return MT_SHOTGUY
	elseif info == MT_SHOTGUY then
		return MT_CHAINGUY
	elseif info == MT_CHAINGUY then
		return MT_TROOP
	elseif info == MT_TROOP then
		return MT_SERGEANT
	elseif info == MT_SERGEANT then
		return MT_SKULL
	elseif info == MT_SKULL then
		return MT_HEAD
	elseif info == MT_HEAD then
		return MT_KNIGHT
	elseif info == MT_KNIGHT then
		return MT_BRUISER
	elseif info == MT_BRUISER then
		return MT_BABY
	elseif info == MT_BABY then
		return MT_PAIN
	elseif info == MT_PAIN then
		return MT_UNDEAD
	elseif info == MT_UNDEAD then
		return MT_FATSO
	elseif info == MT_FATSO then
		return MT_VILE
	elseif info == MT_VILE then
		return MT_SPIDER
	elseif info == MT_SPIDER then
		return MT_CYBORG
	elseif info == MT_CYBORG then
		return MT_FINALPLAYER
	end
	return MT_POSSESSED
end

function finalePlayer(mobj)
	local an
	local sl
	a.SoundAttack(mobj)
	an, sl = mobj.AttackAim(true)
	mobj.angle = an
	an = an + (doomRandom() - doomRandom()) / 2
	mobj.LineAttack(MT_PUFF, doomRandom(1, 5) * 3, an, sl)
end

function finaleRemove(mobj)
	local info
	info = mobj.info
	if info ~= MT_PLAYER and info ~= MT_TELEPORTMAN then
		mobj.Remove()
	end
end

function finaleRespawn()
	globalThingsIterator(finaleRemove)
	thingTagIterator(1, finaleSpawn)
end

function finaleKill(mobj)
	mobj.Damage(false, 0)
	return false
end

function finaleGoKill(mobj)
	thingTagIterator(666, finaleKill)
end

function finaleSpawn(mobj, target)
	if target ~= nil then
		mobj.target = target
	end
	local mo
	local info
	info = finalGetSpawnType(mobj.armortype)
	mobj.armortype = info
	mo = spawnMobj(info, mobj.x, mobj.y, mobj.z, mobj.angle, false)
	mo.speed = 0
	mo.tag = 666
	mo.target = mobj.target
	return false
end

function finaleTarget(mobj)
	mobj.health = 1
	mobj.radius = 0
	mobj.height = 56
	mobj.__invulnerable = true
	mobj.__noRadiusDamage = true
	mobj.__shootable = true
	mobj.gravity = 0
	return false, mobj
end

function finaleInit()
	local mo
	mo = thingTagIterator(2, finaleTarget)
	thingTagIterator(1, finaleSpawn, mo)
	setBackground("BOSSBACK")
end

mtype = {
	_wReady = {
		{"UNKNZ", 1, a.WeaponReady},
		"loop"
	},
	_wFireMain = {
		{"UNKNZ", 70, finaleGoKill},
		{"UNKNZ", 35, finaleRespawn},
		"_wReady"
	}
}
MT_FINALEWEAPON = createMobjType(mtype)

mtype = {
	deathSound = "dspldeth",
	attackSound = "dspistol",
	health = 100,
	radius = 16,
	height = 56,
	mass = 100,
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
		{"PLAYA", 10, a.Look},
		"loop"
	},
	_see = {
		{"PLAYA", 4, a.Chase},
		{"PLAYB", 4, a.Chase},
		{"PLAYC", 4, a.Chase},
		{"PLAYD", 4, a.Chase},
		"loop"
	},
	_missile = {
		{"PLAYE", 6},
		{"*PLAYF", 6, finalePlayer},
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
	}
}
MT_FINALPLAYER = createMobjType(mtype)
