cd ..\_output
del midgard.exe
cl -o midgard.exe /D _MIDGARD_MM_TEST=1 ..\midgard\midgard.c
midgard
pause
