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
// Class: FileBlock
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Mon Mar  6 21:41:03 2006
//

#ifndef _FILEBLOCK_H_
#define _FILEBLOCK_H_

#include "rarblock.h"

class FileBlock : public RARBlock
{
	public:
		FileBlock(std::istream &in);
		 ~FileBlock();
	
		std::string GetFileName();
		time_t GetFileDate();
		unsigned int GetDataSize();
		int GetData(char *buf, unsigned int offset, unsigned int len);
		bool isFolder();
		bool isCompressed();
	protected:
		std::string filename;
		unsigned int start;
		std::istream &in;
		bool folder;
		bool compressed;
		time_t filedate;
};


#endif	//_FILEBLOCK_H_

