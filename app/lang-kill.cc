#include "stdshit.h"
#include "common.h"
const char progName[] = "langKill";

int main(int argc, char* argv[])
{
	// print usage
	if(argc < 2) {
		printf("lang-kill: [input] <output> [-<keep lang: def us english>]\n");
		return 1;
	}
	
	GET_NAME_ARGS
	ARGV_GET_INT(maxSize, 1033)
	LOAD_FILE
	resDir.langKill(maxSize);
	SAVE_FILE
};

