#include "debugl.h"
#include <stdarg.h>
#include <rtthread.h>

#include "common.h"

char DISABLE_DEBUG_LEVEL_DEBUG = 1;
char DISABLE_DEBUG_LEVEL_INFO = 1;
char DISABLE_DEBUG_LEVEL_WARN = 1;
char DISABLE_DEBUG_LEVEL_FATAL = 1;
char DISABLE_DEBUG_LEVEL_ERROR = 1;

struct DEBUGL_FUNC LOG;
static unsigned char init_debug_flag = 0;
extern 
rt_int32_t rt_vsnprintf(char       *buf,
                        rt_size_t   size,
                        const char *fmt,
                        va_list     args);
												
static void va_printf(int level , const char *fmt, va_list args)
{
	static char log_buffer[256];
	rt_vsnprintf(log_buffer, sizeof(log_buffer) - 1, fmt, args);
	rt_kprintf("%s",log_buffer);
}

static void debug(const char *fmt, ...)
{
	int level = 0;
	va_list args;

	if(DISABLE_DEBUG_LEVEL_DEBUG == 1)
		return;
	
	va_start(args, fmt);
	va_printf(level,fmt,args);
	va_end(args);
	//
}
static void info(const char *fmt, ...)
{
	int level = 1;
	va_list args;;
	if( DISABLE_DEBUG_LEVEL_INFO == 1)
		return;
	va_start(args, fmt);
	va_printf(level,fmt,args);
	va_end(args);
	//
}
static void warn(const char *fmt, ...)
{
	int level = 2;
	va_list args;
	if( DISABLE_DEBUG_LEVEL_WARN == 1)
		return;
	//va_list args;
	va_start(args, fmt);
	va_printf(level,fmt,args);
	va_end(args);
	//
}
static void error(const char *fmt, ...)
{
	int level = 3;
	va_list args;
	if( DISABLE_DEBUG_LEVEL_ERROR == 1)
		return;
	va_start(args, fmt);
	va_printf(level,fmt,args);
	va_end(args);
	//
}
static void fatal(const char *fmt, ...)
{
	int level = 4;
	va_list args;
	if (DISABLE_DEBUG_LEVEL_FATAL == 1)
		return ;
	va_start(args, fmt);
	va_printf(level,fmt,args);
	va_end(args);
	//
}

struct DEBUGL_FUNC *__debugl(struct DEBUGL_FUNC *debugl)
{
	if (init_debug_flag == 1)
	{
		return debugl;
	}
	
	debugl->debug = debug;
	debugl->info = info;
	debugl->warn = warn;
	debugl->error = error;
	debugl->fatal = fatal;
	init_debug_flag = 1;
	return debugl;
}

void set_debug_level(unsigned char level)
{
		if ( level <= 4)
			DISABLE_DEBUG_LEVEL_FATAL		=		0; //4
		else
			DISABLE_DEBUG_LEVEL_FATAL		=		1; //4
		
		if ( level <= 3)
			DISABLE_DEBUG_LEVEL_ERROR		=		0; //3
		else
			DISABLE_DEBUG_LEVEL_ERROR		=		1; //3
		
		if ( level <= 2)
			DISABLE_DEBUG_LEVEL_WARN		=		0; //2
		else
			DISABLE_DEBUG_LEVEL_WARN		=		1; //2
		
		if ( level <= 1)
			DISABLE_DEBUG_LEVEL_INFO		=		0; //1
		else
			DISABLE_DEBUG_LEVEL_INFO		=		1; //1
		
		if ( level <= 0)
			DISABLE_DEBUG_LEVEL_DEBUG		=		0; //0
		else
			DISABLE_DEBUG_LEVEL_DEBUG		=		1; //0
	
}