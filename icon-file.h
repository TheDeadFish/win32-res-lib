#ifndef _ICON_FILE_H_
#define _ICON_FILE_H_

// icon resource structures
PACK1(
struct GRPICONDIRENTRY {
	BYTE bWidth; BYTE bHeight;
	BYTE bColorCount; BYTE bReserved;
	WORD wPlanes; WORD wBitCount;
	DWORD dwBytesInRes; WORD nId; };
struct GRPICONDIR { WORD idReserved;
    WORD idType; WORD idCount;
    GRPICONDIRENTRY idEntries[]; };
struct ICONDIRENTRY {
	BYTE bWidth; BYTE bHeight;
	BYTE bColorCount; BYTE bReserved;
	WORD wPlanes; WORD wBitCount;
	DWORD dwBytesInRes; DWORD dwOfs; };
struct ICONDIR { WORD idReserved;
    WORD idType; WORD idCount;
    ICONDIRENTRY idEntries[]; };
);

struct IconFileDir : xArray<byte> {
	byte isPng;	BYTE bpp;
	WORD nId, w, h;
	
	
	
	
	bool reInit(bool isCursor);
	bool optimize(void);
	
	// bitmap helpers
	BITMAPINFOHEADER* bih() { return Void(data); }
	int palSize(void) { return bih()->biClrUsed
		? bih()->biClrUsed : 1<<bih()->biBitCount; }
	u32* palPtr() { return Void(data, bih()->biSize); }
	byte* pixPtr() { return Void(palPtr()+palSize()); }
	int pitch() { return ALIGN4(bih()->biWidth); }
};

extern template void xarray<IconFileDir>::Free();


struct IconFile : xArray<IconFileDir>
{
	bool isCursor; u8 reserved[3];
	
	
	
	
	
	
	// 
	void shrink(int maxRes);
	void compact(void);

	// load save interface
	int load(cch* fileName);
	bool load(byte* data, uint size);
	xarray<byte> save(void);
	bool save(cch* fileName);
	

		
		
		
		
	// resource building
	int resSize() { return offsetof1(
		GRPICONDIR, idEntries[len]); };
	void resBuild(void* data) {
		build(Void(data), NULL); }
	
	bool load(byte* data, uint size, 
		Delegate<xarray<byte>,u32> cb);
	
	// helpers
	//void Free(); void Clear();
	ICONDIRENTRY* load(byte* data,
		uint size, uint dirSize);
	byte* build(ICONDIR* id, byte* dstPos);	
};

#endif