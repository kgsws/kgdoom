-- kgsws' Lua Doom exports
-- Doom monsters

a.Gravity =
function(mobj)
	mobj.flags = mobj.flags | mf.noGravity
end

a.SkullAttack =
function(mobj)
	local an
	local sl
	local sp
	a.SoundAttack(mobj)
	mobj.flags = mobj.flags | mf.skullFly
	an, sl = mobj.AttackAim()
	if sl < 0 then
		sp = 1 + sl
	else
		sp = 1 - sl	
	end
	mobj.angle = an
	mobj.momz = sl * 20
	mobj.momx = finecosine[an] * 20 * sp
	mobj.momy = finesine[an] * 20 * sp
end

-- MT_SKULL
mtype = {
	painSound = "dsdmpain",
	attackSound = "dssklatk",
	activeSound = "dsdmact",
	deathSound = "dsfirxpl",
	ednum = 3006,
	health = 100,
	radius = 16,
	height = 56,
	mass = 50,
	speed = 8,
	reactionTime = 8,
	painChance = 256,
	damage = 3,
	damageScale = {0},
	flags = mf.Monster | mf.float | mf.noGravity,
	_spawn = {
		{"*SKULA", 10, a.Look},
		{"*SKULB", 10, a.Look},
		"loop"
	},
	_see = {
		{"*SKULA", 6, a.Chase},
		{"*SKULB", 6, a.Chase},
		"loop"
	},
	_missile = {
		{"*SKULC", 10, a.SkullAttack},
		{"*SKULD", 4},
		{"*SKULC", 4},
		{"*SKULD", 4},
		"_missile+1"
	},
	_pain = {
		{"*SKULE", 3},
		{"*SKULE", 3, a.SoundPain},
		"_see"
	},
	_death = {
		{"*SKULF", 8, a.Gravity},
		{"*SKULG", 8, a.SoundDeath},
		{"*SKULH", 8},
		{"*SKULI", 8, a.Fall},
		{"SKULJ", 8},
		{"SKULK", 8}
	}
}
MT_SKULL = createMobjType(mtype)

