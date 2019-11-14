#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "restart.h"
#include "fileio.h"

#if 1
#define VERBOSE(p) (p)
#else
#define VERBOSE(p) (0)
#endif

int file_read(char *path, int offset, void *buffer, size_t bufbytes)
{
    if(bufbytes <= 0 || !path || !buffer || offset < 0)
        return IOERR_INVALID_ARGS;
   /* 
    FILE *fp;
    fp = fopen(path, "rb");
    if(!fp)
        return IOERR_INVALID_PATH;
    fseek(fp, offset, SEEK_SET);
    int readNum = fread(buffer, (int)bufbytes, 2, fp);
	if(!readNum){
		
	}
    //printf("%d\n", readNum);
    // int readBytes = readNum * sizeof(buffer)/sizeof(buffer[0]);
	*/
	int fd;
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
    return IOERR_NOT_YET_IMPLEMENTED;
}

int file_write(char *path, int offset, void *buffer, size_t bufbytes)
{
    return IOERR_NOT_YET_IMPLEMENTED;
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
