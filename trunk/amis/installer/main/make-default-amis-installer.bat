ECHO OFF

REM make-amis-installer.bat creates a default (English) installer

call make-amis-installer.bat "eng-US" "U.S. English" ""
SET OUTFILE=Setup-amis30-rc2-U.S.English.exe

move "Setup-amis30-rc2-U.S. English.exe" "%OUTFILE%"
