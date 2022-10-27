@echo off

echo.
echo Instalaci¢n del Partition Manager
echo.
echo Este script va a llevar a cabo los siguientes pasos:
echo.
echo.    1. Hacer un disquette de arranque.
echo.    2. Copiar el Partition Manager al disquette.
echo.    3. Guardar tu MBR en el disquette.
echo.    4. Guardar la informaci¢n de tus particiones en el disquette.
echo.
echo Puedes detener la instalaci¢n en cualquier momento pulsando Ctrl-C.
echo.

if not exist part*.exe goto error_missing_files
if not exist part*.htm goto error_missing_files

if "%1" == "/nf" goto step2

echo Paso 1: Formateando el disquette
format a: /u /s /V:PART_MANAGER
if errorlevel == 1 goto error_formatting_floppy

:step2

echo Paso 2: Copiando el Partition Manager en el disquette...

copy part.exe a:\  > nul
copy part.htm a:\  > nul

echo @echo off > a:\autoexec.bat
echo echo Type PART to run Partition Manager >> a:\autoexec.bat

echo Paso 3: Guardando tu actual MBR...

part -q -d 1 -s a:\orig_mbr.hd1 > nul
part -q -d 2 -s a:\orig_mbr.hd2 > nul
part -q -d 3 -s a:\orig_mbr.hd3 > nul
part -q -d 4 -s a:\orig_mbr.hd4 > nul

echo Paso 4: Guardando informaci¢n de las particiones...

part -i     > a:\part_ide.

if exist a:\orig_mbr.hd1  part -d 1 -p     > a:\part_p.hd1
if exist a:\orig_mbr.hd1  part -d 1 -p -r  > a:\part_p_r.hd1

if exist a:\orig_mbr.hd2  part -d 2 -p     > a:\part_p.hd2
if exist a:\orig_mbr.hd2  part -d 2 -p -r  > a:\part_p_r.hd2

if exist a:\orig_mbr.hd3  part -d 3 -p     > a:\part_p.hd3
if exist a:\orig_mbr.hd3  part -d 3 -p -r  > a:\part_p_r.hd3

if exist a:\orig_mbr.hd4  part -d 4 -p     > a:\part_p.hd4
if exist a:\orig_mbr.hd4  part -d 4 -p -r  > a:\part_p_r.hd4

echo Installaci¢n conclu¡da.
echo.
echo Please, read Partition Manager Help and FAQ before
echo making any changes on your hard disk.
echo Backing up your critical files is also a good idea.   
goto end

:error_missing_files
echo Error: ­Faltan uno o m s ficheros!
pause
goto end

:error_formatting_floppy
echo Error: ­Hubo un error formateando el disquette!
pause
goto end

:end


