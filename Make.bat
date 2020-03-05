@call cmake_gcc.bat

:: install the library
@libmerge %PROGRAMS%\local\lib32\libexshit.a  bin\libres.a
@copy /Y *.h %PROGRAMS%\local\include
