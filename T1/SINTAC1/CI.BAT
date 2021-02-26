@echo off
echo ... Int‚rprete ...
call c:\qc\bin\new-vars
qcl /AS /W2 /Zr %1 /DDEBUGGER=OFF /Feis.exe ds.c
call c:\qc\bin\old-vars
