#!/usr/bin/python3
import sys
import os
from struct import pack

# lump data offset
loffs = 12
# lump array
lumps = []

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

basename = sys.argv[1]
basepath = os.path.split(basename)[0]

# go trough listing
f = open(basename + ".txt")
for line in f:
	line = line.rstrip('\n')
	if len(line) > 0 and line[0] != ";" and line[0] != "#":
		# split lump name and path
		line = line.split("=", 1)
		# always uppercase
		line[0] = line[0].upper()
		# check lump name
		if len(bytearray(line[0], "UTF-8")) > 8:
			raise ValueError("- too long lump name " + line[0])
		# check lump type
		if len(line) > 1:
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

