#ifndef _WINRES_H_
#define _WINRES_H_
#include "icon-file.h"
#include "win-resnm.h"

struct Win32ResDir
{
	struct ResData : xarray<byte> {
		s8 rsv, type = 0; WORD langId; ~ResData();
		bool isIco() { return type < 0; }
		IconFile& ico() { return *(IconFile*)this; }
		void update(xarray<byte> data); };
		
	struct NameDir : WinResName, xArray<ResData> { 
		ResData* findData(int langId); 
		xarray<byte> getData(int langId); };
		
	struct TypeDir : WinResName, xArray<NameDir> { 
		ResData* findData(cch* name, int langId);
		xarray<byte> getData(cch* name, int langId);
		NameDir* findName(cch* name); };
		
	xArray<TypeDir> types;
	
	
	// resource finding
	static cch* fixName(cch* name);
	TypeDir* findType(cch* type);
	NameDir* findName(cch* type, cch* name);
	ResData* findData(cch* type, cch* name, int langId);
	xarray<byte> getData(cch* type, cch* name, int langId);
	
	
	
	bool save(xarray<byte>& file);
	bool save(cch* dst, cch* src);
	int load(cch* fileName);
	void shrinkIcon(int maxSize);
	
// private
	struct loadArg_t { byte* data; u32 size, rva; 
		bool chk(u32 a, u32 b=0) { return (a+b) > size; }
		char* rde_name(PIMAGE_RESOURCE_DIRECTORY_ENTRY red);
		Void rde_get(u32 offset, u32 len); 
		xRngPtr<IMAGE_RESOURCE_DIRECTORY_ENTRY> 
			rde_chk(PIMAGE_RESOURCE_DIRECTORY_ENTRY rde);
	};
	
	int load(loadArg_t args);
	
	
	struct ResBuild 
	{ 
		// public interface
		ResBuild(Win32ResDir* resDir, int& len) {
			len = buildSize(*resDir); }
		~ResBuild() { names.Free(); types.Free(); }
		void* data; void build(int rva); 
		int buildSize(Win32ResDir& resDir);
		
		// unique name list
		struct NameInfo { char* name; int offset; };
		xarray<NameInfo> names;
		int find_name(char* name);
		void add_name(char* name);
		
		// type directory
		int dirSize; int nameSize; int dataSize;
		Void ptr(u32 o) {return Void(data, o); }
		struct TypeDir2 : winResName, xarray<NameDir> {
		~TypeDir2() { if(is_one_of(id, 1, 3)) Free(); }};
		xarray<TypeDir2> types;
	};
};

#endif
