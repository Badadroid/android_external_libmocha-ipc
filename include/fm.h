/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2011-2013 KB <kbjetdroid@gmail.com>
 * Copyright (C) 2011-2013 Dominik Marszk <dmarszk@gmail.com>
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
 *
 */

#ifndef __FM_H__
#define __FM_H__

#include <radio.h>

#if defined(DEVICE_JET)
#include "device/jet/fm.h"
#elif defined(DEVICE_WAVE)
#include "device/wave/fm.h"
#endif
/*
 * Declarations copied from Dolphin headers
 */

/* File Manager */
#define FM_FILENAME_LEN_MAX						236
#define FM_FILEPATH_LEN_MAX						256

#ifndef ADDR
typedef unsigned int		ADDR;
#endif // ADDR

//PACKETTYPE 0x6 FmPacket
struct fmPacketHeader {
	uint32_t fmPacketType; 	//stored as type-0xEFFFFFFF, add 0xEFFFFFFF to map to the FM operation id
	uint32_t reserved1; 		//dummy, unused? always equal 1
	uint32_t packetLen; 		//equal to n, equal to packetLength-16 (headersize)
	uint32_t reqCounter; 	// has to be the same in responsepacket, probably fm request counter
}  __attribute__ ((packed));

struct fmRequest {
	struct fmPacketHeader header; 	// has to be the same in responsepacket, probably fm request counter
	uint8_t *reqBuf; 		//usually first comes unsigned int params, and then string containing name
};

struct fmResponse {
	struct fmPacketHeader header; 	// has to be the same in responsepacket, probably fm request counter
	int32_t funcRet; 		//called function return value
	int32_t errorVal; 		//0 if func_ret == 0, otherwise retvalue of platform LastError()
	uint8_t *respBuf;
};

/*
 * Access Modes
 */
#define FM_CREATE				0x0001
#define FM_READ					0x0002
#define FM_WRITE				0x0004
#define FM_TRUNCATE				0x0008
#define FM_APPEND				0x0010
#define FM_NOSHARE				0x0020
#define FM_DIRECTIO				0x0040	// this mode does not use write buffer. so slow
#define FM_CONCRETE_WRITE		0x0080	// this mode use backup writing up to  FM_CONCRETE_WRITE_BUFFER_MAX
										// this mode is 2x more times slower than normal writing
										// but it garantees atomic writing via logging
#define FM_NOUPDATE_TIME 		0x0100   // if this flag is on, directory entry update for last modified time will not occurs.
										// this can increase file system endurance

/**
 *  	  DirectoryEntry Type
 */
typedef enum
{
	FM_INVALID_TYPE =0x0000, 		 /**FM_INVALID_TYPE*/
	FM_FILE_TYPE = 0x0001,			 /**FM_FILE_TYPE*/
	FM_DIR_TYPE = 0x0002,			 /**FM_DIR_TYPE*/	// same as FM_ATTRIBUTE_DIR
	FM_UNKNOWN_TYPE = 0x0003,		  /**FM_UNKNOWN_TYPE*/   // Sent from Qualcomm side
	FM_TYPE_MAX = 0xFF000000		// for enumeration type size
} FmEntryType;

/**
 * file system type
 */
typedef enum
{
	FM_FS_TFS2 = 0,
	FM_FS_TFS3 = 1,
	FM_FS_TFS4 = 2,	// MMC, TFS4
	FM_FS_EFS2 = 3,
	FM_FS_LFS = 4,
	FM_FS_RFS = 5,			// Ram File System
	FM_FS_REMOTE =6,
	FM_FS_WIN32 = 7,
	FM_FS_TCCFS =8,
	FM_FS_MTEKFS=9,
	FM_FS_MAX =10
} FmFsType;

/**
  * @brief		These are basic structures for Directory Attribute
  */
typedef struct
{
	FmEntryType		type;					/** File or Directory*/
	uint32_t		oldFileSize;			// for backward compatibilty with previous PC tools
	uint32_t		attribute;				/** Attribute*/
	TmDateTime		dt;						/** Creation date/time*/
	char			szDummy[FM_FILEPATH_LEN_MAX];	// for backward compatibility with previous PC tools
	uint32_t		oldAllocatedSize;		// for backward compatibilty with previous PC tools
	TmDateTime		stModifiedDataTime;	/** DON NOT USE THIS FIELD YET!!!*/
	uint64_t		u64EntryUniqID;		/** uniq ID for file or directory, data cluster number + ctime mtime*/
	uint32_t 		fileSize;			/** File size if entry is file. meaningless for directory entry */
	uint32_t 		allocatedSize;		/** real allocated size of file or directory in Sub System
										 *Note: allocated size for sub directories & sub files are not included
										  only allocation size for directory itself!!!*/
	char			szName[FM_FILENAME_LEN_MAX];	/** File name*/
	uint32_t		uReservedField;		/** DON NOT USE THIS FIELD YET!!!*/

} __attribute__ ((packed)) FmDirEntry;

/**
  * @brief		These are basic structures for VolumeStat
  */
typedef struct
{
	uint32_t		availableSizeDummy;	/** Dummy for compatibility with  legacy facotry softwrare*/
	uint32_t		allocatedSizeDummy;	/** Dummy for compatibility with  legacy facotry softwrare*/
	FmFsType	fsType;					/** volume type such as FM_FSTYPE_FIXED, FM_FSTYPE_REMOVABLE...*/
	uint64_t		availableSize;			/** available size */
	uint64_t		allocatedSize;			/** actual usage in File Syste*/
} __attribute__ ((packed)) FmVolumeStat;

/**
  * @brief		These are basic structures for QuotaStat
  */
typedef struct
{
	uint32_t			availableSizeDummy;	/**  Dummy for compatibility with  legacy facotry softwrare*/
	uint32_t			allocatedSizeDummy;	/** Dummy for compatibility with  legacy facotry softwrare	*/
	uint32_t			reservedSizeDummy;	/** Dummy for compatibility with  legacy facotry softwrare		*/
	uint32_t			diskSizeDummy;			/** Dummy for compatibility with  legacy facotry softwrare	 */
	uint64_t			availableSize;			/** available size means Reserved area plus remaining Shared area*/
	uint64_t			allocatedSize;			/** actual usage in File System for registered Quota entry*/
	uint64_t			reservedSize;			/** if requested path is not registered. it will be 0*/
	uint64_t			diskSize;				/** fixed size of Quota Area in TFS. This value is a practical meaning as disk size to user. */
	uint64_t			clusterSize;				/** disk Cluster Size. */
} __attribute__ ((packed)) FmQuotaStat;

typedef struct
{
	char pathname[FM_FILEPATH_LEN_MAX];		// null-terminated pathname
	FmEntryType entryType;					// File or Directory
} __attribute__ ((packed)) FmOpenEntry;


/**
  * @brief		These are file operations requested by CP
  */
#define FM_OPENFILE 		0
#define FM_CLOSEFILE 		1
#define FM_CREATEFILE 		2
#define FM_READFILE 		3
#define FM_WRITEFILE 		4
#define FM_FLUSHFILE 		5
#define FM_SEEKFILE 		6
#define FM_TELLFILE 		7
#define FM_REMOVEFILE 		8
#define FM_MOVEFILE 		9
#define FM_GETFILEATTR 		10
#define FM_FGETFILEATTR 	11
#define FM_SETFILEATTR 		12
#define FM_TRUNCATEFILE 	13
#define FM_OPENDIR 			14
#define FM_CLOSEDIR 		15
#define FM_READDIR 			16
#define FM_CREATEDIR 		17
#define FM_REMOVEDIR 		18
#define FM_GETQUOTASPACE 	19
#define FM_INVALID 			20

#define MAX_FILE_OPS 		20

typedef enum
{
	FM_INVALID_PARAMETER	= 0x10003,
	FM_FILE_MAX_OPEN_ERROR	= 0x20003,
	FM_FILE_MAX_EXIST_ERROR	= 0x30003,
	FM_ENTRY_EXIST_ERROR	= 0x40003,
	FM_VOLUME_FULL_ERROR 	= 0x50003,
	FM_ENTRY_NOT_FOUND_ERROR	= 0x60003,
	FM_INVALID_FILE_HANDLE	= 0x70003,
	FM_FILE_NOT_OPENED_ERROR	= 0x80003,
	FM_PERMISSION_ERROR		= 0x90003,
	FM_FILE_OPENED_ERROR 	= 0xA0003,
	FM_INVALID_SIGNATURE_ERROR	= 0xB0003,
	FM_ALREADY_MOUNTED_ERROR	= 0xC0003,
	FM_EOF_ERROR			= 0xD0003,
	FM_DEVICE_ERROR			= 0xF0003,
	FM_INVALID_PATH_ERROR	= 0x130003,
	FM_ECC_ERROR 			= 0x140003,	
} FmErrorType;

int32_t FmGetLastError();

int32_t FmOpenFile(struct fmRequest *, struct fmResponse *);
int32_t FmCloseFile(struct fmRequest *, struct fmResponse *);
int32_t FmCreateFile(struct fmRequest *, struct fmResponse *);
int32_t FmReadFile(struct fmRequest *, struct fmResponse *);
int32_t FmWriteFile(struct fmRequest *, struct fmResponse *);
int32_t FmFlushFile(struct fmRequest *, struct fmResponse *);
int32_t FmSeekFile(struct fmRequest *, struct fmResponse *);
int32_t FmTellFile(struct fmRequest *, struct fmResponse *);
int32_t FmRemoveFile(struct fmRequest *, struct fmResponse *);
int32_t FmMoveFile(struct fmRequest *, struct fmResponse *);
int32_t FmGetFileAttributes(struct fmRequest *, struct fmResponse *);
int32_t FmFGetFileAttributes(struct fmRequest *, struct fmResponse *);
int32_t FmSetFileAttributes(struct fmRequest *, struct fmResponse *);
int32_t FmTruncateFile(struct fmRequest *, struct fmResponse *);
int32_t FmOpenDir(struct fmRequest *, struct fmResponse *);
int32_t FmCloseDir(struct fmRequest *, struct fmResponse *);
int32_t FmReadDir(struct fmRequest *, struct fmResponse *);
int32_t FmCreateDir(struct fmRequest *, struct fmResponse *);
int32_t FmRemoveDir(struct fmRequest *, struct fmResponse *);
int32_t FmGetQuotaSpace(struct fmRequest *, struct fmResponse *);
//int32_t FmInvalidFile(unsigned int mode, const char *fileName);

int32_t ipc_parse_fm(struct ipc_client* client, struct modem_io *ipc_frame);

#endif
