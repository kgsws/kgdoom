-- kgsws' Lua Doom exports
-- Doom monsters

a.BgScream =
function(mobj)
	mobj.SoundBody("dsbgdth1", "dsbgdth2")
end

a.BgSit =
function(mobj)
	mobj.SoundBody("dsbgsit1", "dsbgsit2")
end

a.TroopAttack =
function(mobj)
	if mobj.MeleeRange() then
		a.FaceTarget(mobj)
		a.SoundAttack(mobj)
		mobj.target.Damage(doomRandom(1, 8) * 3, 0, mobj, mobj)
	else
		local an
		local sl
		an, sl = mobj.AttackAim()
		mobj.angle = an
		mobj.SpawnMissile(MT_TROOPSHOT, an, sl)
	end
end

-- MT_TROOPSHOT
mtype = {
	seeSound = "dsfirsht",
	deathSound = "dsfirxpl",
	speed = 10,
	radius = 6,
	height = 8,
	damage = -3,
	flags = mf.Projectile,
	_spawn = {
		{"*BAL1A", 4},
		{"*BAL1B", 4},
		"loop"
	},
	_death = {
		{"*BAL1C", 6},
		{"*BAL1D", 6},
		{"*BAL1E", 6}
	}
}
MT_TROOPSHOT = createMobjType(mtype)

-- MT_TROOP
mtype = {
	attackSound = "dsclaw",
	painSound = "dspopain",
	activeSound = "dsbgact",
	xdeathSound = "dsslop",
	ednum = 3001,
	health = 60,
	radius = 20,
	height = 56,
	mass = 100,
	speed = 8,
	reactionTime = 8,
	painChance = 200,
	shootz = 32,
	species = 3,
	damageScale = {0},
	flags = mf.Monster,
	_spawn = {
		{"TROOA", 10, a.Look},
		{"TROOB", 10, a.Look},
		"loop"
	},
	_see = {
		{"TROOA", 0, a.BgSit},
		{"TROOA", 3, a.Chase},
		{"TROOA", 3, a.Chase},
		{"TROOB", 3, a.Chase},
		{"TROOB", 3, a.Chase},
		{"TROOC", 3, a.Chase},
		{"TROOC", 3, a.Chase},
		{"TROOD", 3, a.Chase},
		{"TROOD", 3, a.Chase},
		"_see+1"
	},
	_melee = {
		{"TROOE", 6, a.FaceTarget},
		{"TROOF", 6, a.FaceTarget},
		{"TROOG", 6, a.TroopAttack},
		"_see+1"
	},
	_missile = {
		{"TROOE", 6, a.FaceTarget},
		{"TROOF", 6, a.FaceTarget},
		{"TROOG", 6, a.TroopAttack},
		"_see+1"
	},
	_pain = {
		{"TROOH", 2},
		{"TROOH", 2, a.SoundPain},
		"_see+1"
	},
	_death = {
		{"TROOI", 8},
		{"TROOJ", 8, a.BgScream},
		{"TROOK", 6, a.Fall},
		{"TROOL", 6},
		{"TROOM", -1}
	},
	_xdeath = {
		{"TROON", 5},
		{"TROOO", 5, a.SoundXDeath},
		{"TROOP", 5, a.Fall},
		{"TROOQ", 5},
		{"TROOR", 5},
		{"TROOS", 5},
		{"TROOT", 5},
		{"TROOU", -1}
	},
	_raise = {
		{"TROOM", 5},
		{"TROOL", 5},
		{"TROOK", 5},
		{"TROOJ", 5},
		{"TROOI", 5},
		"_see+1"
	}
}
MT_TROOP = createMobjType(mtype)

