/*
 * Copyright (c) 2007-2015, Cameron Rich
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 * * Neither the name of the axTLS project nor the names of its contributors 
 *   may be used to endorse or promote products derived from this software 
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file os_port.h
 *
 * Some stuff to minimise the differences between windows and linux/unix
 */

#ifndef HEADER_OS_PORT_H
#define HEADER_OS_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

//#include "os_int.h"
#include <stdio.h>
#include "debug_progmem.h"

#ifdef WIN32
#define STDCALL                 __stdcall
#define EXP_FUNC                __declspec(dllexport)
#else
#define STDCALL
#define EXP_FUNC
#endif

#if defined(_WIN32_WCE)
#undef WIN32
#define WIN32
#endif

#if defined(ESP8266)

//(aes_str[0], aes_str_0_ram, 24);
#define system_get_string_from_flash(src, dest, sz) \
		({ \
			strncpy_P((dest), (sz), (src)); \
		})

extern int ax_port_read(int clientfd, uint8_t *buf, int bytes_needed);
extern int ax_port_write(int clientfd, uint8_t *buf, uint16_t bytes_needed);

#include "util/time.h"
extern void gettimeofday(struct timeval* t,void* timezone);


#include <errno.h>
// #define alloca(size) __builtin_alloca(size)
#define TTY_FLUSH()
#ifdef putc
#undef putc
#endif
#define putc(x, f)   m_printf("%c", x)
#ifdef printf
#undef printf
#endif

#define printf  LOG_I

#define SOCKET_READ(A,B,C)      ax_port_read(A,B,C)
#define SOCKET_WRITE(A,B,C)     ax_port_write(A,B,C)
#define SOCKET_CLOSE(A)         ax_port_close(A)
#define get_file                ax_get_file
#define EWOULDBLOCK EAGAIN

#define hmac_sha1 ax_hmac_sha1
#define hmac_md5 ax_hmac_md5

#elif defined(WIN32)

/* Windows CE stuff */
#if defined(_WIN32_WCE)
#include <basetsd.h>
#define abort()                 exit(1)
#else
#include <io.h>
#include <process.h>
#include <sys/timeb.h>
#include <fcntl.h>
#endif      /* _WIN32_WCE */

#include <winsock.h>
#include <direct.h>
#undef getpid
#undef open
#undef close
#undef sleep
#undef gettimeofday
#undef dup2
#undef unlink

#define SOCKET_READ(A,B,C)      recv(A,B,C,0)
#define SOCKET_WRITE(A,B,C)     send(A,B,C,0)
#define SOCKET_CLOSE(A)         closesocket(A)
#define srandom(A)              srand(A)
#define random()                rand()
#define getpid()                _getpid()
#define snprintf                _snprintf
#define open(A,B)               _open(A,B)
#define dup2(A,B)               _dup2(A,B)
#define unlink(A)               _unlink(A)
#define close(A)                _close(A)
#define read(A,B,C)             _read(A,B,C)
#define write(A,B,C)            _write(A,B,C)
#define sleep(A)                Sleep(A*1000)
#define usleep(A)               Sleep(A/1000)
#define strdup(A)               _strdup(A)
#define chroot(A)               _chdir(A)
#define chdir(A)                _chdir(A)
#define alloca(A)               _alloca(A)
#ifndef lseek
#define lseek(A,B,C)            _lseek(A,B,C)
#endif

/* This fix gets around a problem where a win32 application on a cygwin xterm
   doesn't display regular output (until a certain buffer limit) - but it works
   fine under a normal DOS window. This is a hack to get around the issue - 
   see http://www.khngai.com/emacs/tty.php  */
#define TTY_FLUSH()             if (!_isatty(_fileno(stdout))) fflush(stdout);

/*
 * automatically build some library dependencies.
 */
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "AdvAPI32.lib")

typedef int socklen_t;

EXP_FUNC void STDCALL gettimeofday(struct timeval* t,void* timezone);
EXP_FUNC int STDCALL strcasecmp(const char *s1, const char *s2);
EXP_FUNC int STDCALL getdomainname(char *buf, int buf_size);

#else   /* Not Win32 */

#include <unistd.h>
#include <pwd.h>
#include <netdb.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOCKET_READ(A,B,C)      read(A,B,C)
#define SOCKET_WRITE(A,B,C)     write(A,B,C)
#define SOCKET_CLOSE(A)         if (A >= 0) close(A)
#define TTY_FLUSH()

#endif  /* Not Win32 */

/* some functions to mutate the way these work */
#ifdef MEMLEAK_DEBUG
#define malloc(A)       ax_port_malloc(A, __FILE__, __LINE__)
#ifndef realloc
#define realloc(A,B)    ax_port_realloc(A,B, __FILE__, __LINE__)
#endif
#define calloc(A,B)     ax_port_calloc(A,B, __FILE__, __LINE__)
#else /*MEMLEAK_DEBUG*/
#define malloc(A)       ax_port_malloc(A)
#ifndef realloc
#define realloc(A,B)    ax_port_realloc(A,B)
#endif
#define calloc(A,B)     ax_port_calloc(A,B)
#endif /*MEMLEAK_DEBUG*/

#define free(x)         ax_port_free(x)

EXP_FUNC void * STDCALL ax_port_malloc(size_t s
#ifdef MEMLEAK_DEBUG
		, const char*, int
#endif
);

EXP_FUNC void * STDCALL ax_port_realloc(void *y, size_t s
#ifdef MEMLEAK_DEBUG
		, const char*, int
#endif
);

EXP_FUNC void * STDCALL ax_port_calloc(size_t n, size_t s
#ifdef MEMLEAK_DEBUG
		, const char*, int
#endif
);
EXP_FUNC void * STDCALL ax_port_free(void*);
EXP_FUNC int STDCALL ax_open(const char *pathname, int flags);


// gettimeofday
// mktime
// ctime
// time



// ================================================

extern char system_get_data_of_array_8(const char *ps);
#define _system_get_data_of_array_8(a,b) system_get_data_of_array_8(&a[b])



inline uint32_t htonl(uint32_t n){
  return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000UL) >> 8) |
    ((n & 0xff000000UL) >> 24);
}

#define ntohl htonl

#define tls_ntohl ntohl

#ifdef CONFIG_PLATFORM_LINUX
void exit_now(const char *format, ...) __attribute((noreturn));
#else
void exit_now(const char *format, ...);
#endif

#ifndef be16toh
#define be16toh(x) ((uint16)tls_ntohs((uint16)(x)))
#endif

#ifndef htobe16
#define htobe16(x) ((uint16)tls_htons((uint16)(x)))
#endif

#ifndef be32toh
#define be32toh(x) ((uint32)tls_ntohl((uint32)(x)))
#endif

#ifndef htobe32
#define htobe32(x) ((uint32)tls_htonl((uint32)(x)))
#endif

#ifndef be64toh
static __inline__ uint64 be64toh(uint64 __x);
static __inline__ uint64 be64toh(uint64 __x) {return (((uint64)be32toh(__x & (uint64)0xFFFFFFFFULL)) << 32) | ((uint64)be32toh((__x & (uint64)0xFFFFFFFF00000000ULL) >> 32));}
#define be64toh(x) be64toh(x)
#endif

#ifndef htobe64
#define htobe64(x) be64toh(x)
#endif

/* Mutexing definitions */
#if defined(CONFIG_SSL_CTX_MUTEXING)
#if defined(WIN32)
#define SSL_CTX_MUTEX_TYPE          HANDLE
#define SSL_CTX_MUTEX_INIT(A)       A=CreateMutex(0, FALSE, 0)
#define SSL_CTX_MUTEX_DESTROY(A)    CloseHandle(A)
#define SSL_CTX_LOCK(A)             WaitForSingleObject(A, INFINITE)
#define SSL_CTX_UNLOCK(A)           ReleaseMutex(A)
#else 
#include <pthread.h>
#define SSL_CTX_MUTEX_TYPE          pthread_mutex_t
#define SSL_CTX_MUTEX_INIT(A)       pthread_mutex_init(&A, NULL)
#define SSL_CTX_MUTEX_DESTROY(A)    pthread_mutex_destroy(&A)
#define SSL_CTX_LOCK(A)             pthread_mutex_lock(&A)
#define SSL_CTX_UNLOCK(A)           pthread_mutex_unlock(&A)
#endif
#else   /* no mutexing */
#define SSL_CTX_MUTEX_INIT(A)
#define SSL_CTX_MUTEX_DESTROY(A)
#define SSL_CTX_LOCK(A)
#define SSL_CTX_UNLOCK(A)
#endif

#ifdef __cplusplus
}
#endif

#endif 
