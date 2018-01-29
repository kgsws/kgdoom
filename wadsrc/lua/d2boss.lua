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
	flags = mf.solid | mf.shootable | mf.fullVolume,
	_spawn = {
		{"BBRNA", -1}
	},
	_pain = {
		{"BBRNB", 36, a.SoundPain},
		"_spawn"
	},
	_death = {
		{"BBRNA", 120, a.SoundDeath},
		{"BBRNA", -1}
	}
}
createMobjType(mtype)

-- MT_BOSSSPIT
mtype = {
	seeSound = "dsbossit",
	ednum = 89,
	height = 32,
	shootz = 10,
	flags = mf.noBlockmap | mf.noSector | mf.fullVolume,
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
	flags = mf.Projectile | mf.noClip | mf.fullVolume,
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
	flags = mf.noBlockmap | mf.noSector,
	_spawn = {
		{"BOSFA", -1}
	}
}
MT_BOSSTARGET = createMobjType(mtype)

-- MT_SPAWNFIRE
mtype = {
	seeSound = "dsflame",
	flags = mf.noBlockmap | mf.noGravity,
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

