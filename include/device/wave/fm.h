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
 
#ifndef __DEVICE_WAVE_FM_H__
#define __DEVICE_WAVE_FM_H__

typedef struct
{
	int32_t		year; 	/**<Year (1900[1900_BASE] or 1970[1970_BASE] ~ 2030)*/
	int32_t		month;	/**<Month (1-12)*/
	int32_t		day;	/**<Day (1-31)*/
	int32_t		hour;	/**<Hour (1-12)*/
	int32_t		minute; 	/**<Minute (1-60)*/
	int32_t		second;	/**<Second (1-60)*/
}  __attribute__ ((packed)) TmDateTime;

/* As per Jet'ish headers + S8500 APPS analysis */
typedef struct
{
	uint32_t		oldFileSize;		// for backward compatibilty with previous PC tools
	uint32_t		startAddr;			/** Only used at LFS*/
	uint32_t		attribute;			/** file attribute like directory or file, hidden, readonly, system, ...	*/
	int32_t			iVol;				/** positioned volume*/
	TmDateTime		dt;					/** Creation date/time*/
	uint32_t		oldAllocatedSize;	// for backward compatibilty with previous PC tools
	TmDateTime		stModifiedDataTime;	/** DON NOT USE THIS FIELD YET!!!*/
	uint64_t		u64EntryUniqID;		/** uniq ID for file or directory */
	uint32_t		unknownField1;
	uint32_t		unknownField2;
	uint32_t		fileSize;			/** File Size in bytes */
	uint32_t		allocatedSize;	/** real allocated size of file & directory in sub System
									*Note: allocated size for sub directories & sub files are not included
										only allocation size for directory itself!!!
									*/
	uint32_t		unknownField3;
}  __attribute__ ((packed)) FmFileAttribute;

#endif