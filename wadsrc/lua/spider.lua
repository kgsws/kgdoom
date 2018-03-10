-- kgsws' Lua Doom exports
-- Doom monsters

-- uses a.SPosAttack

a.SpidRefire =
function(mobj)
	if doomRandom() >= 10 then
		if not mobj.CheckSight() or mobj.target.health <= 0 then
			mobj._see()
		end
	end
end

a.SpidDie =
function(mobj)
	if globalThingsIterator(PIT_BossDeath, mobj.info) then
		return
	end
	if game.map == "E3M8" then
		game.DoomExit()
	elseif game.map == "E4M8" then
		sectorTagIterator(666, PIT_BossLower)
	end
end

-- MT_SPIDER
mtype = {
	seeSound = "dsspisit",
	painSound = "dsdmpain",
	attackSound = "dsshotgn",
	activeSound = "dsdmact",
	deathSound = "dsspidth",
	ednum = 7,
	health = 3000,
	radius = 128,
	height = 100,
	mass = 1000,
	speed = 12,
	reactionTime = 8,
	painChance = 40,
	shootz = 58,
	damageScale = {0},
	__Monster = true,
	__noRadiusDamage = true,
	__fullVolume = true,
	_spawn = {
		{"SPIDA", 10, a.Look},
		{"SPIDB", 10, a.Look},
		"loop"
	},
	_see = {
		{"SPIDA", 3, a.Metal},
		{"SPIDA", 3, a.Chase},
		{"SPIDB", 3, a.Chase},
		{"SPIDB", 3, a.Chase},
		{"SPIDC", 3, a.Metal},
		{"SPIDC", 3, a.Chase},
		{"SPIDD", 3, a.Chase},
		{"SPIDD", 3, a.Chase},
		{"SPIDE", 3, a.Metal},
		{"SPIDE", 3, a.Chase},
		{"SPIDF", 3, a.Chase},
		{"SPIDF", 3, a.Chase},
		"loop"
	},
	_missile = {
		{"SPIDA", 20, a.FaceTarget},
		{"*SPIDG", 4, a.SPosAttack},
		{"*SPIDH", 4, a.SPosAttack},
		{"*SPIDH", 1, a.SpidRefire},
		"_missile+1"
	},
	_pain = {
		{"SPIDI", 3},
		{"SPIDI", 3, a.SoundPain},
		"_see"
	},
	_death = {
		{"SPIDJ", 20, a.SoundDeath},
		{"SPIDK", 10, a.Fall},
		{"SPIDL", 10},
		{"SPIDM", 10},
		{"SPIDN", 10},
		{"SPIDO", 10},
		{"SPIDP", 10},
		{"SPIDQ", 10},
		{"SPIDR", 10},
		{"SPIDS", 30},
		{"SPIDS", -1, a.SpidDie}
	},
	_crush = {
		{"POL5A0", -1, a.Crushed}
	}
}
MT_SPIDER = createMobjType(mtype)

