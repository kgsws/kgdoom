-- kgsws' Lua Doom exports
-- Doom monsters

a.SkelMissile =
function(mobj)
	local an
	local sl
	an, sl = mobj.AttackAim()
	mobj.angle = an
	mobj.SpawnMissile(MT_TRACER, an, sl)
end

a.SkelWoosh =
function(mobj)
	a.FaceTarget(mobj)
	mobj.SoundWeapon("dsskeswg")
end

a.SkelFist =
function(mobj)
	if mobj.MeleeRange() then
		a.FaceTarget(mobj)
		a.SoundAttack(mobj)
		mobj.target.Damage(doomRandom(1, 10) * 6, 0, mobj, mobj)
	end
end

-- MT_TRACER
mtype = {
	seeSound = "dsskeatk",
	deathSound = "dsbarexp",
	speed = 10,
	radius = 11,
	height = 8,
	damage = -10,
	flags = mf.Projectile,
	_spawn = {
		{"*FATBA", 2},
		{"*FATBB", 2},
		"loop"
	},
	_death = {
		{"*FBXPA", 8},
		{"*FBXPB", 6},
		{"*FBXPC", 4}
	}
}
MT_TRACER = createMobjType(mtype)

-- MT_UNDEAD
mtype = {
	seeSound = "dsskesit",
	painSound = "dspopain",
	activeSound = "dsskeact",
	attackSound = "dsskepcht",
	deathSound = "dsskedth",
	ednum = 66,
	health = 300,
	radius = 20,
	height = 56,
	mass = 500,
	speed = 10,
	reactionTime = 8,
	painChance = 100,
	shootz = 48,
	species = 1,
	damageScale = {0},
	flags = mf.Monster,
	_spawn = {
		{"SKELA", 10, a.Look},
		{"SKELB", 10, a.Look},
		"loop"
	},
	_see = {
		{"SKELA", 2, a.Chase},
		{"SKELA", 2, a.Chase},
		{"SKELB", 2, a.Chase},
		{"SKELB", 2, a.Chase},
		{"SKELC", 2, a.Chase},
		{"SKELC", 2, a.Chase},
		{"SKELD", 2, a.Chase},
		{"SKELD", 2, a.Chase},
		{"SKELE", 2, a.Chase},
		{"SKELE", 2, a.Chase},
		{"SKELF", 2, a.Chase},
		{"SKELF", 2, a.Chase},
		"loop"
	},
	_melee = {
		{"SKELG", 6, a.SkelWoosh},
		{"SKELH", 6, a.FaceTarget},
		{"SKELI", 6,a.SkelFist},
		"_see"
	},
	_missile = {
		{"*SKELJ", 10, a.FaceTarget},
		{"SKELK", 10, a.SkelMissile},
		{"SKELK", 10, a.FaceTarget},
		"_see"
	},
	_pain = {
		{"SKELL", 5},
		{"SKELL", 5, a.SoundPain},
		"_see"
	},
	_death = {
		{"SKELL", 7},
		{"SKELM", 7},
		{"SKELN", 7, a.SoundDeath},
		{"SKELO", 7, a.Fall},
		{"SKELP", 7},
		{"SKELQ", -1}
	},
	_raise = {
		{"SKELQ", 5},
		{"SKELP", 5},
		{"SKELO", 5},
		{"SKELN", 5},
		{"SKELM", 5},
		{"SKELL", 5},
		"_see"
	}
}
MT_UNDEAD = createMobjType(mtype)

