-- kgsws' Lua Doom exports
-- decoration definitions

a.BarelExplode =
function(mobj)
	mobj.RadiusDamage(128, 128, 0, mobj.attacker, true)
end

-- MT_BARREL
mtype = {
	deathSound = "dsbarexp",
	ednum = 2035,
	radius = 10,
	height = 42,
	health = 20,
	mass = 100,
	__solid = true,
	__shootable = true,
	__noblood = true,
	_spawn = {
		{"BAR1A", 6},
		{"BAR1B", 6},
		"loop"
	},
	_death =
	{
		{"*BEXPA", 5},
		{"*BEXPB", 5, a.SoundDeath},
		{"*BEXPC", 5},
		{"*BEXPD", 1, a.BarelExplode},
		{"*BEXPE", 10}
	}
}
createMobjType(mtype)

-- MT_MISC29
mtype = {
	ednum = 85,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"*TLMPA", 4},
		{"*TLMPB", 4},
		{"*TLMPC", 4},
		{"*TLMPD", 4},
		"loop"
	}
}
createMobjType(mtype)

-- MT_MISC30
mtype = {
	ednum = 86,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"*TLP2A", 4},
		{"*TLP2B", 4},
		{"*TLP2C", 4},
		{"*TLP2D", 4},
		"loop"
	}
}
createMobjType(mtype)

-- MT_MISC31
mtype = {
	ednum = 2028,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"*COLUA", -1}
	}
}
createMobjType(mtype)

-- MT_MISC32
mtype = {
	ednum = 30,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"COL1A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC33
mtype = {
	ednum = 31,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"COL2A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC34
mtype = {
	ednum = 32,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"COL3A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC35
mtype = {
	ednum = 33,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"COL4A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC36
mtype = {
	ednum = 37,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"COL6A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC37
mtype = {
	ednum = 36,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"COL5A", 14},
		{"COL5B", 14},
		"loop"
	}
}
createMobjType(mtype)

-- MT_MISC38
mtype = {
	ednum = 41,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"*CEYEA", 6},
		{"*CEYEB", 6},
		{"*CEYEC", 6},
		{"*CEYEB", 6},
		"loop"
	}
}
createMobjType(mtype)

-- MT_MISC39
mtype = {
	ednum = 42,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"*FSKUA", 6},
		{"*FSKUB", 6},
		{"*FSKUC", 6},
		"loop"
	}
}
createMobjType(mtype)

-- MT_MISC40
mtype = {
	ednum = 43,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"TRE1A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC41
mtype = {
	ednum = 44,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"*TBLUA", 4},
		{"*TBLUB", 4},
		{"*TBLUC", 4},
		{"*TBLUD", 4},
		"loop"
	}
}
createMobjType(mtype)

-- MT_MISC42
mtype = {
	ednum = 45,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"*TGRNA", 4},
		{"*TGRNB", 4},
		{"*TGRNC", 4},
		{"*TGRND", 4},
		"loop"
	}
}
createMobjType(mtype)

-- MT_MISC43
mtype = {
	ednum = 46,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"*TREDA", 4},
		{"*TREDB", 4},
		{"*TREDC", 4},
		{"*TREDD", 4},
		"loop"
	}
}
createMobjType(mtype)

-- MT_MISC44
mtype = {
	ednum = 55,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"*SMBTA", 4},
		{"*SMBTB", 4},
		{"*SMBTC", 4},
		{"*SMBTD", 4},
		"loop"
	}
}
createMobjType(mtype)

-- MT_MISC45
mtype = {
	ednum = 56,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"*SMGTA", 4},
		{"*SMGTB", 4},
		{"*SMGTC", 4},
		{"*SMGTD", 4},
		"loop"
	}
}
createMobjType(mtype)

-- MT_MISC46
mtype = {
	ednum = 57,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"*SMRTA", 4},
		{"*SMRTB", 4},
		{"*SMRTC", 4},
		{"*SMRTD", 4},
		"loop"
	}
}
createMobjType(mtype)

-- MT_MISC47
mtype = {
	ednum = 47,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"SMITA", -1}
	}
}
createMobjType(mtype)

-- MT_MISC48
mtype = {
	ednum = 48,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"ELECA", -1}
	}
}
createMobjType(mtype)

-- MT_MISC49
mtype = {
	ednum = 34,
	radius = 16,
	height = 16,
	_spawn = {
		{"*CANDA", -1}
	}
}
createMobjType(mtype)

-- MT_MISC50
mtype = {
	ednum = 35,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"*CBRAA", -1}
	}
}
createMobjType(mtype)

-- MT_MISC51
mtype = {
	ednum = 49,
	radius = 16,
	height = 68,
	__solid = true,
	__spawnCeiling = true,
	__noGravity = true,
	_spawn = {
		{"GOR1A", 10},
		{"GOR1B", 15},
		{"GOR1C", 8},
		{"GOR1B", 6},
		"loop"
	}
}
createMobjType(mtype)

-- MT_MISC52
mtype = {
	ednum = 50,
	radius = 16,
	height = 84,
	__solid = true,
	__spawnCeiling = true,
	__noGravity = true,
	_spawn = {
		{"GOR2A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC53
mtype = {
	ednum = 51,
	radius = 16,
	height = 84,
	__solid = true,
	__spawnCeiling = true,
	__noGravity = true,
	_spawn = {
		{"GOR3A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC54
mtype = {
	ednum = 52,
	radius = 16,
	height = 68,
	__solid = true,
	__spawnCeiling = true,
	__noGravity = true,
	_spawn = {
		{"GOR4A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC55
mtype = {
	ednum = 53,
	radius = 16,
	height = 52,
	__solid = true,
	__spawnCeiling = true,
	__noGravity = true,
	_spawn = {
		{"GOR5A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC56
mtype = {
	ednum = 59,
	radius = 16,
	height = 84,
	__spawnCeiling = true,
	__noGravity = true,
	_spawn = {
		{"GOR2A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC57
mtype = {
	ednum = 60,
	radius = 16,
	height = 68,
	__spawnCeiling = true,
	__noGravity = true,
	_spawn = {
		{"GOR4A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC58
mtype = {
	ednum = 61,
	radius = 16,
	height = 84,
	__spawnCeiling = true,
	__noGravity = true,
	_spawn = {
		{"GOR3A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC59
mtype = {
	ednum = 62,
	radius = 16,
	height = 68,
	__spawnCeiling = true,
	__noGravity = true,
	_spawn = {
		{"GOR5A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC60
mtype = {
	ednum = 63,
	radius = 16,
	height = 68,
	__spawnCeiling = true,
	__noGravity = true,
	_spawn = {
		{"GOR1A", 10},
		{"GOR1B", 15},
		{"GOR1C", 8},
		{"GOR1B", 6},
		"loop"
	}
}
createMobjType(mtype)

-- MT_MISC61
mtype = {
	ednum = 22,
	radius = 20,
	height = 16,
	_spawn = {
		{"HEADL", -1}
	}
}
createMobjType(mtype)

-- MT_MISC62
mtype = {
	ednum = 15,
	radius = 20,
	height = 16,
	_spawn = {
		{"PLAYN", -1}
	}
}
createMobjType(mtype)

-- MT_MISC63
mtype = {
	ednum = 18,
	radius = 20,
	height = 16,
	_spawn = {
		{"POSSL", -1}
	}
}
createMobjType(mtype)

-- MT_MISC64
mtype = {
	ednum = 21,
	radius = 20,
	height = 16,
	_spawn = {
		{"SARGN", -1}
	}
}
createMobjType(mtype)

-- MT_MISC65 -- (23) useless and skipped

-- MT_MISC66
mtype = {
	ednum = 20,
	radius = 20,
	height = 16,
	_spawn = {
		{"TROOM", -1}
	}
}
createMobjType(mtype)

-- MT_MISC67
mtype = {
	ednum = 19,
	radius = 20,
	height = 16,
	_spawn = {
		{"SPOSL", -1}
	}
}
createMobjType(mtype)

-- MT_MISC68
mtype = {
	ednum = 10,
	radius = 20,
	height = 16,
	_spawn = {
		{"PLAYW", -1}
	}
}
createMobjType(mtype)

-- MT_MISC69
mtype = {
	ednum = 12,
	radius = 20,
	height = 16,
	_spawn = {
		{"PLAYW", -1}
	}
}
createMobjType(mtype)

-- MT_MISC70
mtype = {
	ednum = 28,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"POL2A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC71
mtype = {
	ednum = 24,
	radius = 20,
	height = 16,
	_spawn = {
		{"POL5A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC72
mtype = {
	ednum = 27,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"POL4A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC73
mtype = {
	ednum = 29,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"*POL3A", 6},
		{"*POL3B", 6},
		"loop"
	}
}
createMobjType(mtype)

-- MT_MISC74
mtype = {
	ednum = 25,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"POL1A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC75
mtype = {
	ednum = 26,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"POL6A", 6},
		{"POL6B", 8},
		"loop"
	}
}
createMobjType(mtype)

-- MT_MISC76
mtype = {
	ednum = 54,
	radius = 32,
	height = 16,
	__solid = true,
	_spawn = {
		{"TRE2A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC77
mtype = {
	ednum = 70,
	radius = 16,
	height = 16,
	__solid = true,
	_spawn = {
		{"*FCANA", 4},
		{"*FCANB", 4},
		{"*FCANC", 4},
		"loop"
	}
}
createMobjType(mtype)

-- MT_MISC78
mtype = {
	ednum = 73,
	radius = 16,
	height = 88,
	__solid = true,
	__spawnCeiling = true,
	__noGravity = true,
	_spawn = {
		{"HDB1A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC79
mtype = {
	ednum = 74,
	radius = 16,
	height = 88,
	__solid = true,
	__spawnCeiling = true,
	__noGravity = true,
	_spawn = {
		{"HDB2A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC80
mtype = {
	ednum = 75,
	radius = 16,
	height = 64,
	__solid = true,
	__spawnCeiling = true,
	__noGravity = true,
	_spawn = {
		{"HDB3A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC81
mtype = {
	ednum = 76,
	radius = 16,
	height = 64,
	__solid = true,
	__spawnCeiling = true,
	__noGravity = true,
	_spawn = {
		{"HDB4A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC82
mtype = {
	ednum = 77,
	radius = 16,
	height = 64,
	__solid = true,
	__spawnCeiling = true,
	__noGravity = true,
	_spawn = {
		{"HDB5A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC83
mtype = {
	ednum = 78,
	radius = 16,
	height = 64,
	__solid = true,
	__spawnCeiling = true,
	__noGravity = true,
	_spawn = {
		{"HDB6A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC84
mtype = {
	ednum = 79,
	radius = 20,
	height = 16,
	__noBlockmap = true,
	_spawn = {
		{"POB1A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC85
mtype = {
	ednum = 80,
	radius = 20,
	height = 16,
	__noBlockmap = true,
	_spawn = {
		{"POB2A", -1}
	}
}
createMobjType(mtype)

-- MT_MISC86
mtype = {
	ednum = 81,
	radius = 20,
	height = 16,
	__noBlockmap = true,
	_spawn = {
		{"BRS1A", -1}
	}
}
createMobjType(mtype)

