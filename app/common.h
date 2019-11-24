#pragma once
#include "win-res.h"

Win32ResDir resDir;
cch* in_file_name;
cch* out_file_name;

#define ERR_EXIT(...) ({ \
	fprintf(stderr, __VA_ARGS__); \
	exit(1); })
	
#define GET_LOAD_NAME \
	in_file_name = argv[1]; \
	argv += 2; argc -= 2;
	
#define GET_SAVE_NAME(def) \
	out_file_name = argv[0]; \
	if(!out_file_name ||(*out_file_name == '-')) { \
		out_file_name = def; \
	} else { argv += 1; argc -= 1; }

#define GET_NAME_ARGS \
	in_file_name = argv[1]; out_file_name = argv[2]; \
	if(!out_file_name ||(*out_file_name == '-')) { \
		out_file_name = in_file_name; argv += 2; argc -= 2; \
	} else { argv += 3; argc -= 3; }
	
int parse_int(cch* str) {
	char* end; int ret = strtol(str, &end, 10);
	if(str == end) ERR_EXIT("bad integer arg: %s", str);
	return ret; }
	
#define ARGV_GET_INT(name, def) \
	int name = def; \
	if(argv[0] && (argv[0][0] == '-')) { \
		name = parse_int(argv[0]+1); }
		
#define ARGV_GET_STR(name, def) \
	cch* name = def; \
	if(argv[0] && (argv[0][0] == '-')) { \
		name = argv[0]+1; }

#define LOAD_FILE \
	if(resDir.load(in_file_name)) { \
		ERR_EXIT("failed to load file: %s\n", out_file_name); }

#define SAVE_FILE \
	if(resDir.save(in_file_name, out_file_name)) { \
		ERR_EXIT("failed to save file: %s\n", out_file_name);} \
		return 0;
