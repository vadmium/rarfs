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
#define FUSE_USE_VERSION 25

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse_opt.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>

#include <iostream>
#include <fstream>
#include "rarblock.h"
#include "rararchive.h"

RARArchive arc;

static int rarfs_getattr(const char *p, struct stat *stbuf)
{
	int res = 0;

	std::string path = p + 1;
	int pos;
	while ( (pos = path.find("/")) != std::string::npos )
		path.replace(pos,1,"\\");
	
	memset(stbuf, 0, sizeof(struct stat));
	if(strcmp(p, "/") == 0) 
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		
		arc.GetDate(path, &stbuf->st_atim);
		arc.GetDate(path, &stbuf->st_mtim);
	}
	else if(arc.HasFolder(path))
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		arc.GetDate(path, &stbuf->st_atim);
		arc.GetDate(path, &stbuf->st_mtim);
	}
	else if(arc.HasFile(path)) 
	{
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = arc.GetFileSize(path);
		arc.GetDate(path, &stbuf->st_atim);
		arc.GetDate(path, &stbuf->st_mtim);
	}
	else
		res = -ENOENT;

	return res;
}

static int rarfs_readdir(const char *p, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
	std::vector<std::string> list = arc.GetFiles();
	std::vector<std::string>::iterator iter;
	std::map < std::string, int > ugly;
	
	std::string path = p + 1;
	int pos;
	while ( (pos = path.find("/")) != std::string::npos )
		path.replace(pos,1,"\\");
		
	if(strcmp(p, "/") != 0)
	{
		if (!arc.HasFolder(path))	
			return -ENOENT;
	}
	
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	
	for(iter = list.begin() ; iter != list.end() ; iter++)
	{
		if( iter->size() > path.size() )
		{
			if ( iter->substr(0, path.size()) == path )
			{
				std::string file = iter->substr(path.size());
				if ( file[0] == '\\' )
					file = file.substr(1);
				
				if( file.find("\\") == std::string::npos )
				{
					filler(buf, file.c_str(), NULL, 0);
				}
			}
		}
	}
	
	list = arc.GetFolders();
	for(iter = list.begin() ; iter != list.end() ; iter++)
	{
		if( iter->size() > path.size() )
		{
			if ( iter->substr(0, path.size()) == path )
			{
				std::string folder = iter->substr(path.size());
				if ( folder[0] == '\\' )
					folder = folder.substr(1);
				folder = folder.substr(0, folder.find("\\"));
				ugly[folder] = 1;
			}
		}
	}
	
	std::map <std::string, int>::iterator ui;
	for( ui = ugly.begin() ; ui != ugly.end() ; ui++)
		filler(buf, ui->first.c_str(), NULL, 0);

	return 0;
}

static int rarfs_open(const char *p, fuse_file_info *fi)
{
	std::string path = p + 1;
	int pos;
	while ( (pos = path.find("/")) != std::string::npos )
		path.replace(pos,1,"\\");
	
	
	if(!arc.HasFile(path))
		return -ENOENT;


	if((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}

static int rarfs_read(const char *p, char *buf, size_t size, off_t offset, fuse_file_info *fi)
{
	std::string path = p + 1;
	int pos;
	while ( (pos = path.find("/")) != std::string::npos )
		path.replace(pos,1,"\\");

	size_t len;
	if(!arc.HasFile(path))
		return -ENOENT;

	return arc.Read(path.c_str(), buf, size, offset) ;
}

static struct fuse_operations rarfs_oper;

struct Opt
{
	Opt();
	static void usage(char const* arg0);
	
	char rarfile[512];
	bool mount_expected;
	
	/* "Key" values for command-line options */
	static const int KEY_HELP = 0;
	
	static const int KEEP = 1;
	static const int DISCARD = 0;
};

Opt::Opt() :
mount_expected(true)
{
	rarfile[0] = 0;
}

static struct fuse_opt rarfs_opts[] = {
	FUSE_OPT_KEY("-h", Opt::KEY_HELP),
	FUSE_OPT_KEY("--help", Opt::KEY_HELP),
	FUSE_OPT_END
};

static int rarfs_opt_proc(void *data, const char *arg, int key,
                          struct fuse_args *outargs)
{
	Opt *param = static_cast<Opt*>(data);

	switch (key) {
		case FUSE_OPT_KEY_NONOPT:
			if( param->rarfile[0] == 0 )
			{
				strcpy(param->rarfile, arg);
				return Opt::DISCARD;
			}
			break;
			
		case Opt::KEY_HELP:
			Opt::usage(outargs->argv[0]);
			putc('\n', stderr);
			
			/* Copied from usage() in helper.c in Fuse 2.9.0 */
			fprintf(stderr,
			        "general options:\n"
			        "    -o opt,[opt...]        mount options\n"
			        "    -h   --help            print help\n"
			        "    -V   --version         print version\n"
			        "\n");
			
			/* Substitute "help without header" option for fuse_
			parse_cmdline() */
			int ret;
			ret = fuse_opt_add_arg(outargs, "-ho");
			if( ret < 0 )
			{
				return ret;
			}
			
			param->mount_expected = false;
			return Opt::DISCARD;
	}
	
	return Opt::KEEP;
}

void Opt::usage(char const* arg0)
{
	fprintf(stderr, "USAGE: %s <file> <dir>\n", arg0);
}

int main( int argc, char ** argv)
{

	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	Opt param;
	
	if (fuse_opt_parse(&args, &param, rarfs_opts, rarfs_opt_proc) == -1)
		exit(1);
				
	if ( param.mount_expected )
	{
		if ( ! arc.Init(param.rarfile) )
		{
			Opt::usage(argv[0]);
			return -1;
		}
		
		rarfs_oper.getattr = rarfs_getattr;
		rarfs_oper.readdir = rarfs_readdir;
		rarfs_oper.open = rarfs_open;
		rarfs_oper.read = rarfs_read;

		arc.Parse(false);
	}
	
	return fuse_main(args.argc, args.argv, &rarfs_oper);
}
