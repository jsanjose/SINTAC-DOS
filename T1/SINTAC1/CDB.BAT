@echo off
echo ... Int‚rprete-debugger ...
call c:\qc\bin\new-vars
qcl /AS /W2 /Zr %1 /DDEBUGGER=ON /Feds.exe ds.c
call c:\qc\bin\old-vars
