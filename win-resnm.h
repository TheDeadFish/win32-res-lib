#ifndef _WINRES_NAME_
#define _WINRES_NAME_

struct winResName { union { size_t id; char* name = 0; };
	cch* getName(char* buff) const; void setName(cch* name); 
	int cmpName(cch* str) const;  void rsFree(); 
	bool isStr() { return IS_PTR(name); }
	SHITSTATIC cch* fixName(cch* str); };

struct WinResName : winResName {
	~WinResName() { rsFree(); } };

#endif
