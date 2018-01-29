-- kgsws' Lua Doom exports
-- Doom line actions

-- original Doom map format activation type table
doomLineType = {}
-- original Doom reactivation table
doomLineRe = {}

-- to catch any missed function
linefunc[0] =
function(mobj, line, side, act)
	mobj.Damage(true)
	print("line special " .. line.special .. " act " .. act)
	line.DoButton("dsswtchn", "dsswtchn")
	return true
end

