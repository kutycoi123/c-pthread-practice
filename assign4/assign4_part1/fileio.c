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
#include "util.h"
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
	if(lseek(fd, offset, SEEK_SET) == -1)
		return IOERR_POSIX;
	int readBytes = read(fd, buffer, bufbytes);
	if(readBytes == -1)
		return IOERR_POSIX;
	if(close(fd) == -1)
		return IOERR_POSIX;
    return readBytes;
}

int file_info(char *path, void *buffer, size_t bufbytes)
{
    if (!path || !buffer || bufbytes <= 0)
		return IOERR_INVALID_ARGS;
	struct stat fileStat;

	int fd = -1;
	fd = open(path, O_RDONLY);
	if(fd == -1)
		return IOERR_POSIX;
	if(fstat(fd, &fileStat) < 0)
		return IOERR_POSIX;	
	int size = fileStat.st_size;
	long accessed = fileStat.st_atim.tv_sec;
	long modified = fileStat.st_mtim.tv_sec; 
	int isDirectory = S_ISDIR(fileStat.st_mode);
	char type = isDirectory ? 'd' : 'f';
	
	sprintf((char*)buffer, "Size:%d Accessed:%ld Modified:%ld Type:%c", size, accessed, modified, type);
	
	if(close(fd) == -1)
		return IOERR_POSIX;
    return 0;
}

int file_write(char *path, int offset, void *buffer, size_t bufbytes)
{
	if(bufbytes <= 0 || !path || !buffer || offset < 0)
        return IOERR_INVALID_ARGS;
	int fd;
	fd = open(path, O_RDWR | O_CREAT, 0777);
	if(fd == -1)
		return IOERR_INVALID_PATH;
	if(lseek(fd, offset, SEEK_SET) == -1)
		return IOERR_POSIX;
	int writeBytes = write(fd, buffer, (int)bufbytes);	
	if(writeBytes == -1)
		return IOERR_POSIX;
	if(close(fd) == -1)
		return IOERR_POSIX;
    return writeBytes;
}

int file_create(char *path, char *pattern, int repeatcount)
{
	int fd = -1;
	fd = open(path, O_WRONLY|O_CREAT, 0777);
	if(fd == -1){
		close(fd);
		return IOERR_INVALID_PATH;
	}
	if(pattern != NULL){
		int size_of_pattern = strlen(pattern);
		int size_of_repeatedPattern = repeatcount * size_of_pattern;
		char* repeatedPattern = malloc(size_of_repeatedPattern);
		int i;	
		for(i = 0; i < size_of_repeatedPattern; ++i){
			repeatedPattern[i] = pattern[i % size_of_pattern];
		}	
		repeatedPattern[i] = '\0';
		if(write(fd, repeatedPattern, size_of_repeatedPattern) == -1){
			free(repeatedPattern);
			return IOERR_POSIX;
		}
		free(repeatedPattern);
	}
	if(close(fd) == -1)
		return IOERR_POSIX;	
    return 0;
}

	
int file_remove(char *path)
{
	if(path == NULL)
		return IOERR_INVALID_ARGS;
	int fd = remove(path);
	if(fd == -1)
		return IOERR_INVALID_PATH;
	
    return 0;
}

int dir_create(char *path)
{
	if(path == NULL)
		return IOERR_INVALID_ARGS;
	int err = mkdir(path, 0777);
	if(err == -1)
		return IOERR_INVALID_PATH;
    return 0;
}

int dir_list(char *path, void *buffer, size_t bufbytes)
{
	if(bufbytes <= 0 || !path || !buffer )
        return IOERR_INVALID_ARGS;
	DIR* dir = opendir(path);
	if(dir == NULL)
		return IOERR_INVALID_PATH;
	struct dirent* d;
	while((d = readdir(dir)) != NULL){
		if(strlen((char*)buffer) + strlen(d->d_name) < bufbytes){
			char*dir_name = malloc(strlen(d->d_name) + 1);
			sprintf(dir_name, "%s\n", d->d_name);
			strcat(buffer, dir_name);
			free(dir_name);
		
		}
		else
			return IOERR_BUFFER_TOO_SMALL;	
	}
	
	if(closedir(dir) == -1)
		return IOERR_POSIX;
    return 0;
}


int file_checksum(char *path)
{
	if(path == NULL)
		return IOERR_INVALID_ARGS;	
	char*buffer	;
	int fd = -1, file_size;
	fd = open(path, O_RDONLY);
	if(fd == -1){
		close(fd);
		return IOERR_INVALID_PATH;
	}
	if((file_size = lseek(fd, 0, SEEK_END)) == -1)
		return IOERR_POSIX;
	buffer = malloc(file_size);
	if(lseek(fd, 0, SEEK_SET) == -1)
		return IOERR_POSIX;	
	if(read(fd, buffer, file_size) == -1)
		return IOERR_POSIX;
	int checksum_val = checksum(buffer, file_size, 0);
	if(close(fd) == -1)
		return IOERR_POSIX;
    return checksum_val;
}

int dir_checksum(char *path)
{
	if(path == NULL)
		return IOERR_INVALID_ARGS;
	DIR* dir = opendir(path);
	if(dir == NULL){
		closedir(dir);
		return IOERR_INVALID_PATH;
	}
	int checksum_val = 0;
	struct dirent* d;
	while((d = readdir(dir)) != NULL){
		char* new_path = malloc(strlen(path) + strlen(d->d_name) + 2);
		sprintf(new_path, "%s/%s", path, d->d_name);

		if(!strcmp(d->d_name, ".") || !strcmp(d->d_name, "..")) continue;

		if(d->d_type == DT_DIR){
			int dir_checksum_val = dir_checksum(new_path);
			if(dir_checksum_val >= 0)
				checksum_val += dir_checksum_val;
			checksum_val = checksum(d->d_name, strlen(d->d_name), checksum_val);
		}else if(d->d_type == DT_REG){
			int file_checksum_val = file_checksum(new_path);
			if(file_checksum >= 0){
				checksum_val += file_checksum_val;
			}
		}
		free(new_path);
	}
	if(closedir(dir) == -1)
		return IOERR_POSIX;
    return checksum_val;
}
