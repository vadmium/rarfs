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

FileBlock::FileBlock(std::istream &in) : RARBlock(in),
in(in)
{
	unsigned int end = in.tellg();
	start = end - size - headsize;
	
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

