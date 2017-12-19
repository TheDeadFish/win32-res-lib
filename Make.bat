call egcc.bat
gcc %CCFLAGS2% win-res.cc icon-file.cc win-dlg.cc win-resnm.cc -c -g
gcc %CCFLAGS2% test.cc -lstdshit *.o -o test.exe
copy /Y *.h %PROGRAMS%\local\include
ar -rcs  %PROGRAMS%\local\lib32\libexshit.a *.o
