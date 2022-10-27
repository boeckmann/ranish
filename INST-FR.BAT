@echo off

echo.
echo Partition Manager Install
echo.
echo Ce script va effectuer les ‚tapes suivantes:
echo.
echo.    1. cr‚er une disquette systŠme bootable.
echo.    2. Copier Partition Manager sur la disquette.
echo.    3. Sauvegarder votre MBR dans un fichier sur la disquette.
echo.    4. Sauvegarder des informations sur vos partitions.
echo.
echo Vous pouvez interrompre … tout moment le script en pressant Ctrl-C
echo.

if not exist part-fr.exe goto error_missing_files
if not exist part-fr.htm goto error_missing_files

if "%1" == "/nf" goto step2

echo Etape 1: Formatage de la disquette
format a: /u /s /V:PART_MANAGER
if errorlevel == 1 goto error_formatting_floppy

:step2

echo Etape 2: Copie de Partition Manager sur la disquette...

copy part-fr.exe a:\  > nul
copy part-fr.htm a:\  > nul

echo @echo off > a:\autoexec.bat
echo echo Taper PART pour lancer Partition Manager >> a:\autoexec.bat

echo Etape 3: Sauvegarde du MBR actuel...

part-fr -q -d 1 -s a:\orig_mbr.hd1 > nul
part-fr -q -d 2 -s a:\orig_mbr.hd2 > nul
part-fr -q -d 3 -s a:\orig_mbr.hd3 > nul
part-fr -q -d 4 -s a:\orig_mbr.hd4 > nul

echo Step 4: Sauvegarde des information sur vos partitions...

part-fr -i     > a:\part_ide.

if exist a:\orig_mbr.hd1  part -d 1 -p     > a:\part_p.hd1
if exist a:\orig_mbr.hd1  part -d 1 -p -r  > a:\part_p_r.hd1

if exist a:\orig_mbr.hd2  part -d 2 -p     > a:\part_p.hd2
if exist a:\orig_mbr.hd2  part -d 2 -p -r  > a:\part_p_r.hd2

if exist a:\orig_mbr.hd3  part -d 3 -p     > a:\part_p.hd3
if exist a:\orig_mbr.hd3  part -d 3 -p -r  > a:\part_p_r.hd3

if exist a:\orig_mbr.hd4  part -d 4 -p     > a:\part_p.hd4
if exist a:\orig_mbr.hd4  part -d 4 -p -r  > a:\part_p_r.hd4

echo Installation termin‚e.
echo.
echo Veuillez SVP lire l'aide et le fichier FAQ de Partition Manager
echo avant d'effectuer des changements sur vos disques durs.
echo Il est vivement conseill‚ de sauvegarder d'abord vos donn‚es importantes!
goto end

:error_missing_files
echo Erreur: Il manque un ou plusieurs fichiers!
pause
goto end

:error_formatting_floppy
echo Erreur: Une erreur est survenue durant le formatage de la disquette!
pause
goto end

:end
