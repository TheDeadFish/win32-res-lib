#include <stdshit.h>
#include "win-dlg.h"

WinDlg::~WinDlg() {}

byte* WinDlg_t1::DlgName::get2(byte* data, byte* end)
{
	#define _DLGNMGET if(curPos > end) return 0; lodsw(curPos, ch);
	byte* curPos = data; 
	end-=2; WCHAR ch; _DLGNMGET switch(ch) { case 0xFFFF: _DLGNMGET
	case 0: RT(&name) = ch; return curPos; } do { _DLGNMGET } while(ch);
	name = utf816_dup((WCHAR*)data); return curPos;
}

int WinDlg_t1::DlgName::build_size()
{
	if(IS_PTR(name)) return utf816_size(name);
	return name ? 4 : 2;
}

byte* WinDlg_t1::DlgName::build(byte* data)
{
	if(IS_PTR(name)) return (byte*)(
		utf816_cpy((WCHAR*)data, name)+1);
	if(name) { WRI(PW(data), 0xFFFF); }
	WRI(PW(data), WORD(size_t(name))); return data;
}

void WinDlg::clear(void) 
{
	this->~WinDlg(); ZINIT; 
}

bool WinDlg::load(byte* base, int size)
{
	this->clear(); byte* data = base;
	if(size < 18) return false;
	byte* end = data+size;
	dlgEx = RW(data+2) == 0xFFFF;
	if(dlgEx) { if(size < 26) return false;
		data = memcpyY(&helpID, data+4, 3);
	} else {lodsl(data, style);
		lodsl(data, exStyle); }
		
	WORD cDlgItems; lodsw(data, cDlgItems);
	data = memcpyY(&x, data, 4);
	if(!menu.get(data, end)) return false;
	if(!clss.get(data, end)) return false;
	if(!title.get(data, end)) return false;
	
	// get font information
	if(style & (DS_SETFONT | DS_SHELLFONT)) { 
		int rem = end-data; if(rem < 2) return false;
		WORD* tmp = &pointsize; memcpy_ref(tmp, data, 1);
		if(dlgEx) { if(rem < 6) return false;
			memcpy_ref(tmp, data, 2); }
		if(!font.get(data, end)) return false; 
	}
	
	// parse controls
	items.xcalloc(cDlgItems);
	for(auto& item : items) {
		while((data-base) & 3) data++;
		if((end-data) < 24) return false;
		if(dlgEx) {data = memcpyY(&item.helpID, data, 6);
		} else { lodsl(data, RI(&item.style));
		lodsl(data, RI(&item.exStyle));
		data = memcpyY(&item.x, data, 5); }
		if(!item.clss.get(data, end)) return false;
		if(!item.title.get(data, end)) return false;
		WORD length; lodsw(data, length); 
		if((end-data) < length) return false;
		item.createData.xcopy(data, length);
		data += length;
	}
	
	return true;
}

int WinDlg::build_size(void)
{
	int totalSize = 0;
	totalSize += dlgEx ? 26 : 18;
	totalSize += menu.build_size();
	totalSize += clss.build_size();
	totalSize += title.build_size();
	if(style & (DS_SETFONT | DS_SHELLFONT)) {
		totalSize += dlgEx ? 6 : 2;
		totalSize += font.build_size(); }
	
	for(auto& item : items) { 
		totalSize = ALIGN4(totalSize);
		totalSize += dlgEx ? 24 : 18;
		totalSize += item.clss.build_size();
		totalSize += item.title.build_size();
		totalSize += item.createData.size+2;
	}
	
	return totalSize;
}

byte* WinDlg::build(byte* base)
{
	byte* data = base;
	if(dlgEx) { stosl(data, 0xFFFF0001);
		data = memcpyX(PI(data), &helpID, 3);
	} else { stosl(data, style);
		stosl(data, exStyle); }
		
	stosw(data, items.len);
	data = memcpyX(PW(data), &x, 4);
	data = menu.build(data);
	data = clss.build(data);
	data = title.build(data);
	
	if(style & (DS_SETFONT | DS_SHELLFONT)) {
		WORD* tmp = &pointsize;	memcpy_ref(PW(data), tmp, 1);
		if(dlgEx) memcpy_ref(PW(data), tmp, 2);
		data = font.build(data); }
	
	for(auto& item : items) { 
		while((data-base) & 3) stosb(data, 0);
		if(dlgEx) {data = memcpyX(PI(data), &item.helpID, 6);
		} else { stosl(data, item.style); stosl(data, item.
			exStyle); data = memcpyX(PW(data), &item.x, 5); }
		data = item.clss.build(data);
		data = item.title.build(data);
		stosw(data, item.createData.size);
		data = memcpyX(data, item.createData
			.data, item.createData.size);
	}
	
	return data;
}

xarray<byte> WinDlg::build(void)
{
	int totalSize = build_size();
	byte* buff = xmalloc(totalSize);
	return {buff, build(buff)};
}
