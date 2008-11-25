@echo off
if "%1" == "" (echo Usage: %0 ALGORITHM_NAME
goto :eof)
cd ..\debug
cd ..\instances
(for %%f in (*.tim) do ..\debug\timetabling.exe %1 %%f ) 1> ..\reports\%1.txt 2> ..\reports\%1.log
cd ..\reports