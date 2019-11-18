#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "restart.h"
#include "fileio.h"
#include "string.h"
#if 1
#define VERBOSE(p) (p)
#else
#define VERBOSE(p) (0)
#endif

int file_read(char *path, int offset, void *buffer, size_t bufbytes)
{
    if(bufbytes <= 0 || !path || !buffer || offset < 0){
        return IOERR_INVALID_ARGS;
	}
	int fd = -1;
	fd = open(path, O_RDONLY);
	if(fd == -1)
		return IOERR_INVALID_PATH;
	lseek(fd, offset, SEEK_SET);
	int readBytes = read(fd, buffer, bufbytes);
    return readBytes;
}

int file_info(char *path, void *buffer, size_t bufbytes)
{
    if (!path || !buffer || bufbytes < 1)
		return IOERR_INVALID_ARGS;
	struct stat fileStat;

	if (stat(path, &fileStat) < 0)
		return IOERR_POSIX; 	
	int size = fileStat.st_size;
	long accessed = fileStat.st_atim.tv_sec;
	long modified = fileStat.st_mtim.tv_sec; 
	int isDirectory = S_ISDIR(fileStat.st_mode);
	char type = isDirectory ? 'd' : 'f';
	
	sprintf(buffer, "Size:%d Accessed:%ld Modified:%ld Type:%c", size, accessed, modified, type);
    return 1;
}

int file_write(char *path, int offset, void *buffer, size_t bufbytes)
{
	if(bufbytes <= 0 || !path || !buffer || offset < 0)
        return IOERR_INVALID_ARGS;
	int fd;
	fd = open(path, O_WRONLY|O_APPEND|O_CREAT, 0777);
	if(fd == -1)
		return IOERR_INVALID_PATH;
	lseek(fd, offset, SEEK_SET);
	int writeBytes = write(fd, buffer, (int)bufbytes);	
    return writeBytes;
}

int file_create(char *path, char *pattern, int repeatcount)
{
    return IOERR_NOT_YET_IMPLEMENTED;
}

int file_remove(char *path)
{
    return IOERR_NOT_YET_IMPLEMENTED;
}

int dir_create(char *path)
{
    return IOERR_NOT_YET_IMPLEMENTED;
}

int dir_list(char *path, void *buffer, size_t bufbytes)
{
    return IOERR_NOT_YET_IMPLEMENTED;
}


int file_checksum(char *path)
{
    return IOERR_NOT_YET_IMPLEMENTED;
}

int dir_checksum(char *path)
{
    return IOERR_NOT_YET_IMPLEMENTED;
}
