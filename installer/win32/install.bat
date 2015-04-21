@echo off

regedit /S "%cd%\opensc64.reg"
regedit /S "%cd%\opensc32.reg"

\Windows\system32\reg import "%cd%\opensc64.reg"
\Windows\system32\reg import "%cd%\opensc32.reg"

