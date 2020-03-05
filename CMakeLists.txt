project(win32-res-lib)
cmake_minimum_required(VERSION 3.12)
set( EXECUTABLE_OUTPUT_PATH ../bin)
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../bin)

add_library( res STATIC
	icon-file.cc win-dlg.cc win-res.cc win-resnm.cc)

link_libraries(-lstdshit res)
add_executable(icon-shrink app/icon-shrink.cc)
add_executable(lang-kill app/lang-kill.cc)
add_executable(res-dump app/res-dump.cc)
