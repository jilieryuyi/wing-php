/**
 *@×Ö·û´®´¦Àí
 */
#include "wing_string.h"

void wing_trim(char *s)   
{  
    char *start;  
    char *end;  
    int len = strlen(s);  
      
    start = s;  
    end = s + len - 1;  
  
    while (1)   
    {     
        char c = *start;  
        if (!isspace(c))  
            break;  
  
        start++;  
        if (start > end)  
        {     
            s[0] = '\0';  
            return;  
        }     
    }     
  
  
    while (1)   
    {     
        char c = *end;  
        if (!isspace(c))  
            break;  
  
        end--;  
        if (start > end)  
        {     
            s[0] = '\0';  
            return;  
        }  
    }  
  
    memmove(s, start, end - start + 1);  
    s[end - start + 1] = '\0';  
}  