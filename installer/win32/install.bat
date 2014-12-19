@echo off

if defined ProgramFiles(x86) (
  @echo setting up 64 Bit registry settings
  regedit /S "%cd%\OpenSC_Reg64.reg"
) else (
  @echo setting up 32 Bit registry settings
  regedit /S "%cd%\OpenSC_Reg32.reg"
)

rem  set /p DUMMY=Hit ENTER to continue...
