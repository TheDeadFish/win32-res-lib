#include "stdshit.h"
#include "common.h"
const char progName[] = "resDump";

struct TypeInfo { cch* type; cch* ext; };

TypeInfo types[] = {
	{"#14", ".ico"}, {"#2", ".bmp"},
	{"#5", ".dlg"}, {"AVI", ".avi"}
};


TypeInfo findType(cch* type) {
	for(auto& t : types) {
		if(!strcmp(t.ext, type)
		||!strcmp(t.type, type))
			return t;
	} return {type, ".res"};
}


int main(int argc, char* argv[])
{
	// print usage
	if(argc < 2) {
		printf("res-dump: <input> [out path] [-type]\n");
		return 1;
	}
	
	GET_LOAD_NAME 
	GET_SAVE_NAME(".")
	ARGV_GET_STR(typeName, ".ico")
	LOAD_FILE
	auto ti = findType(typeName);
	auto* type = resDir.findType(ti.type);
	if(!type) ERR_EXIT("type not found\n");

	// dumping loop
	char buff[16];
	for(auto& name : *type) {
		if(name.len != 1) ERR_EXIT("multiple languages");
		xstr fileName = xstrfmt("%j%k%s", out_file_name,
			name.getName0(buff), ti.ext);
		if(!name[0].dump(fileName)) { ERR_EXIT(
			"failed to dump: %s", fileName.data); }
	}
};
