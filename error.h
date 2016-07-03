#ifndef _ERROR_H_
#define _ERROR_H_

#define WARNING(msg);                        printf("WARNING :\n\t%s\n", msg);
#define WARNING_FUNC(function, msg);         printf("WARNING : function '%s' (line %d)\n\t%s\n", function, __LINE__, msg);
#define ERROR(function);                     printf("ERROR : function '%s' (line %d)\n", function, __LINE__);
#define ERROR_MSG(function, errMsg);         printf("ERROR : function '%s' (line %d) :\n\t%s\n", function, __LINE__, errMsg);
#define ERROR_FILE_OPENING(function, file);  char msg[256]; \
                                             sprintf(msg, "Couldn't open or create file %s", file); \
                                             ERROR_MSG(function, msg);

#define STATUS          char
#define RETURN_SUCCESS  0
#define RETURN_FAILURE  -1

#endif //_ERROR_H_
