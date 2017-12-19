#include <stdshit.h>
#include "win-resnm.h"

cch* winResName::fixName(cch* str) {
	if((IS_PTR(str))&&(str[0] == '#'))
		return (cch*)atoi(str+1); return str; }

void winResName::rsFree(void)
{
	if(IS_PTR(name)) free(name);
	name = 0;
}

cch* winResName::getName(char* buff) const
{
	if(IS_PTR(name)) return name;
	sprintf(buff, "#%d", id);
	return buff;
}

void winResName::setName(cch* str)
{
	str = fixName(str); name = IS_PTR
	(str) ? xstrdup(str).data : (char*)str;
}

int winResName::cmpName(cch* str) const
{
	int isPtr = IS_PTR(str);
	int diff = IS_PTR(name)-isPtr;
	if(diff) return diff;
	if(isPtr) { return strcmp(str, name); }
	return str - name; 
}
