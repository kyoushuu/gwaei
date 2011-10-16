/*@-isoreserved@*/
/*@-isoreservedinternal@*/
/*@-warnmissingglobs@*/
/*@-declundef@*/
/*@-fcnuse@*/

/* stdint.h */

typedef /*@unsignedintegraltype@*/ uint8_t;
typedef /*@unsignedintegraltype@*/ uint32_t;

/* stdlib.h */

typedef /*@unsignedintegraltype@*/ size_t;

/*@null@*/ /*@out@*/ /*@only@*/
void* malloc (size_t size) /*@*/
/*@ensures maxSet(result) == (size - 1) @*/
;

/*@null@*/ /*@only@*/
void* calloc (size_t nobj, size_t size) /*@*/
/*@requires nobj == 1 @*/
/*@ensures maxSet(result) == (size - 1) @*/
;

/*@null@*/
void* realloc (/*@only@*/ /*@out@*/ void *p, size_t size)
/*@modifies *p @*/
/*@ensures maxSet(result) >= (size - 1) @*/
;

void free (/*@null@*/ /*@out@*/ /*@only@*/ void *p)
/*@modifies *p @*/
;

/* string.h */

void /*@alt void* @*/
memcpy (/*@unique@*/ /*@returned@*/ /*@out@*/ void *s1,
	void *s2, size_t n)
/*@modifies *s1 @*/
/*@requires maxRead(s2) >= (n - 1) /\
             maxSet(s1) >= (n - 1) @*/
/*@ensures  maxRead(s1) >= (n - 1) @*/
;

void /*@alt void* @*/
memset(/*@out@*/ /*@returned@*/ void *s, int c, size_t n)
/*@modifies *s @*/
/*@requires maxSet(s) >= (n - 1) @*/
/*@ensures maxRead(s) >= (n - 1) @*/
;

/*@dependent@*/
char* strerror (int errnum) /*@*/ ;

size_t strlen (char *s) /*@*/
/*@ensures result == maxRead(s) @*/
;

/* errno.h */

/*@unchecked@*/ int errno;

/* stdio.h */

/*@constant int EOF; @*/

/*@constant int SEEK_CUR; @*/
/*@constant int SEEK_END; @*/
/*@constant int SEEK_SET; @*/

typedef /*@abstract@*/ /*@mutable@*/ void *FILE;

/*@unchecked@*/ FILE *stderr;
/*@unchecked@*/ FILE *stdin;
/*@unchecked@*/ FILE *stdout;

/*@printflike@*/
int /*@alt void@*/ printf (char *format, ...)
/*@modifies fileSystem, *stdout @*/
;

/*@printflike@*/
int /*@alt void@*/ fprintf (FILE *stream, const char *format, ...)
/*@modifies fileSystem, *stream, errno @*/
;

/*@null@*/
FILE *fopen (const char *filename, const char *mode)
/*@modifies fileSystem, errno @*/
;

int fclose (/*@only@*/ FILE *stream)
/*@modifies fileSystem, *stream, errno @*/
;

int fflush (FILE *stream)
/*@modifies fileSystem, *stream, errno @*/ ;

size_t fread (/*@out@*/ void *ptr, size_t size,
	      size_t nobj, FILE *stream)
/*@modifies fileSystem, *ptr, *stream, errno @*/
/*@requires maxSet(ptr) >= (size - 1) /\
           nobj == 1 @*/
/*@ensures maxRead(ptr) == (size - 1) @*/
;

size_t fwrite (void *ptr, size_t size,
	       size_t nobj, FILE *stream)
/*@modifies fileSystem, *stream, errno @*/
/*@requires maxRead(ptr) >= (size - 1) @*/
;

int fseek (FILE *stream, long int offset, int whence)
/*@modifies fileSystem, *stream, errno @*/
;

long int ftell(FILE *stream)
/*@modifies errno @*/
;

/*@=fcnuse@*/
/*@=declundef@*/
/*@=warnmissingglobs@*/
/*@=isoreservedinternal@*/
/*@=isoreserved@*/
