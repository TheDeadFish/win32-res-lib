#include "stdshit.h"
#include "win-res.h"

const char progName[] = "WinResTest";

int main()
{
	Win32ResDir resDir;
	resDir.load("user32-org.dll");
	resDir.save("user32-org.dll", "user32.dll");
	//resDir.load("shell32-out.dll");
	//resDir.save("shell32-tmp.dll", "shell32-out2.dll");
};
