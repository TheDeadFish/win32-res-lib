#ifndef _WINRES_NAME_
#define _WINRES_NAME_

struct winResName { 
	union { size_t id; char* name = 0; };
	
	winResName() = default;
	winResName(cch* ri) : name((char*)ri) {}
	winResName(size_t ri) : id(ri) {}

	cch* getName0(char* buff) const; 
	cch* getName(char* buff) const; void setName(cch* name); 
	int cmpName(cch* str) const;  void rsFree(); 
	bool isStr() const { return IS_PTR(name); }
	cch* fixName() const { return fixName(name); } 
	SHITSTATIC cch* fixName(cch* str); 
};
	
typedef const winResName cWinResName;
	
	

struct WinResName : winResName {
	~WinResName() { rsFree(); } };

#endif
