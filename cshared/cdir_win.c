#define _CRT_SECURE_NO_WARNINGS
#include "cdir.h"
#include "cmem.h"
#include "cstr.h"
#include <windows.h>


struct cdir_t {
	WIN32_FIND_DATA fd;
	HANDLE      h;
	int         flags;
	char *      path;
	char *      fname;
	cdir_stat_t st;
};

static void _cdir_apply_filter(cdir_t * dir)
{
	do {
		if (dir->fd.dwFileAttributes &  FILE_ATTRIBUTE_DIRECTORY) {
			if (strcmp(".", dir->fd.cFileName) && strcmp("..", dir->fd.cFileName)){
				if (0 == (dir->flags & e_cdir_nodirs)) return;
			}
		}
		else {
			if (0 == (dir->flags & e_cdir_nofiles)) return;
		}
		//skip this file
	} while (FindNextFile(dir->h, &dir->fd));
	FindClose(dir->h);
	dir->h = INVALID_HANDLE_VALUE;
}

cdir_t * cdir_open(const pchar_t * path, const char * mask, int flags)
{
	cdir_t * dir;
	int plen = path ? pchar_len(path) : 0;
	dir = cnew(cdir_t);
	dir->path = pchar_alloc(plen + MAX_PATH + 1);
	pchar_cpy(dir->path, path);
	while (plen > 0 && (dir->path[plen - 1] == '/' || dir->path[plen - 1] == '\\'))plen--;
	if (plen > 0) {
		dir->path[plen] = '\\';
		dir->fname = dir->path + plen + 1;
	}
	else{
		dir->fname = dir->path;
	}
	dir->flags = flags;
	if (mask == NULL) mask = "*";
	strcpy(dir->fname, mask);
	dir->h = FindFirstFile(dir->path, &dir->fd);
	if (INVALID_HANDLE_VALUE == dir->h){
		cfree(dir);
		return NULL;
	}
	_cdir_apply_filter(dir);
	dir->st.path = dir->path;
	dir->st.fname = dir->fname;
	return dir;
}

void cdir_close(cdir_t * dir)
{
	if(dir){
		if(dir->h != INVALID_HANDLE_VALUE){
			FindClose(dir->h);
		}
		cfree(dir);
	}
}

cdir_t * cdir_rewind(cdir_t * dir)
{
	if(dir){
		if(dir->h != INVALID_HANDLE_VALUE){
			FindClose(dir->h);
		}
		*dir->fname  = 0;
		dir->h = FindFirstFile(dir->path, &dir->fd);
		if(INVALID_HANDLE_VALUE == dir->h){
			cfree(dir);
			dir = NULL;
		}
	}
	return dir;
}

const cdir_stat_t* cdir_next(cdir_t * dir)
{
	if (dir && dir->h != INVALID_HANDLE_VALUE){
		pchar_cpy(dir->fname, dir->fd.cFileName);
		dir->st.size = dir->fd.nFileSizeHigh;
		dir->st.size = (dir->st.size << 32) | dir->fd.nFileSizeLow;
		if (FindNextFile(dir->h, &dir->fd)){
			_cdir_apply_filter(dir);
		}else {
			FindClose(dir->h);
			dir->h = INVALID_HANDLE_VALUE;
		}
		return &dir->st;
	}
	return NULL;
}
