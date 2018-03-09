-- kgsws' Lua Doom exports
-- Doom monsters

a.FatRaise =
function(mobj)
	a.FaceTarget(mobj)
	a.SoundAttack(mobj)
end

a.FatAttack1 =
function(mobj)
	local an
	local sl
	an, sl = mobj.AttackAim()
	mobj.SpawnMissile(MT_FATSHOT, an, sl)
	mobj.SpawnMissile(MT_FATSHOT, an + 256, sl)
	mobj.angle = mobj.angle + 256
end

a.FatAttack2 =
function(mobj)
	local an
	local sl
	an, sl = mobj.AttackAim()
	mobj.angle = an - 256
	mobj.SpawnMissile(MT_FATSHOT, an, sl)
	mobj.SpawnMissile(MT_FATSHOT, an - 256, sl)
end

a.FatAttack3 =
function(mobj)
	local an
	local sl
	an, sl = mobj.AttackAim()
	mobj.angle = an
	mobj.SpawnMissile(MT_FATSHOT, an - 128, sl)
	mobj.SpawnMissile(MT_FATSHOT, an + 128, sl)
end

a.FatDeath =
function(mobj)
	if game.map == "MAP07" and not globalThingsIterator(PIT_BossDeath, mobj.info) then
		sectorTagIterator(666, PIT_BossLower)
	end
end

-- MT_FATSHOT
mtype = {
	seeSound = "dsfirsht",
	deathSound = "dsfirxpl",
	speed = 20,
	radius = 6,
	height = 8,
	damage = -8,
	__Projectile = true,
	_spawn = {
		{"*MANFA", 4},
		{"*MANFB", 4},
		"loop"
	},
	_death = {
		{"*MISLB", 8},
		{"*MISLC", 6},
		{"*MISLD", 4}
	}
}
MT_FATSHOT = createMobjType(mtype)

-- MT_FATSO
mtype = {
	seeSound = "dsmansit",
	painSound = "dsmnpain",
	attackSound = "dsmanatk",
	deathSound = "dsmandth",
	ednum = 67,
	health = 600,
	radius = 48,
	height = 64,
	mass = 1000,
	speed = 8,
	reactionTime = 8,
	painChance = 80,
	shootz = 32,
	species = 2,
	damageScale = {0},
	__Monster = true,
	_spawn = {
		{"FATTA", 15, a.Look},
		{"FATTB", 15, a.Look},
		"loop"
	},
	_see = {
		{"FATTA", 4, a.Chase},
		{"FATTA", 4, a.Chase},
		{"FATTB", 4, a.Chase},
		{"FATTB", 4, a.Chase},
		{"FATTC", 4, a.Chase},
		{"FATTC", 4, a.Chase},
		{"FATTD", 4, a.Chase},
		{"FATTD", 4, a.Chase},
		{"FATTE", 4, a.Chase},
		{"FATTE", 4, a.Chase},
		{"FATTF", 4, a.Chase},
		{"FATTF", 4, a.Chase},
		"loop"
	},
	_missile = {
		{"FATTG", 20, a.FatRaise},
		{"*FATTH", 10, a.FatAttack1},
		{"FATTI", 5, a.FaceTarget},
		{"FATTG", 5, a.FaceTarget},
		{"*FATTH", 10, a.FatAttack2},
		{"FATTI", 5, a.FaceTarget},
		{"FATTG", 5, a.FaceTarget},
		{"*FATTH", 10, a.FatAttack3},
		{"FATTI", 5, a.FaceTarget},
		{"FATTG", 5, a.FaceTarget},
		"_see"
	},
	_pain = {
		{"FATTJ", 3},
		{"FATTJ", 3, a.SoundPain},
		"_see"
	},
	_death = {
		{"FATTK", 6},
		{"FATTL", 6, a.SoundDeath},
		{"FATTM", 6, a.Fall},
		{"FATTN", 6},
		{"FATTO", 6},
		{"FATTP", 6},
		{"FATTQ", 6},
		{"FATTR", 6},
		{"FATTS", 6},
		{"FATTT", -1, a.FatDeath}
	},
	_raise = {
		{"FATTR", 5},
		{"FATTQ", 5},
		{"FATTP", 5},
		{"FATTO", 5},
		{"FATTN", 5},
		{"FATTM", 5},
		{"FATTL", 5},
		{"FATTK", 5},
		"_see"
	}
}
MT_FATSO = createMobjType(mtype)

