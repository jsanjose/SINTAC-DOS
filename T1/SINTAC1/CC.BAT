@echo off
echo ... Compilador ...
call c:\qc\bin\new-vars
qcl /AS /W2 /Zr %1 /Fecs.exe cs.c
call c:\qc\bin\old-vars
