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
 
#ifndef __DEVICE_JET_FM_H__
#define __DEVICE_JET_FM_H__

typedef struct
{
	int32_t		year; 	/**<Year (1900[1900_BASE] or 1970[1970_BASE] ~ 2030)*/
	int32_t		month;	/**<Month (1-12)*/
	int32_t		day;	/**<Day (1-31)*/
	int32_t		hour;	/**<Hour (1-12)*/
	int32_t		minute; 	/**<Minute (1-60)*/
	int32_t		second;	/**<Second (1-60)*/
} TmDateTime;

struct FmFileAttribute
{
	uint32_t		oldFileSize;		// for backward compatibilty with previous PC tools
	uint32_t*		startAddr;			/** Only used at LFS*/
	uint32_t		attribute;			/** file attribute like directory or file, hidden, readonly, system, ...	*/
	int32_t			iVol;				/** positioned volume*/
	TmDateTime	dt;					/** Creation date/time*/
	uint32_t		oldAllocatedSize;	// for backward compatibilty with previous PC tools
	TmDateTime	stModifiedDataTime;	/** DON NOT USE THIS FIELD YET!!!*/
	uint64_t		u64EntryUniqID;		/** uniq ID for file or directory, data cluster number + ctimeuint32_tigned long		uReservedField;		DON NOT USE THIS FIELD YET!!!*/
	uint32_t		uReservedField;		/** DON NOT USE THIS FIELD YET!!!*/
	uint64_t/*UINT64*/	fileSize;			/** File Size in bytes */
	uint64_t/*UINT64*/	allocatedSize;	/** real allocated size of file & directory in sub System
									*Note: allocated size for sub directories & sub files are not included
										only allocation size for directory itself!!!
									*/
};

#endif