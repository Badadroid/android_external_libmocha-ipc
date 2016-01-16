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

#ifndef _SAMSUNG_RIL_UTIL_H_
#define _SAMSUNG_RIL_UTIL_H_

struct list_head {
	struct list_head *prev;
	struct list_head *next;
	void *data;
};

struct list_head *list_head_alloc(void *data, struct list_head *prev, struct list_head *next);
void list_head_free(struct list_head *list);

size_t gsm72ascii(unsigned char *gsm7, char **ascii, size_t size);
size_t ascii2gsm7(char *ascii, unsigned char **gsm7, size_t size);
void hex_dump(void *data, int size);
int utf8_write(char *utf8, int offset, int v);

int tun_alloc(char *dev, int flags);
void load_default_ril_config(void);
int load_ril_config(void);
int save_ril_config(void);

size_t data2string_length(const void *data, size_t size);
char *data2string(const void *data, size_t size);
size_t string2data_size(const char *string);
void *string2data(const char *string);

int nmea_put_checksum(char *pNmea, int maxSize);

#endif
