/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 * Copyright (C) 2011-2012 Paul Kocialkowski <contact@paulk.fr>
 *
 * mocha-ril is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mocha-ril is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mocha-ril.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>

#define LOG_TAG "RIL-Mocha-UTIL"
#include <utils/Log.h>
#include "util.h"

#include "mocha-ril.h"

#define RIL_CONFIG_PATH "/data/radio/ril_config.bin"

/**
 * List
 */

struct list_head *list_head_alloc(void *data, struct list_head *prev, struct list_head *next)
{
	struct list_head *list;

	list = calloc(1, sizeof(struct list_head));
	if(list == NULL)
		return NULL;

	list->data = data;
	list->prev = prev;
	list->next = next;

	if(prev != NULL)
		prev->next = list;
	if(next != NULL)
		next->prev = list;

	return list;
}

void list_head_free(struct list_head *list)
{
	if(list == NULL)
		return;

	if(list->next != NULL)
		list->next->prev = list->prev;
	if(list->prev != NULL)
		list->prev->next = list->next;

	memset(list, 0, sizeof(struct list_head));
	free(list);
}

/**
 * Converts GSM7 (8 bits) data to ASCII (7 bits)
 */
size_t gsm72ascii(unsigned char *gsm7, char **ascii, size_t size)
{
	int t, u, d, o = 0;
	size_t i;

	int ascii_length;
	char *dec;

	ascii_length = ((size * 8) - ((size * 8) % 7) ) / 7;

	dec = malloc(ascii_length);
	memset(dec, 0, ascii_length);

	for(i = 0 ; i < size ; i++)
	{
		d = 7 - i % 7;
		if(d == 7 && i != 0)
			o++;

		t = (gsm7[i] - (((gsm7[i] >> d) & 0xff) << d));
		u = (gsm7[i] >> d) & 0xff;

		dec[i + o]+=t << (i + o) % 8;

		if(u)
			dec[i + 1 + o]+=u;
	}

	*ascii = dec;

	return ascii_length;
}

/**
 * Converts ASCII (7 bits) data to GSM7 (8 bits)
 */
size_t ascii2gsm7(char *ascii, unsigned char **gsm7, size_t size)
{
	int d_off, d_pos, a_off, a_pos = 0;
	size_t i;

	int gsm7_length;
	unsigned char *enc;

	gsm7_length = ((size * 7) - (size * 7) % 8) / 8;
	gsm7_length += (size * 7) % 8 > 0 ? 1 : 0;

	enc = malloc(gsm7_length);
	memset(enc, 0, gsm7_length);

	for (i = 0 ; i < size ; i++)
	{
		// offset from the right of data to keep
		d_off = i % 8;

		// position of the data we keep
		d_pos = ((i * 7) - (i * 7) % 8) / 8;
		d_pos += (i * 7) % 8 > 0 ? 1 : 0;

		// adding the data with correct offset
		enc[d_pos] |= ascii[i] >> d_off;

		// numbers of bits to omit to get data to add another place
		a_off = 8 - d_off;

		// position (on the encoded feed) of the data to add
		a_pos = d_pos - 1;

		// adding the data to add at the correct position
		enc[a_pos] |= ascii[i] << a_off;
	}

	*gsm7 = enc;

	return gsm7_length;
}

void hex_dump(void *data, int size)
{
	/* dumps size bytes of *data to stdout. Looks like:
	 * [0000] 75 6E 6B 6E 6F 77 6E 20
	 *				  30 FF 00 00 00 00 39 00 unknown 0.....9.
	 * (in a single line of course)
	 */

	unsigned char *p = data;
	unsigned char c;
	int n;
	char bytestr[4] = {0};
	char addrstr[10] = {0};
	char hexstr[ 16*3 + 5] = {0};
	char charstr[16*1 + 5] = {0};
	for(n=1;n<=size;n++) {
		if (n%16 == 1) {
			/* store address for this line */
			snprintf(addrstr, sizeof(addrstr), "%.4x",
			   ((unsigned int)p-(unsigned int)data) );
		}

		c = *p;
		if (isalnum(c) == 0) {
			c = '.';
		}

		/* store hex str (for left side) */
		snprintf(bytestr, sizeof(bytestr), "%02X ", *p);
		strncat(hexstr, bytestr, sizeof(hexstr)-strlen(hexstr)-1);

		/* store char str (for right side) */
		snprintf(bytestr, sizeof(bytestr), "%c", c);
		strncat(charstr, bytestr, sizeof(charstr)-strlen(charstr)-1);

		if(n%16 == 0) {
			/* line completed */
			ALOGD("[%4.4s]   %-50.50s  %s", addrstr, hexstr, charstr);
			hexstr[0] = 0;
			charstr[0] = 0;
		} else if(n%8 == 0) {
			/* half line: add whitespaces */
			strncat(hexstr, "  ", sizeof(hexstr)-strlen(hexstr)-1);
			strncat(charstr, " ", sizeof(charstr)-strlen(charstr)-1);
		}
		p++; /* next byte */
	}

	if (strlen(hexstr) > 0) {
		/* print rest of buffer if not empty */
		ALOGD("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
	}
}

/* writes the utf8 character encoded in v
 * to the buffer utf8 at the specified offset
 */
int utf8_write(char *utf8, int offset, int v)
{

	int result;

	if (v < 0x80) {
		result = 1;
		if (utf8)
			utf8[offset] = (char)v;
	} else if (v < 0x800) {
		result = 2;
		if (utf8) {
			utf8[offset + 0] = (char)(0xc0 | (v >> 6));
			utf8[offset + 1] = (char)(0x80 | (v & 0x3f));
		}
	} else if (v < 0x10000) {
		result = 3;
		if (utf8) {
			utf8[offset + 0] = (char)(0xe0 | (v >> 12));
			utf8[offset + 1] = (char)(0x80 | ((v >> 6) & 0x3f));
			utf8[offset + 2] = (char)(0x80 | (v & 0x3f));
		}
	} else {
		result = 4;
		if (utf8) {
			utf8[offset + 0] = (char)(0xf0 | ((v >> 18) & 0x7));
			utf8[offset + 1] = (char)(0x80 | ((v >> 12) & 0x3f));
			utf8[offset + 2] = (char)(0x80 | ((v >> 6) & 0x3f));
			utf8[offset + 3] = (char)(0x80 | (v & 0x3f));
		}
	}
	return result;
}

int tun_alloc(char *dev, int flags)
{
	struct ifreq ifr;
	int fd, err;
	char *clonedev = "/dev/tun";

	/* Arguments taken by the function:
	*
	* char *dev: the name of an interface (or '\0'). MUST have enough
	*   space to hold the interface name if '\0' is passed
	* int flags: interface flags (eg, IFF_TUN etc.)
	*/

	/* open the clone device */
	if( (fd = open(clonedev, O_RDWR)) < 0 ) {
		return fd;
	}
	ALOGD("Clonedevice %s opened with fd %d, trying to create %s with flags 0x%X", clonedev, fd, dev, flags);

	/* preparation of the struct ifr, of type "struct ifreq" */
	memset(&ifr, 0, sizeof(ifr));

	ifr.ifr_flags = flags;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */

	if (*dev) {
		/* if a device name was specified, put it in the structure; otherwise,
		* the kernel will try to allocate the "next" device of the
		* specified type */
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	}

	/* try to create the device */
	if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
		close(fd);
		return err;
	}

	/* if the operation was successful, write back the name of the
	* interface to the variable "dev", so the caller can know
	* it. Note that the caller MUST reserve space in *dev (see calling
	* code below) */
	strcpy(dev, ifr.ifr_name);

	/* this is the special file descriptor that the caller will use to talk
	* with the virtual interface */
	return fd;
}

void load_default_ril_config()
{
	ril_data.config.bAutoAttach = 1;
}

/* Return 0 in case of success, non-zero in case of failure */
int load_ril_config()
{
	int fd, n;
	load_default_ril_config();

	if( (fd = open(RIL_CONFIG_PATH, O_RDONLY)) < 0 ) {
		return fd;
	}

	n = read(fd, &ril_data.config, sizeof(ril_config));
	if(n != sizeof(ril_config))
		goto error;
	ALOGD("%s: Read %d bytes from %s", __func__, n, RIL_CONFIG_PATH);
	close(fd);
	return 0;
error:
	ALOGE("%s: Read only %d of %d bytes from %s", __func__, n, sizeof(ril_config), RIL_CONFIG_PATH);
	close(fd);
	return -1;
}

/* Return 0 in case of success, non-zero in case of failure */
int save_ril_config()
{
	int fd, n;

	if( (fd = open(RIL_CONFIG_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0 ) {
		ALOGE("%s: Couldn't open %s for writing, errno: %d", __func__, RIL_CONFIG_PATH, errno);
		return fd;
	}

	n = write(fd, &ril_data.config, sizeof(ril_config));
	if(n != sizeof(ril_config))
		goto error;
	ALOGD("%s: Written %d bytes to %s", __func__, n, RIL_CONFIG_PATH);
	close(fd);
	return 0;
error:
	ALOGE("%s: Wrote only %d of %d bytes to %s", __func__, n, sizeof(ril_config), RIL_CONFIG_PATH);
	close(fd);
	return -1;
}

size_t data2string_length(const void *data, size_t size)
{
	size_t length;

	if (data == NULL || size == 0)
		return 0;

	length = size * 2 + 1;

	return length;
}

char *data2string(const void *data, size_t size)
{
	char *string;
	size_t length;
	char *p;
	size_t i;

	if (data == NULL || size == 0)
		return NULL;

	length = data2string_length(data, size);
	if (length == 0)
		return NULL;

	string = (char *) calloc(1, length);

	p = string;

	for (i = 0; i < size; i++) {
		sprintf(p, "%02x", *((unsigned char *) data + i));
		p += 2 * sizeof(char);
	}

	return string;
}

size_t string2data_size(const char *string)
{
	size_t length;
	size_t size;

	if (string == NULL)
		return 0;

	length = strlen(string);
	if (length == 0)
		return 0;

	if (length % 2 == 0)
		size = length / 2;
	else
		size = (length - (length % 2)) / 2 + 1;

	return size;
}

void *string2data(const char *string)
{
	void *data;
	size_t size;
	size_t length;
	int shift;
	unsigned char *p;
	unsigned int b;
	size_t i;
	int rc;

	if (string == NULL)
		return NULL;

	length = strlen(string);
	if (length == 0)
		return NULL;

	if (length % 2 == 0)
		shift = 0;
	else
		shift = 1;

	size = string2data_size(string);
	if (size == 0)
		return NULL;

	data = calloc(1, size);

	p = (unsigned char *) data;

	for (i = 0; i < length; i++) {
		rc = sscanf(&string[i], "%01x", &b);
		if (rc < 1)
			b = 0;

		if ((shift % 2) == 0)
			*p |= ((b & 0x0f) << 4);
		else
			*p++ |= b & 0x0f;

		shift++;
	}

	return data;
}

int nmea_put_checksum(char *pNmea, int maxSize)
{
	uint8_t checksum = 0;
	int length = 0;

	pNmea++; //skip the $
	while (*pNmea != '\0')
	{
		checksum ^= *pNmea++;
		length++;
	}

	int checksumLength = snprintf(pNmea,(maxSize-length-1),"*%02X\r\n", checksum);
	return (length + checksumLength);
}