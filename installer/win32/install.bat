@echo off

opensc-2014-11-07_12-53-10-win32.msi

if defined ProgramFiles(x86) (
  @echo setting up 64 Bit registry settings
  regedit "%cd%\OpenSC_Reg64.reg"
) else (
  @echo setting up 32 Bit registry settings
  regedit "%cd%\OpenSC_Reg32.reg"
)

set /p DUMMY=Hit ENTER to continue...
