-- kgsws' Lua Doom exports
-- Doom monsters

a.Fire =
function(mobj)
	if mobj.source ~= nil and mobj.source.health >= 0 then
		local mo
		mo = mobj.source.target
		if mobj.source.CheckSight() then
			local x
			local y
			local d
			d = mo.radius + mobj.radius
			x = mo.x + finecosine[mo.angle] * d
			y = mo.y + finesine[mo.angle] * d
			mobj.Teleport(x, y, mo.z, false)
		end
	end
end

a.FireStart =
function(mobj)
	a.SoundSee(mobj)
	a.Fire(mobj)
end

a.FireCrackle =
function(mobj)
	a.SoundActive(mobj)
	a.Fire(mobj)
end

PIT_VileCheck =
function(thing, vile)
	if thing.Flag(mf.corpse) and thing.tics == -1 then
		local temp
		temp = thing.height
		thing.height = thing.info.height
		if thing.CheckPosition() and thing._raise() then
			vile._heal()
			thing.SoundBody("dsslop")
			thing.flags = thing.info.flags
			thing.health = thing.info.health
			thing.target = nil
			return false, thing
		else
			thing.height = temp
		end
	end
end

a.VileChase =
function(mobj)
	local x
	local y
	x = mobj.x + finecosine[mobj.angle] * mobj.radius * 2
	y = mobj.y + finesine[mobj.angle] * mobj.radius * 2
	if blockThingsIterator(x, y, PIT_VileCheck, mobj) == nil then
		a.Chase(mobj)
	end
end

a.VileStart =
function(mobj)
	a.SoundAttack(mobj)
	a.FaceTarget(mobj)
end

a.VileTarget =
function(mobj)
	local mo
	a.FaceTarget(mobj)
	mo = spawnMobj(MT_FIRE, mobj.target.x, mobj.target.y)
	mo.source = mobj
	mobj.mobj = mo
	a.Fire(mo)
end

a.VileAttack =
function(mobj)
	a.FaceTarget(mobj)
	if mobj.CheckSight() then
		mobj.target.Damage(20, 0, mobj, mobj)
		mobj.target.momz = 1000 / mobj.target.info.mass
		mobj.mobj.RadiusDamage(70, 70, 0, mobj, false)
		mobj.mobj.SoundWeapon("dsbarexp")
	end
end

-- MT_FIRE
mtype = {
	seeSound = "dsflamst",
	activeSound = "dsflame",
	radius = 8,
	height = 16,
	flags = mf.noBlockmap | mf.noGravity,
	_spawn = {
		{"*FIREA", 1},
		{"*FIREA", 2, a.FireStart},
		{"*FIREB", 2, a.Fire},
		{"*FIREA", 2, a.Fire},
		{"*FIREB", 2, a.Fire},
		{"*FIREC", 2, a.Fire},
		{"*FIREB", 2, a.Fire},
		{"*FIREC", 2, a.FireCrackle},
		{"*FIREB", 2, a.Fire},
		{"*FIREC", 2, a.Fire},
		{"*FIRED", 2, a.Fire},
		{"*FIREC", 2, a.Fire},
		{"*FIRED", 2, a.Fire},
		{"*FIREC", 2, a.Fire},
		{"*FIRED", 2, a.Fire},
		{"*FIREE", 2, a.Fire},
		{"*FIRED", 2, a.Fire},
		{"*FIREE", 2, a.Fire},
		{"*FIRED", 2, a.Fire},
		{"*FIREE", 2, a.FireCrackle},
		{"*FIREF", 2, a.Fire},
		{"*FIREE", 2, a.Fire},
		{"*FIREF", 2, a.Fire},
		{"*FIREE", 2, a.Fire},
		{"*FIREF", 2, a.Fire},
		{"*FIREG", 2, a.Fire},
		{"*FIREH", 2, a.Fire},
		{"*FIREG", 2, a.Fire},
		{"*FIREH", 2, a.Fire},
		{"*FIREG", 2, a.Fire},
		{"*FIREH", 2, a.Fire},
	}
}
MT_FIRE = createMobjType(mtype)


-- MT_VILE
mtype = {
	seeSound = "dsvilsit",
	painSound = "dsvipain",
	activeSound = "dsvilact",
	attackSound = "dsvilatk",
	deathSound = "dsvildth",
	ednum = 64,
	health = 700,
	radius = 20,
	height = 56,
	mass = 500,
	speed = 15,
	reactionTime = 8,
	painChance = 10,
	shootz = 32,
	damageScale = {0},
	flags = mf.Monster,
	_spawn = {
		{"VILEA", 10, a.Look},
		{"VILEB", 10, a.Look},
		"loop"
	},
	_see = {
		{"VILEA", 2, a.VileChase},
		{"VILEA", 2, a.VileChase},
		{"VILEB", 2, a.VileChase},
		{"VILEB", 2, a.VileChase},
		{"VILEC", 2, a.VileChase},
		{"VILEC", 2, a.VileChase},
		{"VILED", 2, a.VileChase},
		{"VILED", 2, a.VileChase},
		{"VILEE", 2, a.VileChase},
		{"VILEE", 2, a.VileChase},
		{"VILEF", 2, a.VileChase},
		{"VILEF", 2, a.VileChase},
		"loop"
	},
	_missile = {
		{"*VILEG", 10, a.VileStart},
		{"*VILEH", 8, a.VileTarget},
		{"*VILEI", 8, a.FaceTarget},
		{"*VILEJ", 8, a.FaceTarget},
		{"*VILEK", 8, a.FaceTarget},
		{"*VILEL", 8, a.FaceTarget},
		{"*VILEM", 8, a.FaceTarget},
		{"*VILEN", 8, a.FaceTarget},
		{"*VILEO", 8, a.VileAttack},
		{"*VILEP", 20},
		"_see"
	},
	_pain = {
		{"VILEQ", 5},
		{"VILEQ", 5, a.SoundPain},
		"_see"
	},
	_death = {
		{"VILEQ", 7},
		{"VILER", 7, a.SoundDeath},
		{"VILES", 7, a.Fall},
		{"VILET", 7},
		{"VILEU", 7},
		{"VILEV", 7},
		{"VILEW", 7},
		{"VILEX", 7},
		{"VILEY", 7},
		{"VILEZ", -1}
	},
	_heal = {
		{"*VILE[", 10},
		{"*VILE\\", 10},
		{"*VILE]", 10},
		"_see"
	}
}
MT_VILE = createMobjType(mtype)

