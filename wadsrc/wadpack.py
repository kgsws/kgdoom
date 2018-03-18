#!/usr/bin/python3
import sys
import os
from struct import pack
from struct import unpack_from

# single color map generation
def split_range(par):
	par = par.split("-")
	if len(par) > 2:
		raise ValueError("invalid colormap range " + par)
	elif len(par) > 1:
		return [int(par[0]), int(par[1])]
	else:
		return [int(par[0]), int(par[0])]

def colormap_range(data, par):
	par = par.split("=")
	if len(par) != 2:
		raise ValueError("invalid colormap range " + par)
	dest = split_range(par[0])
	srce = split_range(par[1])
	if dest[0] > dest[1]:
		temp = dest[0]
		dest[0] = dest[1]
		dest[1] = temp
		temp = srce[0]
		srce[0] = srce[1]
		srce[1] = temp
	step = (1 + srce[1] - srce[0]) / (1 + dest[1] - dest[0])
	idx = srce[0]
	for i in range(dest[0], dest[1] + 1):
		# print("map " + str(i) + " as " + str(int(idx)))
		data[i] = int(idx)
		idx += step

def colormap_parse(par):
	data = bytearray(range(0, 256, 1))
	par = par.split(",")
	for cmap in par:
		colormap_range(data, cmap)
	return data

# special lump generation
class lump_gen():
	def colormap(name, par):
		# generate colormaps
		global lumps
		global loffs
		par = par.split(";")
		data = bytearray()
		for cmap in par:
			data.extend(colormap_parse(cmap))
		if len(data) > 0:
			# add this lump
			lumps.append([name, data, len(data), loffs])
			# advance offset
			loffs += len(data)
	def wad(name, par):
		# add wad file
		global lumps
		global loffs
		f = open(os.path.join(basepath, par), "rb")
		data = f.read()
		f.close()
		ipwad = unpack_from("<III", data, 0)
		if ipwad[0] != 0x44415749 and ipwad[0] != 0x44415750:
			raise ValueError(par + " is not a WAD file")
		for i in range(0, ipwad[1]):
			offset = i * 16
			entry = unpack_from("<II", data, ipwad[2] + offset)
			name = data[ipwad[2] + offset + 8:ipwad[2] + offset + 16].decode("UTF-8")
			if entry[1] > 0:
				ldata = data[entry[0]:entry[0]+entry[1]]
				# add this lump
				lumps.append([name, ldata, entry[1], loffs])
				# advance offset
				loffs += len(ldata)
			else:
				lumps.append([name])

# add lump from file
def add_lump(name, path):
	global lumps
	global loffs
	global basepath
	# open and read file
	f = open(os.path.join(basepath, path), "rb")
	data = f.read()
	f.close()
	# add script name as a first comment
	if name == "GAMELUA":
		data = bytearray("---" + path, "UTF-8") + b"\n" + data
	# align data to 4 bytes
	size = len(data)
	if size & 3:
		data = data.ljust((size + 3) & ~3, b"\0")
	# add this lump
	lumps.append([name, data, size, loffs])
	# advance offset
	loffs += len(data)

# add empty lump; = marker
def add_empty(name):
	global lumps
	# add this marker
	lumps.append([name])

#
# WAD packer
# create WADs from textfile listing
#

print(" kgsws' WAD packer ")

# lump data offset
loffs = 12
# lump array
lumps = []

basename = sys.argv[1]
basepath = os.path.split(basename)[0]

# go trough listing
f = open(basename + ".txt")
for line in f:
	line = line.rstrip('\n')
	if len(line) > 0 and line[0] != ";" and line[0] != "#":
		# split lump name and path
		line = line.split("=", 1)
		# check for generated lumps
		if len(line) > 1:
			temp = line[1].split(":", 1)
			if len(temp) > 1:
				line[1] = temp[0]
				line.append(temp[1])
		# always uppercase
		line[0] = line[0].upper()
		# check lump name
		if len(bytearray(line[0], "UTF-8")) > 8:
			raise ValueError("too long lump name " + line[0])
		# check lump type
		if len(line) > 2:
			# generate this lump
			getattr(lump_gen, line[1])(line[0], line[2])
		elif len(line) > 1:
			# add lump from file
			add_lump(line[0], line[1])
		else:
			# add empty marker
			add_empty(line[0])
f.close()

# create WAD
f = open(os.path.basename(basename) + ".wad", "wb")
# add header
f.write(pack('<III', 0x44415750, len(lumps), loffs))

# add lump data
for lump in lumps:
	if len(lump) > 1:
		f.write(lump[1])

# add lump listing
for lump in lumps:
	if len(lump) > 1:
		print("- lump " + lump[0] + " " + str(lump[2]) + "B")
		f.write(pack('<II', lump[3], lump[2]))
	else:
		print("- mark " + lump[0])
		f.write(pack('<II', 0, 0))
	f.write(bytearray(lump[0], "UTF-8").ljust(8, b"\0"))

f.close()

