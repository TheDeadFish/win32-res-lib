#include <stdshit.h>
#include "icon-file.h"

template void xarray<IconFileDir>::Free();

int IconFile::load(cch* fName)
{
	xArray<byte> file = loadFile(fName);
	if(!file) return 2;
	return !load(file.data, file.len);
}
	
bool IconFileDir::optimize()
{
	this->reInit(false);
	if(bpp != 8) return false;
	byte* src = pixPtr(); 
	byte* end = src + pitch()*h;
	
	// create unique palette
	u32 tmpPal[16] = {}; int nColors = 0;
	byte lookup[256]; u32* pal = palPtr();
	for(byte* s = src; s < end; s++) {
		int i = ::findi(tmpPal, nColors, pal[*s]);
		if(i < 0) { if(nColors >= 16) return false;
			i = nColors++; tmpPal[i] = pal[*s]; }
		lookup[*s] = i; }
	memcpy(pal, tmpPal, sizeof(tmpPal));
	
	// perform color reduction
	byte* dst = Void(pal+16); while(src < end) {
		WRI(dst, (lookup[*src++] << 4) | lookup[*src++]); }
	memcpy(dst, src, len - (src-data)); len -= src-dst; 
	bih()->biBitCount = 4; bih()->biClrUsed = 0; 
	bpp = 4; return true;
}

bool IconFileDir::reInit(bool isCursor) {
	if(len < sizeof(BITMAPINFOHEADER))
		return false;
	isPng = (RI(data) == 0x474E5089);
	if(isPng) {	bpp = 32; w = bswap32(RU
		(data+16)); h = bswap32(RU(data+20));
	} else { auto* bi = bih(); if(isCursor) { 
		PTRADD(bi, 4); } bpp = bi->biBitCount; 
		w = bi->biWidth; h = abs(bi->biHeight)>>1; }
	return true;
}


ICONDIRENTRY* IconFile::load(
	byte* data, uint size, uint dirSz)
{
	ICONDIR* id = Void(data);
	if(isNeg(size -= sizeof(ICONDIR))
	||isNeg(size-(id->idCount*dirSz))) 
	return 0; isCursor = id->idType == 2;
	Free(); xcalloc(id->idCount);
	return Void(id+1);
}

bool IconFile::load(byte* data, uint size)
{
	#define LOAD_CMN(type, body) \
	type* ide = Void(load(data, size, sizeof(type))); \
	if(ide == NULL) return false; for(auto& ent : *this) { \
	body; ide++; if(!ent.reInit(isCursor)) return false; } return true;
	
	LOAD_CMN(ICONDIRENTRY, byte* pbih = data+ide->dwOfs;
		if(!isCursor) ent.xcopy(pbih, ide->dwBytesInRes);
		else { ent.xalloc(ide->dwBytesInRes+4);
			memcpy(ent.data+4, pbih, ide->dwBytesInRes);
			RW(ent.data+0) = ide->wPlanes;
			RW(ent.data+2) = ide->wBitCount; } );
}

bool IconFile::load(byte* data, uint size, 
	Delegate<xarray<byte>,u32> cb)
{
	LOAD_CMN(GRPICONDIRENTRY, 
		ent.nId = ide->nId;	auto iDat = cb(ide->nId);
		if(!iDat) return false;	ent.xcopy(iDat););
}

bool IconFile::save(cch* fileName)
{
	xArray<byte> file = save();
	return saveFile(fileName,
		file.data, file.len);
}

byte* IconFile::build(ICONDIR* id, byte* dstPos)
{
	// write header
	id->idReserved = 0;
	id->idType = isCursor + 1;
	id->idCount = len;
	ICONDIRENTRY* ide = Void(id+1);
	
	// write image
	for(auto& ent : *this) { ent.reInit(isCursor);

		if(isCursor && !dstPos) { RW(&ide->bWidth) 
			= ent.w; RW(&ide->bColorCount) = ent.h*2;
		} else { ide->bWidth = ent.w; ide->bHeight =
			ent.h; RW(&ide->bColorCount) = (ent.bpp
				< 8) ? (1 << ent.bpp) : 0; }
			
		if(isCursor && dstPos) { ide->wPlanes = 
			RW(ent.data+0); ide->wBitCount = RW(ent.data+2);
		} else { ide->wPlanes = 1; ide->wBitCount = ent.bpp; }
		
		ide->dwBytesInRes = ent.len; 
		if(dstPos) { ide->dwOfs = PTRDIFF(dstPos, id); ide++;
			int d = isCursor ? 4 : 0; dstPos =
			memcpyX(dstPos, ent.data+d, ent.len-d);
		} else { RW(&ide->dwOfs) = ent.nId; 
			PTRADD(ide, sizeof(GRPICONDIRENTRY)); }
	}
	
	return dstPos;
}


xarray<byte> IconFile::save(void)
{
	// allocate save buffer
	int dataPos = sizeof(ICONDIR) + 
		len*sizeof(ICONDIRENTRY);
	int size = dataPos;
	for(auto& ent : *this) {
		size += ent.len; }
	ICONDIR* id = xmalloc(size);
	
	// build icon
	byte* dstPos = build(id, Void(id, dataPos));
	return {(byte*)id, PTRDIFF(dstPos, id)};
}

void IconFile::shrink(int maxRes)
{
	for(auto& ent : *this) 
	{ 
		// remove large icons
		int entRes = max(ent.w, ent.h);
		if(entRes > maxRes) for(auto& ent2 : *this) { 
			if(max(ent2.w, ent2.h) < entRes) {
				ent.Clear(); break; }
		}

		// remove high color icons
		if(ent.bpp == 8) continue;
		for(auto& ent2 : *this) if((ent2.w == ent.w)
		&&(ent2.h == ent.h)) if(ent.bpp < 8) {
		if(ent2.bpp >= 8) { ent.Clear(); break; }}
		ei( inRng1(ent2.bpp, 8, ent.bpp)) {
			ent.Clear(); break; }
	} this->compact();

	for(auto& ent : *this) ent.optimize();
}

void IconFile::compact(void)
{
	IconFileDir* writePos = data;
	for(auto& ent : *this) { if(ent.data) 
	writePos = memcpyX(writePos, &ent, 1); }
	setend(writePos);
}
