#ifndef __debugl__
#define __debugl__

#include <stdlib.h>


struct DEBUGL_FUNC {
	void (*debug)(const char *fmt, ...);
	void (*info)(const char *fmt, ...);
	void (*warn)(const char *fmt, ...);
	void (*error)(const char *fmt, ...);
	void (*fatal)(const char *fmt, ...);
};

extern struct DEBUGL_FUNC LOG;


extern char DISABLE_DEBUG_LEVEL_DEBUG;
extern char DISABLE_DEBUG_LEVEL_INFO;
extern char DISABLE_DEBUG_LEVEL_WARN;
extern char DISABLE_DEBUG_LEVEL_FATAL;
extern char DISABLE_DEBUG_LEVEL_ERROR;

struct DEBUGL_FUNC *__debugl(struct DEBUGL_FUNC *debugl);

extern void set_debug_level(unsigned char level);

#define DEBUGL __debugl(&LOG)

//這個文件放在每個文件的最include處，則會把所有rtkprintf，和printf的輸出，重新定義為 debugl.debug 輸出
//#ifndef NO_DEBUGL_DEF
//#define rt_kprintf DEBUGL->debug
//#define printf DEBUGL->debug
//#endif
//如果需要在某一文件中屏蔽該文件則使用 #undef  NO_DEBUGL_DEF   來關閉


#endif

