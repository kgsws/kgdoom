-- kgsws' Lua Doom exports
-- Doom monsters

a.BruisAttack =
function(mobj)
	if mobj.MeleeRange() then
		a.FaceTarget(mobj)
		a.SoundAttack(mobj)
		mobj.target.Damage(doomRandom(1, 8) * 10, 0, mobj, mobj)
	else
		local an
		local sl
		an, sl = mobj.AttackAim()
		an = shadowAim(mobj, an)
		mobj.angle = an
		mobj.SpawnMissile(MT_BRUISERSHOT, an, sl)
	end
end

a.BruisDeath =
function(mobj)
	if game.map == "E1M8" and not globalThingsIterator(PIT_BossDeath, mobj.info) then
		sectorTagIterator(666, PIT_BossLower)
	end
end

-- MT_BRUISERSHOT
mtype = {
	seeSound = "dsfirsht",
	deathSound = "dsfirxpl",
	speed = 15,
	radius = 6,
	height = 8,
	damage = -8,
	__Projectile = true,
	_spawn = {
		{"*BAL7A", 4},
		{"*BAL7B", 4},
		"loop"
	},
	_death = {
		{"*BAL7C", 6},
		{"*BAL7D", 6},
		{"*BAL7E", 6}
	}
}
MT_BRUISERSHOT = createMobjType(mtype)

-- MT_BRUISER
mtype = {
	seeSound = "dsbrssit",
	painSound = "dsdmpain",
	attackSound = "dsclaw",
	activeSound = "dsdmact",
	deathSound = "dsbrsdth",
	ednum = 3003,
	health = 1000,
	radius = 24,
	height = 64,
	mass = 1000,
	speed = 8,
	reactionTime = 8,
	painChance = 50,
	shootz = 32,
	species = 5,
	damageScale = {0},
	__Monster = true,
	_spawn = {
		{"BOSSA", 10, a.Look},
		{"BOSSB", 10, a.Look},
		"loop"
	},
	_see = {
		{"BOSSA", 3, a.Chase},
		{"BOSSA", 3, a.Chase},
		{"BOSSB", 3, a.Chase},
		{"BOSSB", 3, a.Chase},
		{"BOSSC", 3, a.Chase},
		{"BOSSC", 3, a.Chase},
		{"BOSSD", 3, a.Chase},
		{"BOSSD", 3, a.Chase},
		"loop"
	},
	_melee = {
		{"BOSSE", 8, a.FaceTarget},
		{"BOSSF", 8, a.FaceTarget},
		{"BOSSG", 8, a.BruisAttack},
		"_see"
	},
	_missile = {
		{"BOSSE", 8, a.FaceTarget},
		{"BOSSF", 8, a.FaceTarget},
		{"BOSSG", 8, a.BruisAttack},
		"_see"
	},
	_pain = {
		{"BOSSH", 2},
		{"BOSSH", 2, a.SoundPain},
		"_see"
	},
	_death = {
		{"BOSSI", 8},
		{"BOSSJ", 8, a.SoundDeath},
		{"BOSSK", 8},
		{"BOSSL", 8, a.Fall},
		{"BOSSM", 8},
		{"BOSSN", 8},
		{"BOSSO", -1, a.BruisDeath}
	},
	_raise = {
		{"BOSSO", 8},
		{"BOSSN", 8},
		{"BOSSM", 8},
		{"BOSSL", 8},
		{"BOSSK", 8},
		{"BOSSJ", 8},
		{"BOSSI", 8},
		"_see"
	},
	_crush = {
		{"POL5A0", -1, a.Crushed}
	}

}
MT_BRUISER = createMobjType(mtype)

-- MT_KNIGHT
mtype = {
	seeSound = "dskntsit",
	painSound = "dsdmpain",
	attackSound = "dsclaw",
	activeSound = "dsdmact",
	deathSound = "dskntdth",
	ednum = 69,
	health = 500,
	radius = 24,
	height = 64,
	mass = 1000,
	speed = 8,
	reactionTime = 8,
	painChance = 50,
	shootz = 32,
	species = 5,
	damageScale = {0},
	__Monster = true,
	_spawn = {
		{"BOS2A", 10, a.Look},
		{"BOS2B", 10, a.Look},
		"loop"
	},
	_see = {
		{"BOS2A", 3, a.Chase},
		{"BOS2A", 3, a.Chase},
		{"BOS2B", 3, a.Chase},
		{"BOS2B", 3, a.Chase},
		{"BOS2C", 3, a.Chase},
		{"BOS2C", 3, a.Chase},
		{"BOS2D", 3, a.Chase},
		{"BOS2D", 3, a.Chase},
		"loop"
	},
	_melee = {
		{"BOS2E", 8, a.FaceTarget},
		{"BOS2F", 8, a.FaceTarget},
		{"BOS2G", 8, a.BruisAttack},
		"_see"
	},
	_missile = {
		{"BOS2E", 8, a.FaceTarget},
		{"BOS2F", 8, a.FaceTarget},
		{"BOS2G", 8, a.BruisAttack},
		"_see"
	},
	_pain = {
		{"BOS2H", 2},
		{"BOS2H", 2, a.SoundPain},
		"_see"
	},
	_death = {
		{"BOS2I", 8},
		{"BOS2J", 8, a.SoundDeath},
		{"BOS2K", 8},
		{"BOS2L", 8, a.Fall},
		{"BOS2M", 8},
		{"BOS2N", 8},
		{"BOS2O", -1}
	},
	_raise = {
		{"BOS2O", 8},
		{"BOS2N", 8},
		{"BOS2M", 8},
		{"BOS2L", 8},
		{"BOS2K", 8},
		{"BOS2J", 8},
		{"BOS2I", 8},
		"_see"
	},
	_crush = {
		{"POL5A0", -1, a.Crushed}
	}
}
MT_KNIGHT = createMobjType(mtype)

