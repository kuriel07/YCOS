cd ..\_output
del des.exe
cl -o des.exe /D _DES_TEST=1 ..\auth\des.c ..\misc\mem.c
des
pause