-- kgsws' Lua Doom exports
-- easter egg kgDoom map

egg_render_style = {"!SHADOW", "!HOLEY0", "!HOLEY1"}

function eggSpawnFloor(sector, line)
	sector.AddFloor(line.frontsector, line, line.arg1)
end

function eggAddFloor(line)
	sectorTagIterator(line.arg0, eggSpawnFloor, line)
end
