#include "stdshit.h"
#include "common.h"
const char progName[] = "iconShrink";

int main(int argc, char* argv[])
{
	// print usage
	if(argc < 2) {
		printf("icon-shrink: [input] <output> [-<max size>]\n");
		return 1;
	}
	
	GET_NAME_ARGS
	ARGV_GET_INT(maxSize, 256)
	LOAD_FILE
	resDir.shrinkIcon(maxSize);
	SAVE_FILE
};
