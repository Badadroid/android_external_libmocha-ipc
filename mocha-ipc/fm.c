/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2011-2012 KB <kbjetdroid@gmail.com>
 *
 * Implemented as per the Mocha AP-CP protocol analysis done by Dominik Marszk
 *
 * libmocha-ipc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libmocha-ipc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libmocha-ipc.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * TODO: Optimize this file
 * 		 Implement following functions properly:
 * 		 	FmMoveFile
 * 		 	FmGetFileAttributes
 * 		 	FmFGetFileAttributes
 * 		 	FmSetFileAttributes
 * 		 	FmTruncateFile
 *			FmOpenDir
 *			FmCloseDir
 * 		 	FmReadDir
 * 		 	FmRemoveDir
 * 		 	FmGetQuotaSpaceFile
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <getopt.h>

#include <fm.h>
#include <radio.h>
#include <dirent.h>
#include <errno.h>

#define LOG_TAG "RIL-Mocha-FM"
#include <utils/Log.h>

#define MAX_OPEN_DIRS 	10

#define PATH_MAX_LEN (92)
/* Has to be used with lock held! */
char nameBuf[PATH_MAX_LEN];

DIR* dirArray[MAX_OPEN_DIRS];
uint32_t dirIndex = 0;

#if defined(DEVICE_JET)
char *mochaRoot = "/KFAT0";
#elif defined(DEVICE_WAVE)
char *mochaRoot = "/data/radio";
#endif

int32_t (*fileOps[MAX_FILE_OPS])(struct fmRequest *, struct fmResponse *) =
{
	&FmOpenFile,
	&FmCloseFile,
	&FmCreateFile,
	&FmReadFile,
	&FmWriteFile,
	&FmFlushFile,
	&FmSeekFile,
	&FmTellFile,
	&FmRemoveFile,
	&FmMoveFile,
	&FmGetFileAttributes,
	&FmFGetFileAttributes,
	&FmSetFileAttributes,
	&FmTruncateFile,
	&FmOpenDir,
	&FmCloseDir,
	&FmReadDir,
	&FmCreateDir,
	&FmRemoveDir,
	&FmGetQuotaSpace
};

int32_t FmGetLastError()
{
	int32_t ret;
	switch(errno)
	{
		case EACCES:
			ret = FM_PERMISSION_ERROR;
		break;
		case ENOENT:
			ret = FM_ENTRY_NOT_FOUND_ERROR;
		break;
		case EBADF:
			ret = FM_INVALID_FILE_HANDLE;
		break;
		case EEXIST:
		case ENOTDIR:
			ret = FM_INVALID_PATH_ERROR; /* FM_ENTRY_EXIST_ERROR might be more appropiate here, though Mocha appears to use INVALID_PATH */
		break;
		default:
			DEBUG_I("%s: Not handled errtype %d", __func__, errno);
			ret = 0xDEADFACE;
		break;
	}
	return ret;
}

int32_t FmOpenFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	int32_t retval = 0;
	int32_t mode;
	uint32_t flags = O_RDONLY;

	mode = *(int32_t *)(rx_packet->reqBuf);
	strcpy(nameBuf, mochaRoot);
	strcat(nameBuf, (const char *)(rx_packet->reqBuf + sizeof(mode)));

	if(mode & FM_CREATE)
		flags |= O_CREAT;
	if(mode & FM_WRITE)
		flags |= O_RDWR;
	if(mode & FM_TRUNCATE)
		flags |= O_TRUNC;
	if(mode & FM_APPEND)
		flags |= O_APPEND;
#if 0
	else if(mode & FM_NOSHARE)
		flags |= O_RDWR;
	else if(mode & FM_DIRECTIO)
		flags |= O_RDWR;
	else if(mode & FM_CONCRETE_WRITE)
		flags |= O_RDWR;
	else if(mode & FM_NOUPDATE_TIME)
		flags |= O_RDWR;
#endif
	retval = open(nameBuf, flags, 0660);

	if(retval < 0)
		DEBUG_I("%s: error! %s", __func__, strerror(errno));
	else
		retval |= 0x61000; /* These flags are always present in node returned by Mocha, 
							  we'll mask them out if CP use them anyway */
	tx_packet->funcRet = retval;
	tx_packet->errorVal = (retval < 0 ? (errno == ENOENT ? 0 : FmGetLastError()) : 0);
	/* For some reason Mocha doesn't set error code if there's no specified file, just returns -1 */

	tx_packet->header.packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet);
	tx_packet->respBuf = NULL;

	return 0;
}

int32_t FmCloseFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	int32_t retval = 0;
	int32_t fd;

	fd = *(int32_t *)(rx_packet->reqBuf);
	fd &= 0xFFF;
	
	retval = close(fd);
	
	if(retval < 0)
		DEBUG_I("%s: error! %s", __func__, strerror(errno));
	else
		retval |= 0x61000; /* These flags are always present in node returned by Mocha, 
							  we'll mask them out if CP use them anyway */
	
	tx_packet->errorVal = (retval < 0 ? FmGetLastError() : 0);
	tx_packet->funcRet = (retval < 0 ? 0 : 1); /* returns true/false*/

	tx_packet->header.packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet);
	tx_packet->respBuf = NULL;

	return 0;
}

int32_t FmCreateFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	int32_t retval = 0;
	struct stat sb;
	strcpy(nameBuf, mochaRoot);
	strcat(nameBuf, (const char *)(rx_packet->reqBuf));
	DEBUG_I("%s: fName %s", __func__, nameBuf);

	retval = creat(nameBuf, 0777);
	
	if(retval < 0)
		DEBUG_I("%s: error! %s", __func__, strerror(errno));
		
	tx_packet->errorVal = (retval < 0 ? FmGetLastError() : 0);
	tx_packet->funcRet = retval; /* returns fd */

	tx_packet->header.packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet);
	tx_packet->respBuf = NULL;

	return 0;
}

int32_t FmReadFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	int32_t numRead;
	int32_t fd;
	uint32_t size;
	uint8_t *responseBuf;

	fd = *(int32_t *)(rx_packet->reqBuf);
	fd &= 0xFFF;
	size = *(int32_t *)((rx_packet->reqBuf) + sizeof(fd));

	responseBuf = (uint8_t *)malloc(size + sizeof(numRead));

	numRead = read(fd, (responseBuf + sizeof(numRead)), size);
	
	if(numRead < 0)
		DEBUG_I("%s: error! %s, fd: %d", __func__, strerror(errno), fd);
		
	memcpy(responseBuf, &numRead, sizeof(numRead));

	tx_packet->errorVal = (numRead < 0 ? FmGetLastError() : 0);
	tx_packet->funcRet = (numRead < 0 ? 0 : 1); /* false/true */

	tx_packet->header.packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet) + sizeof(numRead) + numRead;
	tx_packet->respBuf = responseBuf;

	return 0;
}

int32_t FmWriteFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	int32_t numWrite;
	int32_t fd;
	uint32_t size;
	uint8_t *writeBuf;

	fd = *(int32_t *)(rx_packet->reqBuf);
	fd &= 0xFFF;
	size = *(int32_t *)((rx_packet->reqBuf) + sizeof(fd));

	writeBuf = (uint8_t *)((rx_packet->reqBuf) + sizeof(fd) + sizeof(size));

	numWrite = write(fd, writeBuf, size);
	
	if(numWrite < 0)
		DEBUG_I("%s: error! %s, fd: %d", __func__, strerror(errno), fd);
		
	tx_packet->errorVal = (numWrite < 0 ? FmGetLastError() : 0);
	tx_packet->funcRet = (numWrite < 0 ? 0 : 1); /* false/true */

	tx_packet->header.packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet);
	tx_packet->respBuf = NULL;

	return 0;
}

int32_t FmFlushFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	int32_t retval = 0;
	int32_t fd;

	fd = *(int32_t *)(rx_packet->reqBuf);
	fd &= 0xFFF;

	retval = fsync(fd);

	tx_packet->errorVal = (retval < 0 ? FmGetLastError() : 0);
	tx_packet->funcRet = (retval < 0 ? 0 : 1); /* false/true */

	tx_packet->header.packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet);
	tx_packet->respBuf = NULL;

	return 0;
}

int32_t FmSeekFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	int32_t retval = 0;
	int32_t fd;
	uint32_t offset, origin;

	fd = *(int32_t *)(rx_packet->reqBuf);
	fd &= 0xFFF;
	origin = *(int32_t *)((rx_packet->reqBuf) + sizeof(fd));
	offset = *(int32_t *)((rx_packet->reqBuf) + sizeof(fd) + sizeof(origin));

	retval = lseek(fd, offset, origin);

	tx_packet->errorVal = (retval < 0 ? FmGetLastError() : 0);
	tx_packet->funcRet = (retval < 0 ? 0 : 1); /* true/false */

	tx_packet->header.packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet);
	tx_packet->respBuf = NULL;

	return 0;
}

int32_t FmTellFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	int32_t retval = 0;
	int32_t fd;

	fd = *(int32_t *)(rx_packet->reqBuf);
	fd &= 0xFFF;

	retval = lseek(fd, 0, SEEK_CUR);

	tx_packet->errorVal = (retval < 0 ? FmGetLastError() : 0);
	tx_packet->funcRet = retval;

	tx_packet->header.packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet);
	tx_packet->respBuf = NULL;

	return 0;
}

int32_t FmRemoveFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	int32_t retval = 0;
	
	strcpy(nameBuf, mochaRoot);
	strcat(nameBuf, (const char *)(rx_packet->reqBuf));

	retval = remove(nameBuf);

	tx_packet->errorVal = (retval < 0 ? FmGetLastError() : 0);
	tx_packet->funcRet = (retval < 0 ? 0 : 1);

	tx_packet->header.packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet);
	tx_packet->respBuf = NULL;

	return retval;

}

int32_t FmMoveFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	DEBUG_I("Inside FmMoveFile");

	DEBUG_I("Leaving FmMoveFile");
	return 0;
}

/*
 * FIXME: Put proper timestamp in FileAttribute structure
 */
TmDateTime fmTime = {
		.year = 2011,
		.month = 12,
		.day = 29,
		.hour = 10,
		.minute = 45,
		.second = 45,
};

int32_t FmGetFileAttributes(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	int32_t retval = 0;
	struct stat sb;
	FmFileAttribute *fAttr;
	
	strcpy(nameBuf, mochaRoot);
	strcat(nameBuf, (const char *)(rx_packet->reqBuf));
	DEBUG_I("%s: fName %s", __func__, nameBuf);

	retval = stat(nameBuf, &sb);

	fAttr = (FmFileAttribute *)malloc(sizeof(FmFileAttribute));
	memset(fAttr, 0, sizeof(FmFileAttribute));
	
	tx_packet->funcRet = (retval < 0 ? 0 : 1); /* returns true on success */
	tx_packet->errorVal = (retval < 0 ? FmGetLastError() : 0);
	tx_packet->header.packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet) + sizeof(FmFileAttribute);

	if(retval >= 0)
	{
		fAttr->oldFileSize = sb.st_size;
		fAttr->attribute = sb.st_mode;
		fAttr->iVol = sb.st_dev;
		fAttr->dt.year = fmTime.year;
		fAttr->dt.month = fmTime.month;
		fAttr->dt.day = fmTime.day;
		fAttr->dt.hour = fmTime.hour;
		fAttr->dt.minute = fmTime.minute;
		fAttr->dt.second = fmTime.second;
		fAttr->oldAllocatedSize = sb.st_size;
		fAttr->stModifiedDataTime.year = fmTime.year;
		fAttr->stModifiedDataTime.month = fmTime.month;
		fAttr->stModifiedDataTime.day = fmTime.day;
		fAttr->stModifiedDataTime.hour = fmTime.hour;
		fAttr->stModifiedDataTime.minute = fmTime.minute;
		fAttr->stModifiedDataTime.second = fmTime.second;
		fAttr->u64EntryUniqID = sb.st_ino;
		fAttr->fileSize = sb.st_size;
		fAttr->allocatedSize = sb.st_size;
	}
	else
	{
		fAttr->iVol = 0xFFFFFFFF;
	}

	tx_packet->respBuf = (uint8_t *)fAttr;

	return 0;
}

int32_t FmFGetFileAttributes(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	int32_t retval = 0;
	struct stat sb;
	int32_t fd;
	FmFileAttribute *fAttr;

	fd = *(int32_t *)(rx_packet->reqBuf);
	fd &= 0xFFF;

	retval = fstat(fd, &sb);

	fAttr = malloc(sizeof(FmFileAttribute));
	memset(fAttr, 0, sizeof(FmFileAttribute));
	
	tx_packet->funcRet = (retval < 0 ? 0 : 1); /* returns true on success */
	tx_packet->errorVal = (retval < 0 ? FmGetLastError() : 0);
	tx_packet->header.packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet) + sizeof(FmFileAttribute);

	if(retval >= 0)
	{
		fAttr->oldFileSize = sb.st_size;
		fAttr->attribute = sb.st_mode;
		fAttr->iVol = sb.st_dev;
		fAttr->dt.year = fmTime.year;
		fAttr->dt.month = fmTime.month;
		fAttr->dt.day = fmTime.day;
		fAttr->dt.hour = fmTime.hour;
		fAttr->dt.minute = fmTime.minute;
		fAttr->dt.second = fmTime.second;
		fAttr->oldAllocatedSize = sb.st_size;
		fAttr->stModifiedDataTime.year = fmTime.year;
		fAttr->stModifiedDataTime.month = fmTime.month;
		fAttr->stModifiedDataTime.day = fmTime.day;
		fAttr->stModifiedDataTime.hour = fmTime.hour;
		fAttr->stModifiedDataTime.minute = fmTime.minute;
		fAttr->stModifiedDataTime.second = fmTime.second;
		fAttr->u64EntryUniqID = sb.st_ino;
		fAttr->fileSize = sb.st_size;
		fAttr->allocatedSize = sb.st_size;
	}
	else
	{
		DEBUG_I("%s: error! %s", __func__, strerror(errno));
		fAttr->iVol = 0xFFFFFFFF;
	}

	tx_packet->respBuf = (uint8_t *)fAttr;

	return retval;
}

/* Doesn't seem to occur in normal communication, maybe there's no need to implement it. */
int32_t FmSetFileAttributes(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	DEBUG_I("Inside FmSetFileAttributes - TBD");
	tx_packet->errorVal = 0xDEADFACE;
	tx_packet->funcRet = 0; /* false */
	tx_packet->header.packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet);
	tx_packet->respBuf = NULL;
	DEBUG_I("Leaving FmSetFileAttributes");
	return 0;
}

int32_t FmTruncateFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	int32_t retval = 0;
	int32_t fd;
	uint32_t length;

	fd = *(int32_t *)(rx_packet->reqBuf);
	fd &= 0xFFF;
	length = *(int32_t *)((rx_packet->reqBuf) + sizeof(fd));

	retval = ftruncate(fd, length);

	if(retval < 0)
		DEBUG_I("%s: error! %s, fd: %d", __func__, strerror(errno), fd);
		
	tx_packet->errorVal = (retval < 0 ? FmGetLastError() : 0);
	tx_packet->funcRet = (retval < 0 ? 0 : 1); /* true/false */

	tx_packet->header.packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet);
	tx_packet->respBuf = NULL;

	return 0;
}

int32_t FmOpenDir(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	DEBUG_I("Inside FmOpenDir");
	int32_t retval = 0;
	DIR * dir;
	uint8_t *payload;
	
	strcpy(nameBuf, mochaRoot);
	strcat(nameBuf, (const char *)(rx_packet->reqBuf));
	DEBUG_I("%s: fName %s", __func__, nameBuf);
	dir = opendir(nameBuf);

	if(dir)	{
		tx_packet->errorVal = 0;
		tx_packet->funcRet = dirIndex;
		
		dirArray[dirIndex++] = dir;
		if(dirIndex == MAX_OPEN_DIRS-1)
			dirIndex = 0;
	} else {		
		DEBUG_I("%s: failed to open %s, error: %s", __func__, nameBuf, strerror(errno));
		tx_packet->errorVal = FmGetLastError();
		tx_packet->funcRet = -1;
	}

	tx_packet->header.packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet);
	tx_packet->respBuf = NULL;

	DEBUG_I("Leaving FmOpenDir");
	return 0;
}

int32_t FmCloseDir(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	DEBUG_I("Inside FmCloseDir");

	int32_t retval = 0;
	uint8_t *payload;
	int32_t fd;

	fd = *(int32_t *)(rx_packet->reqBuf);
	retval = closedir(dirArray[fd]);

	tx_packet->errorVal = (retval < 0 ? FmGetLastError() : 0);
	tx_packet->funcRet = (retval < 0 ? 0 : 1); /* false/true */

	tx_packet->header.packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet);
	tx_packet->respBuf = NULL;

	DEBUG_I("Leaving FmCloseDir");
	return 0;
}

int32_t FmReadDir(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	int32_t retval = 0;
	struct dirent * dir;
	int32_t fd;

	fd = *(int32_t *)(rx_packet->reqBuf);
	fd &= 0xFFF;
	//dir = readdir((DIR *)dirArray[fd]);

	// TODO :
	// What to return if the directory is not empty?

	/* Response has 0x660 bytes + 8bytes header */
	
	tx_packet->errorVal = 0;
	tx_packet->funcRet = 0;

	tx_packet->header.packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet);
	tx_packet->respBuf = NULL;

	return 0;
}

int32_t FmCreateDir(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	int32_t retval = 0;

	strcpy(nameBuf, mochaRoot);
	strcat(nameBuf, (const char *)(rx_packet->reqBuf));
	DEBUG_I("%s: fName %s", __func__, nameBuf);

	retval = mkdir(nameBuf, 0777);

	if(retval < 0)
		DEBUG_I("error creating directory %s, error: %s", nameBuf, strerror(errno));
		
	tx_packet->funcRet = (retval < 0 ? 0 : 1); /* false/true */
	tx_packet->errorVal = (retval < 0 ? FmGetLastError() : 0);

	tx_packet->header.packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet);
	tx_packet->respBuf = NULL;

	return retval;
}

int32_t FmRemoveDir(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	DEBUG_I("Inside FmRemoveDir");
	/* Response has 0x38 bytes + 8bytes header */
	DEBUG_I("Leaving FmRemoveDir");
	return 0;
}

int32_t FmGetQuotaSpace(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	DEBUG_I("Inside FmGetQuotaSpace");

	DEBUG_I("Leaving FmGetQuotaSpace");
	return 0;
}

int32_t get_request_packet(void *data, struct fmRequest *rx_packet)
{
	rx_packet->header = *(struct fmPacketHeader *)data;
	rx_packet->reqBuf = (uint8_t *)((uint8_t *)data + sizeof(struct fmPacketHeader));

	return 0;
}

int32_t ipc_parse_fm(struct ipc_client* client, struct modem_io *ipc_frame)
{
	int32_t retval;
	struct fmRequest rx_packet;
	struct fmResponse tx_packet;
	struct modem_io request;
	uint8_t *frame;
	uint8_t *payload;
	int32_t frame_length;

	get_request_packet(ipc_frame->data, &rx_packet);

	tx_packet.header = rx_packet.header;
	retval = fileOps[(tx_packet.header.fmPacketType + 0xEFFFFFFF)](&rx_packet, &tx_packet);

    frame_length = (sizeof(struct fmPacketHeader) + tx_packet.header.packetLen);

	request.magic = ipc_frame->magic;
	request.cmd = ipc_frame->cmd;
	request.datasize = frame_length;

    frame = (uint8_t*)malloc(frame_length);

    *(struct fmPacketHeader*)(frame) = tx_packet.header;

    payload = (frame + sizeof(struct fmPacketHeader));
    *(uint32_t*)(payload) = tx_packet.funcRet;
    payload = (payload + sizeof(tx_packet.funcRet));
    *(uint32_t*)(payload) = tx_packet.errorVal;
    payload = (payload + sizeof(tx_packet.errorVal));

    memcpy(payload, tx_packet.respBuf, tx_packet.header.packetLen - (sizeof(tx_packet.errorVal) + sizeof(tx_packet.funcRet)));

	request.data = frame;

	ipc_send(&request);

	if(tx_packet.respBuf != NULL)
        free(tx_packet.respBuf);

    if(frame != NULL)
        free(frame);

    return 0;
}
