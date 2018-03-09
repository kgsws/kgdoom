-- kgsws' Lua Doom exports
-- Doom monsters

a.CyberAttack =
function(mobj)
	local an
	local sl
	an, sl = mobj.AttackAim()
	mobj.angle = an
	mobj.SpawnMissile(MT_ROCKET, an, sl)
end

a.CyberDie =
function(mobj)
	if globalThingsIterator(PIT_BossDeath, mobj.info) then
		return
	end
	if game.map == "E4M6" then
		sectorTagIterator(666, PIT_BossOpen)
	elseif game.map == "E2M8" then
		game.DoomExit()
	end
end

-- MT_CYBORG
mtype = {
	seeSound = "dscybsit",
	painSound = "dsdmpain",
	activeSound = "dsdmact",
	deathSound = "dscybdth",
	ednum = 16,
	health = 4000,
	radius = 40,
	height = 110,
	mass = 1000,
	speed = 16,
	reactionTime = 8,
	painChance = 20,
	shootz = 32,
	species = 7,
	damageScale = {0},
	__Monster = true,
	__noRadiusDamage = true,
	__fullVolume = true,
	_spawn = {
		{"CYBRA", 10, a.Look},
		{"CYBRB", 10, a.Look},
		"loop"
	},
	_see = {
		{"CYBRA", 3, a.Hoof},
		{"CYBRA", 3, a.Chase},
		{"CYBRB", 3, a.Chase},
		{"CYBRB", 3, a.Chase},
		{"CYBRC", 3, a.Chase},
		{"CYBRC", 3, a.Chase},
		{"CYBRD", 3, a.Metal},
		{"CYBRD", 3, a.Chase},
		"loop"
	},
	_missile = {
		{"CYBRE", 6, a.FaceTarget},
		{"*CYBRF", 12, a.CyberAttack},
		{"CYBRE", 12, a.FaceTarget},
		{"*CYBRF", 12, a.CyberAttack},
		{"CYBRE", 12, a.FaceTarget},
		{"*CYBRF", 12, a.CyberAttack},
		"_see"
	},
	_pain = {
		{"CYBRG", 10, a.SoundPain},
		"_see"
	},
	_death = {
		{"CYBRH", 10},
		{"CYBRI", 10, a.SoundDeath},
		{"CYBRJ", 10},
		{"CYBRK", 10},
		{"CYBRL", 10},
		{"CYBRM", 10, a.Fall},
		{"CYBRN", 10},
		{"CYBRO", 10},
		{"CYBRP", 30},
		{"CYBRP", -1, a.CyberDie}
	}
}
MT_CYBORG = createMobjType(mtype)

