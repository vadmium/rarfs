#
#  Copyright (C) 2003-2008  Kent Gustavsson <nedo80@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

INCLUDES = -I..
bin_PROGRAMS = rarfs

AM_CXXFLAGS = `pkg-config --cflags fuse` 
AM_LDFLAGS = `pkg-config --libs fuse`

rarfs_SOURCES = main.cc \
	rarblock.h\
	rarblock.cc\
	rararchive.h                   \
	rararchive.cc                   \
	markerblock.h                   \
	markerblock.cc                   \
	archiveblock.h                   \
	archiveblock.cc                   \
	fileblock.h                   \
	fileblock.cc                   \
	main.h 
