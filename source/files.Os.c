#include "common/general.h"
#include <errno.h>

u32 GetFileSize(FILE *file) {
    fseek(file, 0, SEEK_END);
    return ftell(file);
}

bool DS_InitFS(int argc, char **argv)
{
	#ifndef FATONLY
		if(nitroFSInit(&basePath))
		{
			printf("init : done");
			chdir("nitro:/");
			chdir(ROOT);
			if(!argv){saveAvailable=false;fsMode=1;} // no fat but nitro (gba slot)
			else{
				fsMode=2; // nitro and fat
				saveAvailable=true;
				chdir(basePath);
				int r=mkdir("dscraft", S_IRWXU|S_IRGRP|S_IXGRP);//!ROOT!
				if(r!=0 && errno!=EEXIST)saveAvailable=false;
				r=mkdir("dscraft/packs", S_IRWXU|S_IRGRP|S_IXGRP);
				if(r!=0 && errno!=EEXIST)saveAvailable=false;
				r=mkdir("dscraft/worlds", S_IRWXU|S_IRGRP|S_IXGRP);
				if(r!=0 && errno!=EEXIST)saveAvailable=false;
				r=mkdir("dscraft/screens", S_IRWXU|S_IRGRP|S_IXGRP);
				if(r!=0 && errno!=EEXIST)saveAvailable=false;
				NOGBA("can save : %d",saveAvailable);
				chdir("nitro:/");
				chdir(ROOT);
			}
			return true;
		}
	#endif
	fsMode=3; // fat only ?
	saveAvailable=false;
	if(!fatInitDefault())return false;
	saveAvailable=true;
	// int r=mkdir("fat:/dscraft", S_IRWXU|S_IRGRP|S_IXGRP);//!ROOT!
	// if(r!=0 && errno!=EEXIST)saveAvailable=false;
	// r=mkdir("fat:/dscraft/packs", S_IRWXU|S_IRGRP|S_IXGRP);
	// if(r!=0 && errno!=EEXIST)saveAvailable=false;
	// r=mkdir("fat:/dscraft/worlds", S_IRWXU|S_IRGRP|S_IXGRP);
	// if(r!=0 && errno!=EEXIST)saveAvailable=false;
	// r=mkdir("fat:/dscraft/screens", S_IRWXU|S_IRGRP|S_IXGRP);
	// if(r!=0 && errno!=EEXIST)saveAvailable=false;
	// chdir("fat:/");
	// chdir(ROOT);
	int r=mkdir("dscraft", S_IRWXU|S_IRGRP|S_IXGRP);//!ROOT!
	if(r!=0 && errno!=EEXIST)saveAvailable=false;
	r=mkdir("dscraft/packs", S_IRWXU|S_IRGRP|S_IXGRP);
	if(r!=0 && errno!=EEXIST)saveAvailable=false;
	r=mkdir("dscraft/worlds", S_IRWXU|S_IRGRP|S_IXGRP);
	if(r!=0 && errno!=EEXIST)saveAvailable=false;
	r=mkdir("dscraft/screens", S_IRWXU|S_IRGRP|S_IXGRP);
	if(r!=0 && errno!=EEXIST)saveAvailable=false;
	// chdir("sd:/");
	chdir(ROOT);
	return true;
}

void* DS_OpenFile (char* filename, char* dir, bool bufferize, bool binary)
{
	char path[255];getcwd(path,255);
	chdir(dir);
	FILE* file;
	NOGBA("opening %s...",filename);
	
	if(binary==false)file = fopen(filename, "r+");
	else file = fopen(filename, "rb+");
	//DS_Debug("done.");
	
		if(file == NULL){NOGBA("error: couldn't locate \"%s\" in %s ! (%s)\n", filename, dir, path);}
	
	if(bufferize == true)
	{
		u8* buffer;
		long lsize;
		fseek (file, 0 , SEEK_END);
		lsize = ftell (file);
		lastSize=lsize;
		rewind (file);
		buffer = (u8*) malloc (lsize);
		
			if(buffer==NULL)DS_Error(3);
			
		fread (buffer, 1, lsize, file);
		fclose (file);
		chdir(path);
		return buffer;
	}
	
	chdir(path);
	return file;
}

void* DS_OpenFile2 (char* filename, char* dir, bool binary)
{
	char path[255];getcwd(path,255);
	int r=chdir(dir);
	if(strlen(dir)<=0)r=-1;
	FILE* file;
	DS_Debug("opening %s...",filename);
	
		if(binary==false)file = fopen(filename, "r+");
		else file = fopen(filename, "rb+");
	
	DS_Debug("done.");
	
		if(file == NULL)DS_Error(5);
		
		u8* buffer;
		long lsize;
		fseek (file, 0 , SEEK_END);
		lsize = ftell (file);
		rewind (file);
		buffer = (u8*) DS_mAlloc (lsize, CurrentState);
		
			if(buffer==NULL)DS_Error(3);
			
		fread (buffer, 1, lsize, file);
		fclose (file);
		chdir(path);
		return buffer;
}
