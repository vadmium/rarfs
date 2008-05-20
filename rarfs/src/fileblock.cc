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
#include "main.h"

/* MS-DOS time/date conversion routines derived from: */

/*
 *  linux/fs/msdos/misc.c
 *
 *  Written 1992,1993 by Werner Almesberger
 */

/* Convert a MS-DOS time/date pair to a UNIX date (seconds since 1 1 70). */

/* Linear day numbers of the respective 1sts in non-leap years. */
static int day_n[] = { 0,31,59,90,120,151,181,212,243,273,304,334,0,0,0,0 };
int date_dos2unix(unsigned short time,unsigned short date)
{
       int month,year,secs;

       month = ((date >> 5) & 15)-1;
       year = date >> 9;
       secs = (time & 31)*2+60*((time >> 5) & 63)+(time >> 11)*3600+86400*
           ((date & 31)-1+day_n[month]+(year/4)+year*365-((year & 3) == 0 &&
           month < 2 ? 1 : 0)+3653)-7200;   // the code seems to be 2 hours off always -> -7200
                       /* days since 1.1.70 plus 80's leap day */
       return secs;
}

FileBlock::FileBlock(std::istream &in) : RARBlock(in),
in(in)
{
	unsigned int end = in.tellg();
	start = end - size - headsize;

	in.seekg(end + 16-size-headsize);
	unsigned short time;
	unsigned short date;
	time = in.get();
	time += in.get() *256;
	date = in.get();
	date += in.get() *256;
	filedate=date_dos2unix(time,date);

	in.seekg(end + 21-size-headsize);

	if ( in.get() == 48 )
		compressed = false;
	else
		compressed = true;

	unsigned int filenamesize = in.get();
	filenamesize += in.get() * 256;
	
	in.seekg(4, std::ios::cur);
	char m[filenamesize+1];
	
#ifdef OK_TO_BREAK
	if( flags & 0x0100 )
		in.seekg(8, std::ios::cur);
#endif

	in.read(m, filenamesize);
	m[filenamesize] = 0;
	filename = m;
	in.seekg(end);
	
	if ( ( flags & 0xE0 ) == 0xE0 )
		folder = true;
	else
		folder = false;
}

time_t
FileBlock::GetFileDate()
{
       return filedate;
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

	in.seekg(start + headsize + offset -4);
	
	if ( offset > size || !len) 
		return 0;
	if ( offset + len > size )
		len = size - offset;
	
	in.read(buf, len);
	in.seekg(old);
	
	return len;
}

