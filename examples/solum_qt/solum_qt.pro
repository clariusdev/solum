TEMPLATE = subdirs

SUBDIRS = solum openigtlink

openigtlink.subdir = ../../openigtlink
solum.depends = openigtlink
