/* Declarations for common convenience functions.
   Copyright (C) 2006-2011 Red Hat, Inc.
   Copyright (C) 2022 Mark J. Wielaard <mark@klomp.org>
   Copyright (C) 2023 Khem Raj.
   This file is part of elfutils.

   This file is free software; you can redistribute it and/or modify
   it under the terms of either

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at
       your option) any later version

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at
       your option) any later version

   or both in parallel, as here.

   elfutils is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see <http://www.gnu.org/licenses/>.  */

#ifndef LIB_SYSTEM_H
#define LIB_SYSTEM_H	1

/* Prevent double inclusion of config.h, config.h includes eu-config.h.  */
#ifdef HAVE_CONFIG_H
#ifndef EU_CONFIG_H
# include <config.h>
#endif
#endif

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include <byteswap.h>
#include <endian.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <unistd.h>

#if BYTE_ORDER == LITTLE_ENDIAN
# define LE32(n)	(n)
# define LE64(n)	(n)
# define BE32(n)	bswap_32 (n)
# define BE64(n)	bswap_64 (n)
#elif BYTE_ORDER == BIG_ENDIAN
# define BE32(n)	(n)
# define BE64(n)	(n)
# define LE32(n)	bswap_32 (n)
# define LE64(n)	bswap_64 (n)
#else
# error "Unknown byte order"
#endif

#ifndef MAX
#define MAX(m, n) ((m) < (n) ? (n) : (m))
#endif

#ifndef MIN
#define MIN(m, n) ((m) < (n) ? (m) : (n))
#endif

#if !HAVE_DECL_POWEROF2
#define powerof2(x) (((x) & ((x) - 1)) == 0)
#endif

#if !HAVE_DECL_MEMPCPY
#define mempcpy(dest, src, n) \
    ((void *) ((char *) memcpy (dest, src, n) + (size_t) n))
#endif

#if !HAVE_DECL_REALLOCARRAY
static inline void *
reallocarray (void *ptr, size_t nmemb, size_t size)
{
  if (size > 0 && nmemb > SIZE_MAX / size)
    {
      errno = ENOMEM;
      return NULL;
    }
  return realloc (ptr, nmemb * size);
}
#endif

/* Return TRUE if the start of STR matches PREFIX, FALSE otherwise.  */

static inline int
startswith (const char *str, const char *prefix)
{
  return strncmp (str, prefix, strlen (prefix)) == 0;
}

/* Return TRUE if STR[FROM] is a valid string with a zero terminator
   at or before STR[TO - 1].  Note FROM is an index into the STR
   array, while TO is the maximum size of the STR array.  This
   function returns FALSE when TO is zero or FROM >= TO.  */
static inline bool
validate_str (const char *str, size_t from, size_t to)
{
#if HAVE_DECL_MEMRCHR
  // Check end first, which is likely a zero terminator,
  // to prevent function call
  return (to > 0
	  && (str[to - 1] == '\0'
	      || (to > from
		  && memrchr (&str[from], '\0', to - from - 1) != NULL)));
#else
  do {
    if (to <= from)
      return false;

    to--;
  } while (str[to]);

  return true;
#endif
}

/* A special gettext function we use if the strings are too short.  */
#define sgettext(Str) \
  ({ const char *__res = strrchr (_(Str), '|');			      \
     __res ? __res + 1 : Str; })

#define gettext_noop(Str) Str

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(expression) \
  ({ ssize_t __res; \
     do \
       __res = expression; \
     while (__res == -1 && errno == EINTR); \
     __res; })
#endif

#ifndef ACCESSPERMS
#define ACCESSPERMS (S_IRWXU|S_IRWXG|S_IRWXO) /* 0777 */
#endif

#ifndef ALLPERMS
#define ALLPERMS (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO) /* 07777 */
#endif

#ifndef DEFFILEMODE
#define DEFFILEMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)/* 0666 */
#endif

static inline ssize_t __attribute__ ((unused))
pwrite_retry (int fd, const void *buf, size_t len, off_t off)
{
  ssize_t recvd = 0;

  do
    {
      ssize_t ret = TEMP_FAILURE_RETRY (pwrite (fd, ((char *)buf) + recvd, len - recvd,
						off + recvd));
      if (ret <= 0)
	return ret < 0 ? ret : recvd;

      recvd += ret;
    }
  while ((size_t) recvd < len);

  return recvd;
}

static inline ssize_t __attribute__ ((unused))
write_retry (int fd, const void *buf, size_t len)
{
  ssize_t recvd = 0;

  do
    {
      ssize_t ret = TEMP_FAILURE_RETRY (write (fd, ((char *)buf) + recvd, len - recvd));
      if (ret <= 0)
	return ret < 0 ? ret : recvd;

      recvd += ret;
    }
  while ((size_t) recvd < len);

  return recvd;
}

static inline ssize_t __attribute__ ((unused))
pread_retry (int fd, void *buf, size_t len, off_t off)
{
  ssize_t recvd = 0;

  do
    {
      ssize_t ret = TEMP_FAILURE_RETRY (pread (fd, ((char *)buf) + recvd, len - recvd,
					       off + recvd));
      if (ret <= 0)
	return ret < 0 ? ret : recvd;

      recvd += ret;
    }
  while ((size_t) recvd < len);

  return recvd;
}

/* The demangler from libstdc++.  */
extern char *__cxa_demangle (const char *mangled_name, char *output_buffer,
			     size_t *length, int *status);

/* A static assertion.  This will cause a compile-time error if EXPR,
   which must be a compile-time constant, is false.  */

#define eu_static_assert(expr)						\
  extern int never_defined_just_used_for_checking[(expr) ? 1 : -1]	\
    __attribute__ ((unused))

/* We really want a basename implementation that doesn't modify the
   input argument.  Normally you get that from string.h with _GNU_SOURCE
   define.  But some libc implementations don't define it and other
   define it, but provide an implementation that still modifies the
   argument.  So define our own and poison a bare basename symbol.  */
static inline const char *
xbasename(const char *s)
{
  const char *p = strrchr(s, '/');
  return p ? p+1 : s;
}
#pragma GCC poison basename

#endif /* system.h */
