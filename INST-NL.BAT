@echo off

echo.
echo Installatie van de Partition Manager:
echo.
echo Dit batch-bestand zal de volgende dingen doen:
echo.
echo.    1. Een systeemdiskette maken.
echo.    2. Partition Manager naar de diskette kopieren.
echo.    3. De huidige MBR naar een bestand op de diskette opslaan.
echo.    4. Informatie over de partities opslaan.
echo.
echo U kunt de installatie onderbreken door Ctrl-C in te drukken.
echo.

if not exist part.exe goto error_missing_files
if not exist part.htm goto error_missing_files

if "%1" == "/nf" goto step2

echo Step 1: Bezig met het formatteren van de diskette...
format a: /u /s /V:PART_MANAGER
if errorlevel == 1 goto error_formatting_floppy

:step2

echo Step 2: Bezig Partition Manager naar de diskette te kopieren...

copy part.exe a:\  > nul
copy part.htm a:\  > nul

echo @echo off > a:\autoexec.bat
echo echo Type PART om Partition Manager op te starten >> a:\autoexec.bat

echo Step 3: Bezig met het opslaan van de huidige MBR...

part -q -d 1 -s a:\orig_mbr.hd1 > nul
part -q -d 2 -s a:\orig_mbr.hd2 > nul
part -q -d 3 -s a:\orig_mbr.hd3 > nul
part -q -d 4 -s a:\orig_mbr.hd4 > nul

echo Step 4: Bezig informatie over de partities op te slaan...

part -i     > a:\part_ide.

if exist a:\orig_mbr.hd1  part -d 1 -p     > a:\part_p.hd1
if exist a:\orig_mbr.hd1  part -d 1 -p -r  > a:\part_p_r.hd1

if exist a:\orig_mbr.hd2  part -d 2 -p     > a:\part_p.hd2
if exist a:\orig_mbr.hd2  part -d 2 -p -r  > a:\part_p_r.hd2

if exist a:\orig_mbr.hd3  part -d 3 -p     > a:\part_p.hd3
if exist a:\orig_mbr.hd3  part -d 3 -p -r  > a:\part_p_r.hd3

if exist a:\orig_mbr.hd4  part -d 4 -p     > a:\part_p.hd4
if exist a:\orig_mbr.hd4  part -d 4 -p -r  > a:\part_p_r.hd4

echo De installatie is voltooid.
echo.
echo Leest u aub. eerst de documentatie en de FAQ alvorens
echo veranderingen op uw vaste schijf aan te brengen. 
echo Het maken van reservekopieen van uw belangrijke bestanden kan ook geen kwaad!   
goto end

:error_missing_files
echo Error: 1 of meer bestanden ontbreken!
pause
goto end

:error_formatting_floppy
echo Error: Er is een fout opgetreden tijdens het formatteren van de diskette!
pause
goto end

:end
