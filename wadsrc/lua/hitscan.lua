-- kgsws' Lua Doom exports
-- puff and blood

a.PuffStuff =
function(mobj)
	mobj.momz = 1
end

a.BloodStuff =
function(mobj)
	local source
	source = mobj.source
	mobj.__missile = true
	mobj.__noGravity = false
	mobj.momz = -0.5
	mobj.Thrust(0.25 + doomRandom() / 512, (doomRandom() - 128)*2)
	if source.info == MT_HEAD then
		mobj.translation = "BLOODMAP:0"
	elseif source.info == MT_BRUISER or source.info == MT_KNIGHT then
		mobj.translation = "BLOODMAP:1"
	end
end

-- MT_PUFF
mtype = {
	radius = 1,
	height = 12,
	pass = 3,
	__noBlockmap = true,
	__noGravity = true,
	__dropOff = true,
	__troughMobj = true,
	_spawn = {
		{"*PUFFA", 4, a.PuffStuff},
		{"PUFFB", 4},
		{"PUFFC", 4},
		{"PUFFD", 4}
	},
	_pain = {
		{"BLUDC", 8, a.BloodStuff},
		{"BLUDB", 8},
		{"BLUDA", 8}
	}
}
MT_PUFF = createMobjType(mtype)

-- MT_PUFF_MELEE (new type)
mtype = {
	radius = 1,
	height = 12,
	pass = 3,
	__noBlockmap = true,
	__noGravity = true,
	__dropOff = true,
	__troughMobj = true,
	_spawn = {
		{"PUFFC", 4, a.PuffStuff},
		{"PUFFD", 4}
	},
	_pain = {
		{"BLUDC", 8, a.BloodStuff},
		{"BLUDB", 8},
		{"BLUDA", 8}
	}
}
MT_PUFF_MELEE = createMobjType(mtype)

