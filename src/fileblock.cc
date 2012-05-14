/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


//
// Class: FileBlock
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Mon Mar  6 21:41:03 2006
//

#include "fileblock.h"
#include <time.h>
#include <ios>

/* MS-DOS time/date conversion routines derived from: */

/*
 *  linux/fs/msdos/misc.c
 *
 *  Written 1992,1993 by Werner Almesberger
 */

/* Convert a MS-DOS time/date pair (unspecified local time zones) to a UNIX
time_t. */

time_t date_dos2unix(unsigned short time,unsigned short date)
{
       struct tm tm;
       tm.tm_year = 1980 + (date >> 9) - 1900,
       tm.tm_mon = ((date >> 5) & 15) - 1,
       tm.tm_mday = date & 31,
       tm.tm_hour = time >> 11,
       tm.tm_min = (time >> 5) & 63,
       tm.tm_sec = (time & 31)*2,
       tm.tm_isdst = -1; /* unknown; mktime will guess */
       return mktime(&tm);
}

FileBlock::FileBlock(std::istream &in) : RARBlock(in),
in(in)
{
	unsigned int end = in.tellg();
	start = end - size - headsize;

	in.seekg(end + 20-size-headsize);
	unsigned short time;
	unsigned short date;
	time = in.get();
	time += in.get() *256;
	date = in.get();
	date += in.get() *256;
	filedate.tv_sec=date_dos2unix(time,date);

	in.seekg(end + 25-size-headsize);

	if ( in.get() == 48 )
		compressed = false;
	else
		compressed = true;

	unsigned int filenamesize = in.get();
	filenamesize += in.get() * 256;
	
	in.seekg(4, std::ios::cur);
	char m[filenamesize+1];
	
	if( flags & 0x0100 )
		in.seekg(8, std::ios::cur);

	in.read(m, filenamesize);
	m[filenamesize] = 0;
	filename = m;
	
	if ( flags & 0x0400 ) /* Salt field present */
	{
		in.seekg(min(in.tellg() + std::streamoff(8),
			std::streampos(start + headsize)));
	}
	
	/* Extended time stamp field present */
	if ( flags & 0x1000 &&
	in.tellg() + std::streamoff(2) <= start + headsize )
	{
		ParseXtime();
	}
	
	in.seekg(end);
	
	if ( ( flags & 0xE0 ) == 0xE0 )
		folder = true;
	else
		folder = false;
}

/* Derived from _parse_ext_time function in "rarfile" Python module.
http://rarfile.berlios.de/ */
void
FileBlock::ParseXtime()
{
	unsigned int flags = in.get();
	flags += in.get() * 256;
	int field = 4;
	
	--field;
	unsigned int field_flags = flags >> field * 4 & 0xF;
	
	unsigned long frac = 0; /* 100 ns units; 24 bits */
	if ( field_flags & 8 ) /* mtime field present */
	{
		filedate.tv_sec += field_flags >> 2 & 1;
		
		for ( unsigned int i = 0; i < (field_flags & 3); ++i )
		{
			frac >>= 8;
			frac += in.get() << 16;
		}
	}
	filedate.tv_nsec = frac * 100;
	
	/* Not interested in the three other time fields */
	while ( field > 0 )
	{
		--field;
		field_flags = flags >> field * 4 & 0xF;
		if ( 0 != (field_flags & 8) )
		{
			in.seekg(field_flags & 3, std::ios::cur);
		}
	}
}

void
FileBlock::GetFileDate(struct timespec* tp)
{
       tp->tv_sec = filedate.tv_sec;
       tp->tv_nsec = filedate.tv_nsec;
}

FileBlock::~FileBlock()
{
	
}

bool
FileBlock::isFolder()
{
	return folder;
}

bool
FileBlock::isCompressed()
{
	return compressed;
}

unsigned int
FileBlock::GetDataSize()
{
	return size;
}

std::string
FileBlock::GetFileName()
{
	return filename;
}

int
FileBlock::GetData(char *buf, unsigned int offset, unsigned int len)
{
	std::streampos old = in.tellg();

	in.seekg(start + headsize + offset);
	
	if ( offset > size || !len) 
		return 0;
	if ( offset + len > size )
		len = size - offset;
	
	in.read(buf, len);
	in.seekg(old);
	
	return len;
}

