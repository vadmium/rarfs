/***************************************************************************
 *  Copyright (C) 2006-2008  Kent Gustavsson <nedo80@gmail.com>
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
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sun Mar  5 00:40:27 2006
//

#include "rararchive.h"
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdlib>
#include <time.h>

RARArchive::RARArchive()
{
	default_date = 0;
}


RARArchive::~RARArchive()
{
	std::vector <std::ifstream*>::iterator iter;
	
	for( iter = streams.begin() ; iter != streams.end() ; iter++)
	{
		(*iter)->close();
	}
}

int
RARArchive::Init(std::string filename)
{	
	struct stat st;
	stat(filename.c_str(), &st);
	default_date = st.st_mtime;
	
// Hate this part
	this->filename = filename;
	
	volsuffix = ".rar";
	voldigits = 0;
	std::string numprefix;
	std::size_t offset;
	
	offset = volsuffix.size();
	if ( offset <= filename.size() &&
	filename.substr(filename.size()-offset) == volsuffix )
	{
		/* Ends in ".rar"; check for "part<N>.rar" */
		numprefix = "part";
	}
	else
	{
		/* Try ".<N>" */
		volsuffix = "";
		numprefix = ".";
	}
	
	while(true)
	{
		offset = voldigits+volsuffix.size();
		if ( offset >= filename.size() ||
		!isdigit(filename[filename.size()-offset-1]))
		{
			break;
		}
		
		voldigits++;
		if ( voldigits > 5 )
		{
			std::cerr << "Too many digits in volume number" <<
				std::endl;
			return false;
		}
	}
	
	offset = numprefix.size() + voldigits + volsuffix.size();
	if ( offset <= filename.size() && filename.substr(
	filename.size() - offset, numprefix.size()) == numprefix )
	{
		offset = volsuffix.size()+voldigits;
		volprefix = filename.substr(0, filename.size()-offset);
		firstfile = atoi( filename.substr(
			filename.size()-offset, voldigits).c_str() );
	}
	else
		voldigits = 0;
	
	return true;
}

std::string
RARArchive::GetFileName(int n)
{
	if ( n == 0 )
		return filename;
		
	std::stringstream f;

	if ( voldigits == 0 )
	{
		/* n=0 => .rar (see above), n=1 => .r00, n=2 => .r01, etc */
		if( n-1 < 10 )
			f << filename.substr(0,filename.size()-2) << 0 << n-1;
		else
			f << filename.substr(0,filename.size()-2) << n-1;
	}
	else
	{
		char prev;
		
		f << volprefix;
		
		prev = f.fill ('0');
		f.width (voldigits);
		f << n + firstfile;
		f.fill(prev);
		f.width(0);
		
		f << volsuffix;
	}
		
	return f.str();
}

bool
RARArchive::HasFile(std::string f)
{
	if( fileblocks.find(f) == fileblocks.end() )
		return false;
	return true;
	
}

bool
RARArchive::HasFolder(std::string f)
{
	if( folderblocks.find(f) == folderblocks.end() )
		return false;
	return true;
	
}
unsigned long long int 
RARArchive::GetFileSize(std::string file)
{
	unsigned long long int size = 0;
	if( fileblocks.find(file) == fileblocks.end() )
		return 0;
		
	std::vector <FileBlock *>::iterator i;
	for(i = fileblocks[file].begin() ; i != fileblocks[file].end() ; i++ )
	{
		size += (*i)->GetDataSize();
	}
	
	return size;
}

void
RARArchive::Parse(bool showcompressed)
{
	int n = 0;
	for(;;)
	{
		std::ifstream *file = new std::ifstream(GetFileName(n++).c_str());
		if(!file->good())
			return;
		streams.push_back(file);
		
		for(;;)
		{	
			if(!file->good())
				break;
			
			char buf[4];
			file->read(buf,3);
			//for some rar files there are seeks past file end, then directly to file end
			//which makes file->good() return true, but there is nothing to read
			//so with file->gcount() we check if we actually read something
			//otherwise we use old bufer and crash somewhere
			if(file->gcount()==0)
				break;

			file->seekg (-3, std::ios::cur);
			
			RARBlock* block = 0;
			switch( buf[2] )
			{
				case 0x00:
					break;
				case RARBlock::MARKER:
					block = new MarkerBlock(*file);
					break;
				case RARBlock::ARCHIVE:
					block = new ArchiveBlock(*file);
					break;
				case RARBlock::FILE:
					FileBlock *f;
					f = new FileBlock(*file);
					
					if ( showcompressed || !f->isCompressed() )
					{
						if ( f->isFolder() )
							folderblocks[f->GetFileName()].push_back(f);
						else
						{
							fileblocks[f->GetFileName()].push_back(f);
	
							for ( int pos = 0 ; ( pos = f->GetFileName().find("\\", pos) ) != std::string::npos ; pos++)
							{
								if( folderblocks.find(f->GetFileName().substr(0,pos)) == folderblocks.end())
									folderblocks[f->GetFileName().substr(0,pos)].push_back(NULL);
							}
						}
					}
					
					block = f;
					
					break;
				case RARBlock::SUB_3_00:
					block = new FileBlock(*file);
					break;
				default:
					block = new RARBlock(*file);
			}
			if ( block )
			{
				blocks.push_back( block );
				file->seekg( block->GetEndPos() );
			}
		
			if ( buf[2] == 0 || buf[2] == RARBlock::END ||
			buf[2] == RARBlock::RECOVERY_2_00 )
			{
				
				break;
			}
		}
		file->clear();
		file->seekg(0);
	}

}

unsigned int
RARArchive::Read(const char *path, char *buf, size_t size, off_t offset)
{
	size_t pos = 0;
	if ( fileblocks.find(path) == fileblocks.end() )
	//	return -ENOENT;
		return -1;
		
	std::vector <FileBlock *>::iterator i;
	for(i = fileblocks[path].begin() ; i != fileblocks[path].end() ; i++ )
	{
		if ( (*i)->GetDataSize() > offset )
		{	
			size_t len = (*i)->GetData(buf + pos, offset, size);
			pos += len;
			size -= len;
			offset = 0;
		}
		else
		{
			offset -= (*i)->GetDataSize();
		}
	}
	return pos;
}

std::vector < std::string >
RARArchive::GetFolders()
{	
	std::vector <std::string> retdata;
	
	std::map<std::string, std::vector <FileBlock *> >::iterator iter;
	
	for( iter = folderblocks.begin() ; iter != folderblocks.end() ; iter++)
		retdata.push_back(iter->first);


	return retdata;
}

std::vector < std::string >
RARArchive::GetFiles()
{
	std::vector <std::string> retdata;
	std::map<std::string, std::vector <FileBlock * > >::iterator iter;
	
	for( iter = fileblocks.begin() ; iter != fileblocks.end() ; iter++)
	{
		retdata.push_back(iter->first);
	}
	
	return retdata;
}

void
RARArchive::PrintFiles()
{


	std::map<std::string, std::vector <FileBlock * > >::iterator iter;
	
	for( iter = fileblocks.begin() ; iter != fileblocks.end() ; iter++)
	{
		unsigned long long int s = 0;
		std::vector <FileBlock *>::iterator i;
		for(i = iter->second.begin() ; i != iter->second.end() ; i++ )
		{
			s += (*i)->GetDataSize();
		}
			
		std::cout << iter->first << " blocks: " << iter->second.size() << " size: " << s << std::endl;
	}
}

void
RARArchive::GetDate(std::string file, struct timespec* tp)
{
	if( fileblocks.find(file) == fileblocks.end() )
	{
		tp->tv_sec = default_date;
		tp->tv_nsec = 0;
		return;
	}

	std::vector <FileBlock *>::iterator i;
	i = fileblocks[file].begin();
	(*i)->GetFileDate(tp);
}

void
RARArchive::PrintFolders()
{
	std::map<std::string, std::vector <FileBlock * > >::iterator iter;
	
	for( iter = folderblocks.begin() ; iter != folderblocks.end() ; iter++)
	{
		unsigned long long int s = 0;
		std::vector <FileBlock *>::iterator i;
		for(i = iter->second.begin() ; i != iter->second.end() ; i++ )
			s += (*i)->GetDataSize();
			
		std::cout << iter->first << " blocks: " << iter->second.size() << " size: " << s << std::endl;
	}
}

