@echo off
set main_script=%1
if "%main_script%" EQU "" echo No main script! & goto :eof
set "main_script=%main_script:.c=%"
echo ==================================================================
echo Compiling csvparser.c
gcc -c ../release/csvparser.c
echo ==================================================================
echo Compiling %main_script%.c
gcc -c %main_script%.c
echo ==================================================================
echo Creating %main_script%.out
gcc csvparser.o %main_script%.o -o %main_script%.out