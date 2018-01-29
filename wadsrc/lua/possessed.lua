-- kgsws' Lua Doom exports
-- Doom monsters

a.PosScream =
function(mobj)
	mobj.SoundBody("dspodth1", "dspodth2", "dspodth3")
end

a.PosSit =
function(mobj)
	mobj.SoundBody("dsposit1", "dsposit2", "dsposit3")
end

a.PosDrop =
function(mobj)
	local mo
	mo = spawnMobj(MT_CLIP, mobj.x, mobj.y, mobj.z)
	mo.flags = mo.flags | mf.dropped
end

a.PosAttack =
function(mobj)
	local an
	local sl
	a.SoundAttack(mobj)
	an, sl = mobj.AttackAim(true)
	mobj.angle = an
	an = an + (doomRandom() - doomRandom()) * 2
	mobj.LineAttack(MT_PUFF, doomRandom(1, 5) * 3, an, sl)
end

a.SPosAttack =
function(mobj)
	local an
	local sl
	a.SoundAttack(mobj)
	an, sl = mobj.AttackAim(true)
	mobj.angle = an
	for i=0,2 do
		mobj.LineAttack(MT_PUFF, doomRandom(1, 5) * 3, an + (doomRandom() - doomRandom()) * 2, sl)
	end
end

a.SPosDrop =
function(mobj)
	local mo
	mo = spawnMobj(MT_SHOTGUN, mobj.x, mobj.y, mobj.z)
	mo.flags = mo.flags | mf.dropped
end

a.CPosRefire =
function(mobj)
	if doomRandom() >= 40 then
		if not mobj.CheckSight() or mobj.target.health <= 0 then
			mobj._see(1)
		end
	end
end

a.CPosDrop =
function(mobj)
	local mo
	mo = spawnMobj(MT_CHAINGUN, mobj.x, mobj.y, mobj.z)
	mo.flags = mo.flags | mf.dropped
end


-- MT_POSSESSED
mtype = {
	attackSound = "dspistol",
	painSound = "dspopain",
	activeSound = "dsposact",
	xdeathSound = "dsslop",
	ednum = 3004,
	health = 20,
	radius = 20,
	height = 56,
	mass = 100,
	speed = 8,
	reactionTime = 8,
	painChance = 200,
	shootz = 36,
	damageScale = {0},
	flags = mf.Monster,
	_spawn = {
		{"POSSA", 10, a.Look},
		{"POSSB", 10, a.Look},
		"loop"
	},
	_see = {
		{"POSSA", 0, a.PosSit},
		{"POSSA", 4, a.Chase},
		{"POSSA", 4, a.Chase},
		{"POSSB", 4, a.Chase},
		{"POSSB", 4, a.Chase},
		{"POSSC", 4, a.Chase},
		{"POSSC", 4, a.Chase},
		{"POSSD", 4, a.Chase},
		{"POSSD", 4, a.Chase},
		"_see+1"
	},
	_missile = {
		{"POSSE", 10, a.FaceTarget},
		{"*POSSF", 8, a.PosAttack},
		{"POSSE", 8},
		"_see+1"
	},
	_pain = {
		{"POSSG", 3},
		{"POSSG", 3, a.SoundPain},
		"_see+1"
	},
	_death = {
		{"POSSH", 5, a.PosDrop},
		{"POSSI", 5, a.PosScream},
		{"POSSJ", 5, a.Fall},
		{"POSSK", 5},
		{"POSSL", -1}
	},
	_xdeath = {
		{"POSSM", 5, a.PosDrop},
		{"POSSN", 5, a.SoundXDeath},
		{"POSSO", 5, a.Fall},
		{"POSSP", 5},
		{"POSSQ", 5},
		{"POSSR", 5},
		{"POSSS", 5},
		{"POSST", 5},
		{"POSSU", -1}
	},
	_raise = {
		{"POSSK", 5},
		{"POSSJ", 5},
		{"POSSI", 5},
		{"POSSH", 5},
		"_see+1"
	}
}
MT_POSSESSED = createMobjType(mtype)

-- MT_SHOTGUY
mtype = {
	attackSound = "dsshotgn",
	painSound = "dspopain",
	activeSound = "dsposact",
	xdeathSound = "dsslop",
	ednum = 9,
	health = 30,
	radius = 20,
	height = 56,
	mass = 100,
	speed = 8,
	reactionTime = 8,
	painChance = 170,
	shootz = 36,
	damageScale = {0},
	flags = mf.Monster,
	_spawn = {
		{"SPOSA", 10, a.Look},
		{"SPOSB", 10, a.Look},
		"loop"
	},
	_see = {
		{"SPOSA", 0, a.PosSit},
		{"SPOSA", 3, a.Chase},
		{"SPOSA", 3, a.Chase},
		{"SPOSB", 3, a.Chase},
		{"SPOSB", 3, a.Chase},
		{"SPOSC", 3, a.Chase},
		{"SPOSC", 3, a.Chase},
		{"SPOSD", 3, a.Chase},
		{"SPOSD", 3, a.Chase},
		"_see+1"
	},
	_missile = {
		{"SPOSE", 10, a.FaceTarget},
		{"*SPOSF", 10, a.SPosAttack},
		{"SPOSE", 10},
		"_see+1"
	},
	_pain = {
		{"SPOSG", 3},
		{"SPOSG", 3, a.SoundPain},
		"_see+1"
	},
	_death = {
		{"SPOSH", 5, a.SPosDrop},
		{"SPOSI", 5, a.PosScream},
		{"SPOSJ", 5, a.Fall},
		{"SPOSK", 5},
		{"SPOSL", -1}
	},
	_xdeath = {
		{"SPOSM", 5, a.SPosDrop},
		{"SPOSN", 5, a.SoundXDeath},
		{"SPOSO", 5, a.Fall},
		{"SPOSP", 5},
		{"SPOSQ", 5},
		{"SPOSR", 5},
		{"SPOSS", 5},
		{"SPOST", 5},
		{"SPOSU", -1}
	},
	_raise = {
		{"SPOSL", 5},
		{"SPOSK", 5},
		{"SPOSJ", 5},
		{"SPOSI", 5},
		{"SPOSH", 5},
		"_see+1"
	}
}
MT_SHOTGUY = createMobjType(mtype)

-- MT_CHAINGUY
mtype = {
	attackSound = "dsshotgn",
	painSound = "dspopain",
	activeSound = "dsposact",
	xdeathSound = "dsslop",
	ednum = 65,
	health = 70,
	radius = 20,
	height = 56,
	mass = 100,
	speed = 8,
	reactionTime = 8,
	painChance = 170,
	shootz = 36,
	damageScale = {0},
	flags = mf.Monster,
	_spawn = {
		{"CPOSA", 10, a.Look},
		{"CPOSB", 10, a.Look},
		"loop"
	},
	_see = {
		{"CPOSA", 0, a.PosSit},
		{"CPOSA", 3, a.Chase},
		{"CPOSA", 3, a.Chase},
		{"CPOSB", 3, a.Chase},
		{"CPOSB", 3, a.Chase},
		{"CPOSC", 3, a.Chase},
		{"CPOSC", 3, a.Chase},
		{"CPOSD", 3, a.Chase},
		{"CPOSD", 3, a.Chase},
		"_see+1"
	},
	_missile = {
		{"CPOSE", 10, a.FaceTarget},
		{"*CPOSF", 4, a.PosAttack},
		{"*CPOSE", 4, a.PosAttack},
		{"CPOSF", 1, a.CPosRefire},
		"_missile+1"
	},
	_pain = {
		{"CPOSG", 3},
		{"CPOSG", 3, a.SoundPain},
		"_see+1"
	},
	_death = {
		{"CPOSH", 5, a.CPosDrop},
		{"CPOSI", 5, a.PosScream},
		{"CPOSJ", 5, a.Fall},
		{"CPOSK", 5},
		{"CPOSL", 5},
		{"CPOSM", 5},
		{"CPOSN", -1}
	},
	_xdeath = {
		{"CPOSO", 5, a.CPosDrop},
		{"CPOSP", 5, a.SoundXDeath},
		{"CPOSQ", 5, a.Fall},
		{"CPOSR", 5},
		{"CPOSS", 5},
		{"CPOST", -1}
	},
	_raise = {
		{"CPOSN", 5},
		{"CPOSM", 5},
		{"CPOSL", 5},
		{"CPOSK", 5},
		{"CPOSJ", 5},
		{"CPOSI", 5},
		{"CPOSH", 5},
		"_see+1"
	}
}
MT_CHAINGUY = createMobjType(mtype)

-- MT_WOLFSS
mtype = {
	seeSound = "dssssit",
	painSound = "dspopain",
	activeSound = "dsposact",
	attackSound = "dsshotgn",
	deathSound = "dsssdth",
	xdeathSound = "dsslop",
	ednum = 84,
	health = 50,
	radius = 20,
	height = 56,
	mass = 100,
	speed = 8,
	reactionTime = 8,
	painChance = 170,
	shootz = 36,
	damageScale = {0},
	flags = mf.Monster,
	_spawn = {
		{"SSWVA", 10, a.Look},
		{"SSWVB", 10, a.Look},
		"loop"
	},
	_see = {
		{"SSWVA", 3, a.Chase},
		{"SSWVA", 3, a.Chase},
		{"SSWVB", 3, a.Chase},
		{"SSWVB", 3, a.Chase},
		{"SSWVC", 3, a.Chase},
		{"SSWVC", 3, a.Chase},
		{"SSWVD", 3, a.Chase},
		{"SSWVD", 3, a.Chase},
		"loop"
	},
	_missile = {
		{"SSWVE", 10, a.FaceTarget},
		{"SSWVF", 10, a.FaceTarget},
		{"*SSWVG", 4, a.PosAttack},
		{"SSWVF", 6, a.FaceTarget},
		{"*SSWVG", 4, a.PosAttack},
		{"SSWVF", 1, a.CPosRefire},
		"_missile+1"
	},
	_pain = {
		{"SSWVH", 3},
		{"SSWVH", 3, a.SoundPain},
		"_see"
	},
	_death = {
		{"SSWVI", 5, a.PosDrop},
		{"SSWVJ", 5, a.SoundDeath},
		{"SSWVK", 5, a.Fall},
		{"SSWVL", 5},
		{"SSWVM", -1}
	},
	_xdeath = {
		{"SSWVN", 5, a.PosDrop},
		{"SSWVO", 5, a.SoundXDeath},
		{"SSWVP", 5, a.Fall},
		{"SSWVQ", 5},
		{"SSWVR", 5},
		{"SSWVS", 5},
		{"SSWVT", 5},
		{"SSWVU", 5},
		{"SSWVV", -1}
	},
	_raise = {
		{"SSWVM", 5},
		{"SSWVL", 5},
		{"SSWVK", 5},
		{"SSWVJ", 5},
		{"SSWVI", 5},
		"_see"
	}
}
MT_WOLFSS = createMobjType(mtype)

