#include <stdshit.h>
#include "win-res.h"

ALWAYS_INLINE bool ovfchk32(u32 a, u32 b, u32 c) { u32 tmp;
	if(__builtin_add_overflow(a, b, &tmp)) return true;
	nothing(); return (tmp > c); }

struct ImgResDir : IMAGE_RESOURCE_DIRECTORY {
	IMAGE_RESOURCE_DIRECTORY_ENTRY ent[1]; 
	int size(int i) { return offsetof(
		ImgResDir, ent[i]); } };
	
Win32ResDir::ResData::~ResData()
{
	if(type<0) { ico().~IconFile(); }
	ei(type==0) { this->free(); }
}

void Win32ResDir::ResData::update(xarray<byte> data)
{
	if(isIco()) { ico().load(data.data, data.len); }
	else { free(); xcopy(data.data, data.len); }
}

// resource finding - core
Win32ResDir::TypeDir* Win32ResDir::findType(cch* str) {
	str = winResName::fixName(str); for(auto& type : types) 
	if(!type.cmpName(str)) return &type; return NULL; }
Win32ResDir::NameDir* Win32ResDir::TypeDir::findName(cch* str) {
	str = winResName::fixName(str); for(auto& name : *this)
	if(!name.cmpName(str)) return &name; return NULL; }
Win32ResDir::ResData* Win32ResDir::NameDir::findData(int langId){
	for(auto& data : *this) { if(data.langId
		== langId) return &data; } return NULL; }
		
// resource finding - helpers
Win32ResDir::ResData* Win32ResDir::TypeDir::findData(cch* ns, int langId) {
	NameDir* name = findName(ns); return name ? name->findData(langId) : 0; }
Win32ResDir::ResData* Win32ResDir::findData(cch* ts, cch* ns, int langId) {
	TypeDir* type = findType(ts); return type ? type->findData(ns, langId) : 0; }
Win32ResDir::NameDir* Win32ResDir::findName(cch* ts, cch* ns) {
	TypeDir* type = findType(ts); return type ? type->findName(ns) : 0; }
xarray<byte> Win32ResDir::NameDir::getData(int langId){ ResData* data = 
	findData(langId); return data ? *data : xarray<byte>{0,0}; }
xarray<byte> Win32ResDir::TypeDir::getData(cch* ns, int langId) { ResData* 
	data = findData(ns, langId); return data ? *data : xarray<byte>{0,0}; }
xarray<byte> Win32ResDir::getData(cch* ts, cch* ns, int langId) { ResData* 
	data = findData(ts, ns, langId); return data ? *data : xarray<byte>{0,0}; }

void initBmpHeader(BITMAPFILEHEADER*
	bmHead, void* data, DWORD size)
{
	*bmHead = { 0x4D42, size + sizeof(*bmHead),
		0, 0, sizeof(*bmHead) };
	BITMAPINFOHEADER* bi = Void(data);
	int palSize = bi->biClrUsed;
	if((!palSize)&&(bi->biBitCount < 16))
		palSize = 1 << bi->biBitCount;
	bmHead->bfOffBits += bi->biSize + palSize*4;
}

xarray<byte> res_loadBmp(void* data, int size)
{
	BITMAPFILEHEADER bh;
	initBmpHeader(&bh, data, size);
	byte* out = xmalloc(bh.bfSize);
	byte* dstPos = memcpy8(out, &bh, sizeof(bh));
	dstPos = memcpy8(dstPos, data, size);
	return {out, bh.bfSize};
}

struct peRes_peInfo 
{
	IMAGE_DATA_DIRECTORY*  dataDir;
	IMAGE_SECTION_HEADER* rsrcSection;
	IMAGE_SECTION_HEADER* lastSection;
	DWORD* SizeOfImage; DWORD* CheckSum;
	DWORD FileAlignment, SectAlignment;
	bool init(byte* data);
	DWORD alignSect(DWORD val) { return
		ALIGN(val, SectAlignment-1); }
};

bool peRes_peInfo::init(byte* data)
{
	IMAGE_DOS_HEADER* dosHeadr = Void(data);
	IMAGE_NT_HEADERS32* peHeadr = 
		Void(data) + dosHeadr->e_lfanew;
	IMAGE_FILE_HEADER* fileHeadr = &peHeadr->FileHeader;
	IMAGE_OPTIONAL_HEADER32* optHeadr = &peHeadr->OptionalHeader;
	IMAGE_SECTION_HEADER* peSects = Void(optHeadr)
		+ fileHeadr->SizeOfOptionalHeader;
	if(fileHeadr->Machine == IMAGE_FILE_MACHINE_AMD64) {
		IMAGE_OPTIONAL_HEADER64* oph64 = Void(optHeadr);
		dataDir = oph64->DataDirectory; SizeOfImage =
		&oph64->SizeOfImage; CheckSum = &oph64->CheckSum;
		FileAlignment = oph64->FileAlignment;
		SectAlignment = optHeadr->SectionAlignment;
	} else { dataDir = optHeadr->DataDirectory; 
		SizeOfImage = &optHeadr->SizeOfImage;
		CheckSum = &optHeadr->CheckSum; 
		FileAlignment = optHeadr->FileAlignment;
		SectAlignment = optHeadr->SectionAlignment;
		
		
		
	}

	// locate sections of interest
	lastSection = peSects+fileHeadr->NumberOfSections;
	rsrcSection = NULL; auto* curPos = peSects;
	for(; curPos < lastSection; curPos++)
		if(!strcmp((char*)curPos->Name, ".rsrc")) {
			rsrcSection = curPos; break; }
	
	return true;
}

int Win32ResDir::load(cch* fileName) 
{
	auto file = loadFile(fileName); 
	if(!file) return 3;
	SCOPE_EXIT(free(file.data));
	peRes_peInfo head; head.init(file.data);
	if(!head.rsrcSection) return 2;
	return load({head.rsrcSection->PointerToRawData
		+ file.data, head.rsrcSection->SizeOfRawData, 
		head.rsrcSection->VirtualAddress});
}

xRngPtr<IMAGE_RESOURCE_DIRECTORY_ENTRY> Win32ResDir::
	loadArg_t::rde_chk(PIMAGE_RESOURCE_DIRECTORY_ENTRY rde)
{
	u32 offset = rde ? rde->OffsetToDirectory : 0;
	if(chk(offset,sizeof(IMAGE_RESOURCE_DIRECTORY))) return {0,0};
	PIMAGE_RESOURCE_DIRECTORY ird = Void(data, offset); 
	u32 count = ird->NumberOfNamedEntries+ird->NumberOfIdEntries;
	if(chk(offset, count*sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY)))
		return {0, 0}; return {Void(ird+1), count};
}

Void  Win32ResDir::loadArg_t::rde_get(u32 offset, u32 len) {
	if(ovfchk32(offset, len, size)) return 0;
	return Void(data, offset); }
	
char* Win32ResDir::loadArg_t::rde_name(PIMAGE_RESOURCE_DIRECTORY_ENTRY rde)
{
	if(rde->NameIsString) { if(chk(rde->NameOffset+2)) return 0;
		PIMAGE_RESOURCE_DIR_STRING_U rds = Void(data, rde->NameOffset);
		if(chk(rde->NameOffset+2,rds->Length*2)) return 0;
		return utf816_dup(rds->NameString, rds->Length);
	} else { return (char*)rde->Name; }
}

bool Win32ResDir::save(xarray<byte>& file)
{
	peRes_peInfo head; head.init(file.data);
	if(!head.rsrcSection) return false;
	
	// calculate size delta
	int len; Win32ResDir::ResBuild rbd(this, len);
	int rsrcLen = ALIGN(len, head.FileAlignment-1);
	int delta = rsrcLen-head.rsrcSection->SizeOfRawData;
	if(delta > 0) { xrealloc(file.data, 
		file.len + delta); head.init(file.data); }

	// copy data
	int rsrcEnd = head.rsrcSection->PointerToRawData 
		+ head.rsrcSection->SizeOfRawData;
	memmove(file.data + rsrcEnd + delta, file.data + rsrcEnd, 
		file.len - rsrcEnd); file.len += delta;
	rbd.data = file.data + head.rsrcSection->PointerToRawData;
	rbd.build(head.rsrcSection->VirtualAddress);
	
	// update resource section
	int imgDelta = head.alignSect(len)-head.alignSect(
		head.rsrcSection->Misc.VirtualSize);
	head.dataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size = len;
	head.rsrcSection->Misc.VirtualSize = len;
	head.rsrcSection->SizeOfRawData = rsrcLen;	
	
	// update extra sections
	*head.SizeOfImage += imgDelta;
	auto* sect = head.rsrcSection+1;
	for(; sect < head.lastSection; sect++) {
		sect->PointerToRawData += delta;
		sect->VirtualAddress += imgDelta;
		if(!strcmp((char*)sect->Name, ".reloc")) {
			head.dataDir[IMAGE_DIRECTORY_ENTRY_BASERELOC].
			VirtualAddress = sect->VirtualAddress; }
	}
	
	// correct checksum
	*head.CheckSum = 0; u16 checkSum = 0;
	nothing(); /* !! strict alias fix !! */
	for(int i = 0; i < file.len; i += 2) {
		u32 tmp = checkSum + RW(file.data+i);
		checkSum = tmp + (tmp>>16); }
	*head.CheckSum = checkSum + file.len;
	return true;
}

bool Win32ResDir::save(cch* src, cch* dst)
{
	if(src == NULL) src = dst;
	xArray<byte> file = loadFile(src);
	if(!file) return false;
	if(!save(file)) return false;
	return saveFile(dst, file.data, file.len);
}



	
int Win32ResDir::load(loadArg_t args)
{
	TypeDir curIco[2]; TypeDir* pCurIco;
	types.Clear();

	// type loop
	auto rde = args.rde_chk(0);
	if(!rde) return 1; for(; rde.chk(); rde.fi()) { 
	pCurIco = (rde->Name == 1) ? curIco 
		: (rde->Name == 3) ? curIco+1 : NULL;
	auto& type = pCurIco ? *pCurIco : types.push_back();
	type.name = args.rde_name(rde);
	
	// name loop
	auto rde2 = args.rde_chk(rde); 
	if(!rde2) return 1; for(; rde2.chk(); rde2.fi()) {
	auto& name = type.push_back(); name.name = args.rde_name(rde2);
	
	// data loop
	auto rde3 = args.rde_chk(rde2);
	if(!rde3) return 1; for(; rde3.chk(); rde3.fi()) {
	auto& data = name.push_back(); data.langId = rde3->Name;
	
	// get resource data
	if(args.chk(rde3->OffsetToData, sizeof(IMAGE_RESOURCE_DATA_ENTRY))) return 1;
	PIMAGE_RESOURCE_DATA_ENTRY rdat = Void(args.data, rde3->OffsetToData);
	if(ovfchk32(rdat->OffsetToData-args.rva, rdat->Size, args.size)) return 1;
	
	
	// read resource data
	byte* pDat = Void(args.data + rdat->OffsetToData-args.rva);
	if(pCurIco || is_one_of(type.id, 12, 14)) {
		data.init(pDat, rdat->Size); data.type = 1; } 
	ei(type.id == 2) { data.init(res_loadBmp(pDat, rdat->Size));
	} else { data.xcopy(pDat, rdat->Size); }
	}}}
	
	// combine icons/cursors
	for(size_t i = 0; i < 2; i++) {
	auto* type = findType((cch*)(i ? 14 : 12));
	if(type == NULL) continue; 
	for(auto& name : *type) for(auto& ent : name) {
		ent.type = -1; if(!ent.ico().load(
		release(ent.data), release(ent.len),
		MakeDlgtLmbd([&](u32 nId) { return curIco[i]
		.getData((cch*)nId, ent.langId); }))) return 1;
	}}
	
	return 0;
}

int Win32ResDir::ResBuild::find_name(char* name)
{
	for(auto& val : names) if(val.name == name)
		return val.offset; return -1;
}

void Win32ResDir::ResBuild::add_name(char* name)
{
	dirSize += sizeof(ImgResDir);
	if(!IS_PTR(name) || (find_name(name) >= 0)) return;
	names.push_back(name, nameSize);
	nameSize += utf816_size(name);
}

int Win32ResDir::ResBuild
	::buildSize(Win32ResDir& resDir)
{
	// build icons/cursors
	memset(this, 0, sizeof(*this));
	for(auto& type : resDir.types) { 
		rNew(types.xnxalloc(), type.id, type.get());
	if(is_one_of(type.id, 12, 14)) { auto& icoType = rNew(
		types.xnxalloc(), type.id-11,xarray<NameDir>());
	u32 icoId = 1; 		
	for(auto& name : type) for(auto& dent : name) {
	for(auto& ient : dent.ico()) { ient.nId = icoId;
		icoType.push_back(icoId++).push_back(
		ient.get(), 0, 1, dent.langId); }
	}}}
	qsort(types, [](const ResBuild::TypeDir2& a, const 
		ResBuild::TypeDir2& b) { return b.cmpName(a.name); });

	// calculate sizes
	dirSize += sizeof(IMAGE_RESOURCE_DIRECTORY);
	for(auto& type : types) { add_name(type.name);
	for(auto& name : type) { add_name(name.name);
	for(auto& data : name) { 
	dirSize += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);
	dirSize += sizeof(IMAGE_RESOURCE_DATA_ENTRY); 
		
		
	u32 resLen;
	if(data.isIco()) { resLen = data.ico().resSize(); }
	else { resLen = data.len; if(type.id == 2) {
		resLen -= sizeof(BITMAPFILEHEADER); } }
	dataSize += ALIGN4(resLen);
	}}}
	
	// combine sizes
	nameSize = ALIGN4(nameSize);
	int totalSize = dirSize + 
		nameSize + dataSize;
	dataSize = nameSize+dirSize;
	nameSize = release(dirSize);			
	return totalSize;
}

void Win32ResDir::ResBuild::build(int rva)
{
	struct IRD { 
		ImgResDir* ird; int i; auto& e() { return ird->ent[i]; }
	IRD(ResBuild* rbd, int len) : i(0) { ird = rbd->ptr(rbd->dirSize);
		memset(ird, 0, ird->size(len)); rbd->dirSize += ird->size(len); }
	void initEnt(ResBuild* rbd, char* name) { e().OffsetToData 
		= rbd->dirSize | 0x80000000; if(IS_PTR(name)) { int no = 
		rbd->find_name(name) + rbd->nameSize; e().Name = no|INT_MIN;
		PIMAGE_RESOURCE_DIR_STRING_U rds = rbd->ptr(no);
		WCHAR* end = utf816_cpy_(rds->NameString, name);
		rds->Length = end-rds->NameString; ird->NumberOfNamedEntries++;
	} else { e().Name = (DWORD)name; ird->NumberOfIdEntries++; 
	}}};
	
	// type loop
	for(IRD ird1(this, types.len); ird1.i < types.len;
	ird1.i++) { auto& type = types[ird1.i];
		ird1.initEnt(this, type.name);
		
	// name loop
	for(IRD ird2(this, type.len); ird2.i < type.len;
	ird2.i++) { auto& name = type[ird2.i];
		ird2.initEnt(this, name.name);
		
	// data loop
	for(IRD ird3(this, name.len); ird3.i < name.len;
	ird3.i++) { auto& ent = name[ird3.i];
		ird3.initEnt(this, (char*)(size_t)(ent.langId));
		ird3.e().OffsetToData = dirSize;

		// write the data
		u32 resLen; if(ent.isIco()) { 
			ent.ico().resBuild(ptr(dataSize));
			resLen = ent.ico().resSize(); 
		} else { void* iDat = ent.data; 
			resLen = ent.len; if(type.id == 2) {
				iDat += sizeof(BITMAPFILEHEADER);
				resLen -= sizeof(BITMAPFILEHEADER); }
			memcpy(ptr(dataSize), iDat, resLen); 
		}
		
		// write data entry
		*(PIMAGE_RESOURCE_DATA_ENTRY)(ptr(dirSize)) 
			= {dataSize + rva, resLen, 0, 0};
		dirSize += sizeof(IMAGE_RESOURCE_DATA_ENTRY);	
		dataSize += ALIGN4(resLen);
	}}}
}

void Win32ResDir::shrinkIcon(int maxSize)
{
	for(auto& type : types) for(auto& name : type)
	for(auto& data : name) if(data.isIco()) {
		data.ico().shrink(maxSize); }
}
