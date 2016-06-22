/*
 * Memory functions for ESP8266 taken from Arduino-Esp project

  WiFiClientSecure.cpp - Variant of WiFiClient with TLS support
  Copyright (c) 2015 Ivan Grokhotkov. All rights reserved.
  This file is part of the esp8266 core for Arduino environment.


  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include "debug_progmem.h"

extern void *pvPortMalloc( size_t xWantedSize );
extern void vPortFree( void *pv );
extern void *pvPortZalloc(size_t size);
extern void *ets_memset(void *s, int c, size_t n);
extern size_t xPortGetFreeHeapSize(void);
extern void *pvPortCalloc(unsigned int n, unsigned int count);
extern void *pvPortRealloc(void * p, size_t size);

#define free vPortFree
#define malloc pvPortMalloc
#define realloc pvPortRealloc
#define memset ets_memset
#define system_get_free_heap_size xPortGetFreeHeapSize

#ifdef MEMLEAK_DEBUG
#define DEBUG_TLS_MEM_PRINT LOG_I
#endif

void* ax_port_malloc(size_t size
#ifdef MEMLEAK_DEBUG
		, const char* file, int line
#endif
) {
    void* result = (void *)malloc(size);

    if (result == NULL) {
#ifdef MEMLEAK_DEBUG
        DEBUG_TLS_MEM_PRINT("%s:%d malloc %d failed, left %d\r", file, line, size, system_get_free_heap_size());
#endif
        while(true){}
    }
#ifdef MEMLEAK_DEBUG
    DEBUG_TLS_MEM_PRINT("%s:%d malloc %d => %x left %d\r", file, line, size, (uint32_t)result, system_get_free_heap_size());
#endif
    return result;
}

void* ax_port_calloc(size_t size, size_t count
#ifdef MEMLEAK_DEBUG
		, const char* file, int line
#endif
) {
    void* result = (void* )ax_port_malloc(size * count
#ifdef MEMLEAK_DEBUG
    													, file, line
#endif
    );
    memset(result, 0, size * count);
    return result;
}

void* ax_port_realloc(void* ptr, size_t size
#ifdef MEMLEAK_DEBUG
		, const char* file, int line
#endif
) {
    void* result = (void* )realloc(ptr, size);
    if (result == NULL) {
#ifdef MEMLEAK_DEBUG
        DEBUG_TLS_MEM_PRINT("%s:%d realloc %d failed, left %d\r", file, line, size, system_get_free_heap_size());
#endif
        while(true){}
    }

#ifdef MEMLEAK_DEBUG
    DEBUG_TLS_MEM_PRINT("%s:%d realloc %d=>%x, left %d\r", file, line, size, result, system_get_free_heap_size());
#endif
    return result;
}

void ax_port_free(void* ptr) {
    free(ptr);

    //    DEBUG_TLS_MEM_PRINT("free %x, left %d\r", p[-3], system_get_free_heap_size());
}