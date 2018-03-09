-- kgsws' Lua Doom exports
-- Doom monsters

a.SargTarget =
function(mobj)
	a.FaceTarget(mobj)
	a.SoundAttack(mobj)
end

a.SargAttack =
function(mobj)
	if mobj.MeleeRange() then
		a.FaceTarget(mobj)
		mobj.target.Damage(doomRandom(1, 10) * 4, 0, mobj, mobj)
	end
end

-- MT_SERGEANT
mtype = {
	seeSound = "dssgtsit",
	attackSound = "dssgtatk",
	painSound = "dsdmpain",
	activeSound = "dsdmact",
	deathSound = "dssgtdth",
	ednum = 3002,
	health = 150,
	radius = 30,
	height = 56,
	mass = 400,
	speed = 10,
	reactionTime = 8,
	painChance = 180,
	shootz = 36,
	damageScale = {0},
	__Monster = true,
	_spawn = {
		{"SARGA", 10, a.Look},
		{"SARGB", 10, a.Look},
		"loop"
	},
	_see = {
		{"SARGA", 2, a.Chase},
		{"SARGA", 2, a.Chase},
		{"SARGB", 2, a.Chase},
		{"SARGB", 2, a.Chase},
		{"SARGC", 2, a.Chase},
		{"SARGC", 2, a.Chase},
		{"SARGD", 2, a.Chase},
		{"SARGD", 2, a.Chase},
		"loop"
	},
	_melee = {
		{"SARGE", 8, a.SargTarget},
		{"SARGF", 8, a.FaceTarget},
		{"SARGG", 8, a.SargAttack},
		"_see"
	},
	_pain = {
		{"SARGH", 2},
		{"SARGH", 2, a.SoundPain},
		"_see"
	},
	_death = {
		{"SARGI", 8},
		{"SARGJ", 8, a.SoundDeath},
		{"SARGK", 4},
		{"SARGL", 4, a.Fall},
		{"SARGM", 4},
		{"SARGN", -1}
	},
	_raise = {
		{"SARGN", 5},
		{"SARGM", 5},
		{"SARGL", 5},
		{"SARGK", 5},
		{"SARGJ", 5},
		{"SARGI", 5},
		"_see"
	}
}
MT_SERGEANT = createMobjType(mtype)

-- MT_SHADOWS
-- only modify what's different
mtype.ednum = 58
mtype.__shadow = true
MT_SHADOWS = createMobjType(mtype)

