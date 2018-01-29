-- kgsws' Lua Doom exports
-- Doom monsters

-- uses a.SpidRefire

a.BspiAttack =
function(mobj)
	local an
	local sl
	an, sl = mobj.AttackAim()
	mobj.angle = an
	mobj.SpawnMissile(MT_ARACHPLAZ, an, sl)
end

a.BspiMetal =
function(mobj)
	mobj.SoundPickup("dsbspwlk")
	a.Chase(mobj)
end

PIT_BspiLower =
function(sector)
	sector.GenericFloor(sector.floorheight + sector.GetShortestTexture(false), 1, 0, "-", "-", "dsstnmov")
end

a.BspiDeath =
function(mobj)
	if game.map == "MAP07" and not globalThingsIterator(PIT_BossDeath, mobj.info) then
		sectorTagIterator(667, PIT_BspiLower)
	end
end

-- MT_ARACHPLAZ
mtype = {
	seeSound = "dsplasma",
	deathSound = "dsfirxpl",
	speed = 25,
	radius = 13,
	height = 8,
	damage = -5,
	flags = mf.Projectile,
	_spawn = {
		{"*APLSA", 5},
		{"*APLSB", 5},
		"loop"
	},
	_death = {
		{"*APBXA", 5},
		{"*APBXB", 5},
		{"*APBXC", 5},
		{"*APBXD", 5},
		{"*APBXE", 5}
	}
}
MT_ARACHPLAZ = createMobjType(mtype)

-- MT_BABY
mtype = {
	seeSound = "dsbspsit",
	painSound = "dsdmpain",
	activeSound = "dsbspact",
	deathSound = "dsbspdth",
	ednum = 68,
	health = 500,
	radius = 64,
	height = 64,
	mass = 600,
	speed = 12,
	reactionTime = 8,
	painChance = 128,
	shootz = 32,
	species = 6,
	damageScale = {0},
	flags = mf.Monster,
	_spawn = {
		{"BSPIA", 10, a.Look},
		{"BSPIB", 10, a.Look},
		"loop"
	},
	_see = {
		{"BSPIA", 20},
		{"BSPIA", 3, a.BspiMetal},
		{"BSPIA", 3, a.Chase},
		{"BSPIB", 3, a.Chase},
		{"BSPIB", 3, a.Chase},
		{"BSPIC", 3, a.Chase},
		{"BSPIC", 3, a.Chase},
		{"BSPID", 3, a.BspiMetal},
		{"BSPID", 3, a.Chase},
		{"BSPIE", 3, a.Chase},
		{"BSPIE", 3, a.Chase},
		{"BSPIF", 3, a.Chase},
		{"BSPIF", 3, a.Chase},
		"_see+1"
	},
	_missile = {
		{"BSPIA", 20, a.FaceTarget},
		{"*BSPIG", 4, a.BspiAttack},
		{"*BSPIH", 4},
		{"*BSPIH", 1, a.SpidRefire},
		"_missile+1"
	},
	_pain = {
		{"BSPII", 3},
		{"BSPII", 3, a.SoundPain},
		"_see+1"
	},
	_death = {
		{"BSPIJ", 20, a.SoundDeath},
		{"BSPIK", 7, a.Fall},
		{"BSPIL", 7},
		{"BSPIM", 7},
		{"BSPIN", 7},
		{"BSPIO", 7},
		{"BSPIP", -1, a.BspiDeath}
	},
	_raise = {
		{"BSPIP", 5},
		{"BSPIO", 5},
		{"BSPIN", 5},
		{"BSPIM", 5},
		{"BSPIL", 5},
		{"BSPIK", 5},
		{"BSPIJ", 5},
		"_see+1"
	}
}
MT_BABY = createMobjType(mtype)

