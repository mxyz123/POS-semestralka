#ifdef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE_OLD _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif

#define _POSIX_C_SOURCE 199309L
#include <time.h>

void microsleep(unsigned long micros);

#undef _POSIX_C_SOURCE

#ifdef  _POSIX_C_SOURCE_OLD
#define _POSIX_C_SOURCE _POSIX_C_SOURCE_OLD
#undef  _POSIX_C_SOURCE_OLD
#endif