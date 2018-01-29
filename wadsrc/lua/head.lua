-- kgsws' Lua Doom exports
-- Doom monsters

a.HeadAttack =
function(mobj)
	if mobj.MeleeRange() then
		a.FaceTarget(mobj)
		mobj.target.Damage(doomRandom(1, 6) * 10, 0, mobj, mobj)
	else
		local an
		local sl
		an, sl = mobj.AttackAim()
		mobj.angle = an
		mobj.SpawnMissile(MT_HEADSHOT, an, sl)
	end
end

-- MT_HEADSHOT
mtype = {
	seeSound = "dsfirsht",
	deathSound = "dsfirxpl",
	speed = 10,
	radius = 6,
	height = 8,
	damage = -5,
	flags = mf.Projectile,
	_spawn = {
		{"*BAL2A", 4},
		{"*BAL2B", 4},
		"loop"
	},
	_death = {
		{"*BAL2C", 6},
		{"*BAL2D", 6},
		{"*BAL2E", 6}
	}
}
MT_HEADSHOT = createMobjType(mtype)

-- MT_HEAD
mtype = {
	seeSound = "dscacsit",
	painSound = "dsdmpain",
	activeSound = "dsdmact",
	deathSound = "dscacdth",
	ednum = 3005,
	health = 400,
	radius = 31,
	height = 56,
	mass = 400,
	speed = 8,
	reactionTime = 8,
	painChance = 128,
	shootz = 32,
	species = 4,
	damageScale = {0},
	flags = mf.Monster | mf.float | mf.noGravity,
	_spawn = {
		{"HEADA", 10, a.Look},
		"loop"
	},
	_see = {
		{"HEADA", 3, a.Chase},
		"loop"
	},
	_missile = {
		{"HEADB", 5, a.FaceTarget},
		{"HEADC", 5, a.FaceTarget},
		{"*HEADD", 5, a.HeadAttack},
		"_see"
	},
	_pain = {
		{"HEADE", 3},
		{"HEADE", 3, a.SoundPain},
		{"HEADF", 6},
		"_see"
	},
	_death = {
		{"HEADG", 8},
		{"HEADH", 8, a.SoundDeath},
		{"HEADI", 8},
		{"HEADJ", 8},
		{"HEADK", 8, a.Fall},
		{"HEADL", -1}
	},
	_raise = {
		{"HEADL", 8},
		{"HEADK", 8},
		{"HEADJ", 8},
		{"HEADI", 8},
		{"HEADH", 8},
		{"HEADG", 8},
		"_see"
	}
}
MT_HEAD = createMobjType(mtype)

