#ifndef _WINDLG_H_
#define _WINDLG_H_
#include "win-resnm.h"

struct WinDlg_t1
{
	struct DlgName : WinResName { 
	
		// internal interface
		int build_size(void); 
		byte* build(byte* data); 
		ALWAYS_INLINE bool get(byte*& data, byte* end) {
			return data = get2(data, end); }
		byte* get2(byte* data, byte* end);
	};

	DlgName clss, title;
	DWORD helpID, exStyle, style;
	short x, y, cx, cy;
};

struct WinDlg : WinDlg_t1
{
	DlgName menu; DlgName font;
	WORD pointsize;	WORD weight;
	BYTE italic; BYTE charset;	

	struct DlgItem : WinDlg_t1 { DWORD id;
		xArray<byte> createData; };
	xArray<DlgItem> items; bool dlgEx;
		
	~WinDlg(); void clear(void);
	bool load(byte* data, int size);
	int build_size(void);
	byte* build(byte* data);
	xarray<byte> build(void);
};

#endif
