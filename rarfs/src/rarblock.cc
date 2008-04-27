/***************************************************************************
 *  Copyright (C) 2006  Kent Gustavsson <nedo80@gmail.com>
 ****************************************************************************/
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
// Class: RARBlock
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Feb 28 16:05:38 2006
//

#include "rarblock.h"


RARBlock::RARBlock(std::istream &in)
{
	checksum = in.get() * 256;
	checksum += in.get();
	
	blocktype = in.get();
	
	flags = in.get();
	flags += in.get()* 256;
	
	headsize = in.get();
	headsize += in.get() * 256;
	
	if ( ( flags & 0x8000 ) != 0 )
	{
		size = in.get();
		size += in.get() * 256;
		size += in.get() * 256 * 256;
		size += in.get() * 256 * 256 * 256;
	}
	else
		size = 0;
	
	
	for ( int datalen = headsize - 7; datalen > 0 ; datalen -- )
		in.get();
	
	in.seekg(size, std::ios::cur);
}

RARBlock::~RARBlock()
{
	
}

bool
RARBlock::isBlock()
{
	return (blocktype);
}

void
RARBlock::print()
{

	std::cout << "Checksum: 0x" << (std::hex) << checksum << std::endl;
	std::cout << "BlockType: 0x" << (std::hex) << (int)blocktype;
	
	switch ( blocktype )
	{
		case 0x72:
			std::cout << " Marker Block" << std::endl;
			break;
		case 0x73:
			std::cout << " Archive Header" << std::endl;
			break;
		case 0x74:
			std::cout << " File Header" << std::endl;
			break;
		case 0x75:
			std::cout << " Comment Header" << std::endl;
			break;
		case 0x76:
			std::cout << " Extra Information" << std::endl;
			break;
		case 0x77:
			std::cout << " Sub Block" << std::endl;
			break;
		case 0x78:
			std::cout << " Recovery Record" << std::endl;
			break;
		default:
			std::cout << " Unknown Block" << std::endl;
			break;
	}
	
	std::cout << "Flags: 0x" << (std::hex) << flags << std::endl;
	if( blocktype == 0x74 )
	{
		std::cout << "\tData in previous file " << (int)(flags & 0x01) << std::endl;
		std::cout << "\tData in continues file " << (int)(flags & 0x02) << std::endl;
		std::cout << "\tPassword " << (int)(flags & 0x04) << std::endl;
		std::cout << "\tFile Comment " << (int)(flags & 0x08) << std::endl;
		std::cout << "\tSolid flag " << (int)(flags &0x10) << std::endl;
		std::cout << "\tDictionary: ";
		
		switch ((int)(flags &0xe0) >> 5)
		{
			case 0:
				std::cout << "64 Kb" << std::endl;
				break;
			case 1:
				std::cout << "128 Kb" << std::endl;
				break;
			case 2:
				std::cout << "256 Kb" << std::endl;
				break;
			case 3:
				std::cout << "512 Kb" << std::endl;
				break;
			case 4:
				std::cout << "1024 Kb" << std::endl;
				break;
			case 5:
				std::cout << "reserved_1" << std::endl;
				break;
			case 6:
				std::cout << "reserved_2" << std::endl;
				break;
			case 7:
				std::cout << "directory" << std::endl;
				break;
		
		}
	}
	std::cout << "Size: " << headsize << std::endl;
	std::cout << "DataSize: " << size << std::endl;
}	
