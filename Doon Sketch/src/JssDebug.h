//
//  JssDebug.h
//  Matting
//
//  Created by Zhang Guangjian on 9/4/13.
//  Copyright (c) 2013 Jobssoft Studio. All rights reserved.
//

// If used along with CHDebug.h , Must be included after CHDebug.h

#ifndef JssDebug_h
#define JssDebug_h

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifndef DEBUG
#undef JSS_MSG_SWITCH
#define JSS_MSG_SWITCH  (0)
#endif

#undef DBG_D
#undef DBG_I
#undef DBG_T
#undef DBG_W
#undef DBG_E
#undef DBG_MSG
#undef BEGIN_FUNC
#undef END_FUNC
#undef EXIT

#define IS_STR_NIL(s)   (((s) == nil) || [(s) isEqualToString:@""])
#define IS_STR_NOT_NIL(s)   (!IS_STR_NIL(s))

#define DBG_MSG(level, msg, ...) \
do \
{ \
    if (JSS_MSG_SWITCH) {\
        CPrintMessage("", "1.0", level, (char*) __FUNCTION__, __LINE__, msg, ##__VA_ARGS__);\
    }   \
} while(0)

#define DBG_D(msg, ...)	DBG_MSG(MSG_DBG, msg, ##__VA_ARGS__);
#define DBG_I(msg, ...)	DBG_MSG(MSG_INF, msg, ##__VA_ARGS__);
#define DBG_T(msg, ...)	DBG_MSG(MSG_TRK, msg, ##__VA_ARGS__);
#define DBG_W(msg, ...)	DBG_MSG(MSG_WRN, msg, ##__VA_ARGS__);
#define DBG_E(msg, ...)	DBG_MSG(MSG_ERR, msg, ##__VA_ARGS__);

#define BEGIN_FUNC()  \
do {\
    DBG_D("+Enter function: %s", __FUNCTION__);\
} while (0)

#define EXIT()  \
do {\
    goto Exit;\
} while (0)

#define END_FUNC()  \
Exit:\
do {\
    DBG_D("+Leave function: %s", __FUNCTION__); \
} while (0)

#ifndef MSG_DBG
enum {
    MSG_DBG,
    MSG_INF,
    MSG_TRK,
    MSG_WRN,
    MSG_ERR,
    MSG_MAX
};
#endif

static char *levString(int lev)
{
    static char *msgLevTable[MSG_MAX] = {"DBG", "INF", "TRK", "WRN", "ERR"};
    return msgLevTable[lev];
}

static inline void CPrintMessage(char *sModuleName, char *sModuleVersion, int nMesLevel, char *sFuncName, unsigned int uiLine, char *format, ...)
{
	char tagMsg[4096];
	char userInputMsg[4096];
	char *outputMsg = NULL;

	// Declare variable argument list
    va_list		pArgList;
    
	// Initialize a variable argument list
    va_start(pArgList, format);
    
    do
    {
        sprintf(tagMsg, "{ %s(%s) %s %s, %d }: ", sModuleName, sModuleVersion, levString(nMesLevel), /*@""*/sFuncName, uiLine);
        vsprintf(userInputMsg, format, pArgList);
        outputMsg = strcat(tagMsg, userInputMsg);
        
    } while(0);
    
    // free a variable argument list
    va_end(pArgList);
    
    //RETAILMSG(TRUE, (pszMessage));	// For wince.
    puts(outputMsg);
    
} // end of 'inline void PrintMessage(int iLevel, WCHAR *szFormat, ...)'

#endif
