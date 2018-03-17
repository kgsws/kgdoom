-- kgsws' Lua Doom exports
-- Doom monsters

a.PainSkull =
function(mobj)
	if game.map == "MAP33" then
		-- Doom2 finale map
		mobj.SoundBody("dssklatk")
		return
	end
	local x
	local y
	local d
	local mo
	d = ((mobj.radius + MT_SKULL.radius) * 1.5 + 4)
	x = mobj.x + finecosine[mobj.angle] * d
	y = mobj.y + finesine[mobj.angle] * d
	mo = spawnMobj(MT_SKULL, x, y, mobj.z + 8)
	if not mo.CheckPosition() then
		mo.Damage(mo.health, 0)
	else
		mo.target = mobj.target
		a.SkullAttack(mo)
	end
end

a.PainAttack =
function(mobj)
	a.FaceTarget(mobj)
	a.PainSkull(mobj)
end

a.PainDie =
function(mobj)
	mobj.angle = mobj.angle + 2048
	a.PainSkull(mobj)
	mobj.angle = mobj.angle + 2048
	a.PainSkull(mobj)
	mobj.angle = mobj.angle + 2048
	a.PainSkull(mobj)
end

-- MT_PAIN
mtype = {
	seeSound = "dspesit",
	painSound = "dspepain",
	activeSound = "dsdmact",
	deathSound = "dspedth",
	ednum = 71,
	health = 400,
	radius = 31,
	height = 56,
	mass = 400,
	speed = 8,
	reactionTime = 8,
	painChance = 128,
	shootz = 32,
	pass = 1,
	damageScale = {0},
	__Monster = true,
	__float = true,
	__noGravity = true,
	_spawn = {
		{"PAINA", 10, a.Look},
		"loop"
	},
	_see = {
		{"PAINA", 3, a.Chase},
		{"PAINA", 3, a.Chase},
		{"PAINB", 3, a.Chase},
		{"PAINB", 3, a.Chase},
		{"PAINC", 3, a.Chase},
		{"PAINC", 3, a.Chase},
		"loop"
	},
	_missile = {
		{"PAIND", 5, a.FaceTarget},
		{"PAINE", 5, a.FaceTarget},
		{"*PAINF", 5, a.PainAttack},
		"_see"
	},
	_pain = {
		{"PAING", 6},
		{"PAING", 6, a.SoundPain},
		"_see"
	},
	_death = {
		{"PAINH", 8},
		{"PAINI", 8, a.SoundDeath},
		{"PAINJ", 8},
		{"PAINK", 8},
		{"PAINL", 8, a.PainDie},
		{"PAINM", 8}
	},
	_raise = {
		{"PAINM", 8},
		{"PAINL", 8},
		{"PAINK", 8},
		{"PAINJ", 8},
		{"PAINI", 8},
		{"PAINH", 8},
		"_see"
	},
	_crush = {
		{"POL5A0", -1, a.Crushed}
	}
}
MT_PAIN = createMobjType(mtype)

