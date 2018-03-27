-- kgsws' Lua Doom exports
-- Doom line actions

-- Level Exit

function levelExit(mobj, line, side, act)
	if act ~= doomLineType[line.special] or mobj.player == nil then
		return false
	end
	line.special = 0
	line.DoButton("dsswtchx")
	if eggCheckSecret(mobj) then
		-- Doom1 easter egg map
		game.Exit("KGSECRET")
	else
		game.DoomExit()
	end
	return true
end

function levelExitSecret(mobj, line, side, act)
	if act ~= doomLineType[line.special] or mobj.player == nil then
		return false
	end
	line.special = 0
	line.DoButton("dsswtchx")
	game.DoomExit(true)
	return true
end

linefunc[11] = levelExit
doomLineType[11] = lnspec.use

linefunc[52] = levelExit
doomLineType[52] = lnspec.cross

linefunc[51] = levelExitSecret
doomLineType[51] = lnspec.use

linefunc[124] = levelExitSecret
doomLineType[124] = lnspec.cross

