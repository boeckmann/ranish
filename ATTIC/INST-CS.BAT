@echo off

echo.
echo Instalace Partition Manageru
echo.
echo Tato d�vka provede n�sleduj�c� kroky:
echo.
echo.    1. Vytvo�� startovac� syst�movou disketu.
echo.    2. Nakop�ruje na ni Partition Manager.
echo.    3. Uschov� v�� MBR do souboru na disket�.
echo.    4. Uschov� informace o oblastech va�eho disku.
echo.
echo Tuto d�vku m��ete kdykoliv p�eru�it stisknut�m Ctrl-C
echo.

if not exist part.exe goto error_missing_files
if not exist part.htm goto error_missing_files

if "%1" == "/nf" goto step2

echo Krok 1: Form�tov�n� diskety...
format a: /u /s /V:PART_MANAGER
if errorlevel == 1 goto error_formatting_floppy

:step2

echo Krok 2: Kop�rov�n� Partition Manageru na disketu...

copy part-cs.exe a:\part.exe  > nul
copy part.htm a:\  > nul

echo @echo off > a:\autoexec.bat
echo echo Pro spu�t�n� Partition Manageru napi�te PART >> a:\autoexec.bat

echo Krok 3: �schova st�vaj�c�ho MBR...

part -q -d 1 -s a:\orig_mbr.hd1 > nul
part -q -d 2 -s a:\orig_mbr.hd2 > nul
part -q -d 3 -s a:\orig_mbr.hd3 > nul
part -q -d 4 -s a:\orig_mbr.hd4 > nul

echo Krok 4: �schova informac� o oblastech na disku...

part -i     > a:\part_ide.

if exist a:\orig_mbr.hd1  part -d 1 -p     > a:\part_p.hd1
if exist a:\orig_mbr.hd1  part -d 1 -p -r  > a:\part_p_r.hd1

if exist a:\orig_mbr.hd2  part -d 2 -p     > a:\part_p.hd2
if exist a:\orig_mbr.hd2  part -d 2 -p -r  > a:\part_p_r.hd2

if exist a:\orig_mbr.hd3  part -d 3 -p     > a:\part_p.hd3
if exist a:\orig_mbr.hd3  part -d 3 -p -r  > a:\part_p_r.hd3

if exist a:\orig_mbr.hd4  part -d 4 -p     > a:\part_p.hd4
if exist a:\orig_mbr.hd4  part -d 4 -p -r  > a:\part_p_r.hd4

echo Instalace dokon�ena.
echo.
echo P�e�t�te si pros�m Partition Manager Help a FAQ d��ve,
echo ne� za�nete prov�d�t zm�ny na va�em disku.
echo Nejl�pe ud�l�te, kdy� si v�dy zaz�lohujete d�le�it� data.
goto end

:error_missing_files
echo Chyba: N�kter� soubor nebyl nalezen!
pause
goto end

:error_formatting_floppy
echo Chyba: Nastala chyba p�i form�tov�n� diskety!
pause
goto end

:end
