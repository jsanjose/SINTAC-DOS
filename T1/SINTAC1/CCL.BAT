@echo off
echo ... Listador ...
call c:\qc\bin\new-vars
qcl /AS /W2 /Zr %1 ls.c
call c:\qc\bin\old-vars
