:: build library
call egcc.bat
pushd bin\obj
gcc %CCFLAGS2% ..\..\*.cc -c
popd
ar -rcs  bin\libexshit.a bin\obj\*.o

:: build tools
gcc %CCFLAGS2% app\icon-shrink.cc -o bin\icon-shrink.exe bin\libexshit.a -lstdshit
gcc %CCFLAGS2% app\lang-kill.cc -o bin\lang-kill.exe bin\libexshit.a -lstdshit

:: install the library
copy /Y *.h %PROGRAMS%\local\include
copy /Y bin\*.a %PROGRAMS%\local\lib32
