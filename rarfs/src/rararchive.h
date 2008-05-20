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
// Class: RARArchive
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sun Mar  5 00:40:27 2006
//

#ifndef _RARARCHIVE_H_
#define _RARARCHIVE_H_

#include <string>
#include <vector>
#include <map>
#include "rarblock.h"
#include "fileblock.h"
#include "markerblock.h"
#include "archiveblock.h"

class RARArchive
{
	public:
		RARArchive();
		~RARArchive();
		
		int Init(std::string filename);
		std::string GetFileName(int n);
		void PrintFiles();
		void PrintFolders();
		void Parse(bool showcompressed = false);
		unsigned int Read(const char *path, char *buf, size_t size, off_t offset);
		bool HasFile(std::string file);
		bool HasFolder(std::string f);
		unsigned long long int GetFileSize(std::string file);
		std::vector < std::string > GetFiles();
		std::vector < std::string > GetFolders();
		time_t GetDate(std::string file);
	protected:
		std::string filename;
		int firstfile;
		int archivetype;
		int default_date;
		std::vector<RARBlock *> blocks;
		std::map<std::string, std::vector<FileBlock *> > fileblocks;
		std::map<std::string, std::vector<FileBlock *> > folderblocks;
		
		std::vector<std::ifstream *> streams;
};


#endif	//_RARARCHIVE_H_

