#include "part.h"

     /* VERSION_DATE "2.37.05            June 10, 1998" */
#define VERSION_DATE "2.90.00        built " __DATE__
#define VERSION      "2.90.00"


#ifdef LANG_de		/* German  */
  #define de(x)	x;
#else
  #define de(x)
#endif

#ifdef LANG_ru		/* Russian */
  #define ru(x)	x;
#else
  #define ru(x)
#endif

#ifdef LANG_se		/* Swedish */
  #define se(x)	x;
#else
  #define se(x)
#endif

#ifdef LANG_sp		/* Spanish */
  #define sp(x)	x;
#else
  #define sp(x)
#endif

#ifdef LANG_fr		/* French */
  #define fr(x)	x;
#else
  #define fr(x)
#endif

#ifdef LANG_it		/* Italian */
  #define it(x)	x;
#else
  #define it(x)
#endif

#ifdef LANG_nl          /* Dutch   */
  #define nl(x) x;
#else
  #define nl(x)
#endif

#ifdef LANG_cs          /* Czech   */
  #define cs(x) x;
#else
  #define cs(x)
#endif

#if !defined(LANG_de) && !defined(LANG_ru) && !defined(LANG_se) && \
    !defined(LANG_sp) && !defined(LANG_fr) && !defined(LANG_it) && \
    !defined(LANG_nl) && !defined(LANG_cs)
  #define en(x)	x;
#else
  #define en(x)
#endif

void set_messages(void)
{
 int i;

en( PROGRAM_TITLE = "Ranish Partition Manager          Version "VERSION_DATE)
se( PROGRAM_TITLE = "Ranish Partion program            Version "VERSION_DATE)
sp( PROGRAM_TITLE = "Ranish Director de Partici¢n      Versi¢n "VERSION_DATE)
de( PROGRAM_TITLE = "Ranish Partition Manager          Version "VERSION_DATE)
ru( PROGRAM_TITLE = "Ranish Partition Manager           Ç•‡·®Ô "VERSION_DATE)
fr( PROGRAM_TITLE = "Ranish Partition Manager          Version "VERSION_DATE)
it( PROGRAM_TITLE = "Ranish Partition Manager         Versione "VERSION_DATE)
nl( PROGRAM_TITLE = "Ranish Partition Manager           Versie "VERSION_DATE)
cs( PROGRAM_TITLE = "Ranish Partition Manager            Verze "VERSION_DATE)

en( MANAGER_TITLE = "Advanced Boot Manager v"VERSION"")
se( MANAGER_TITLE = "Advanced Boot Manager v"VERSION"")
sp( MANAGER_TITLE = "Avansado Boot Manager v"VERSION"")
de( MANAGER_TITLE = "Advanced Boot Manager v"VERSION"")
ru( MANAGER_TITLE = "Advanced Boot Manager v"VERSION"")
fr( MANAGER_TITLE = "Advanced Boot Manager v"VERSION"")
it( MANAGER_TITLE = "Boot Manager Avanzato v"VERSION"")
nl( MANAGER_TITLE = "Advanced Boot Manager v"VERSION"")
cs( MANAGER_TITLE = "Advanced Boot Manager v"VERSION"")

en( HEADER_GET  = "Hard Disk ..    Getting hard disk parameters ...")
se( HEADER_GET  = "HÜrd Disk ..    LÑser hÜrd disk parametrar ...")
sp( HEADER_GET  = "Disco Duro..    Adquiriedo Disco Duro par†metros ...")
de( HEADER_GET  = "Festplatte..    Ermittle Plattenparameter ...")
ru( HEADER_GET  = "Ç®≠Á•·‚•‡ ..    á†Ø‡†Ë®¢†Ó Ø†‡†¨•‚‡Î §®·™† ...")
fr( HEADER_GET  = "Disque Dur..    Recherche des paramätres ...")
it( HEADER_GET  = "Disco rig...    Acquisizione parametri disco ...")
nl( HEADER_GET  = "Harde sch...    Ophalen van vaste schijf parameters ...")
cs( HEADER_GET  = "PevnÏ disk..    Z°sk†v†n° parametrÖ disku ...")

en( HEADER_EMBR = "EMBR Level..  ..... Mbytes")
se( HEADER_EMBR = "EMBR Level..  ..... Mbytes")
sp( HEADER_EMBR = "EMBR Nivel..  ..... Mbytes")
de( HEADER_EMBR = "EMBR Stufe..  ..... Mbytes")
ru( HEADER_EMBR = "EMBR Level..  ..... å°†©‚")
fr( HEADER_EMBR = "EMBR Niv  ..  ..... Moctet")
it( HEADER_EMBR = "EMBR Level..  ..... Mbytes")
nl( HEADER_EMBR = "EMBR Level..  ..... Mbyte ")
cs( HEADER_EMBR = "ÈroveÂ EMBR.  ..... MBytÖ ")

en( HEADER_CHS  = "Hard Disk ..  ..... Mbytes  [ .... cylinders  x ... heads  x ... sectors ]")
se( HEADER_CHS  = "HÜrd Disk ..  ..... Mbytes  [ .... cylinders  x ... heads  x ... sectors ]")
sp( HEADER_CHS  = "Disco Duro..  ..... Mbytes  [ .... cilindros  x ... lados  x ... sectores]")
de( HEADER_CHS  = "Festplatte..  ..... Mbytes  [ .... Zylinder   x ... Kîpfe  x ... Sektoren]")
ru( HEADER_CHS  = "Ç®≠Á•·‚•‡ ..  ..... å°†©‚   [ ....  Ê®´®≠§‡†  x ... ·‚Æ‡Æ≠ x ... ·•™‚Æ‡† ]")
fr( HEADER_CHS  = "Disque Dur..  ..... Moctet  [ .... Cylindres  x ... tàtes  x ... secteurs]")
it( HEADER_CHS  = "Disco rig...  ..... Mbytes  [ ....  cilindri  x ...  test  x ... settori ]")
nl( HEADER_CHS  = "Vaste sch...  ..... Mbyte   [ .... cylinders  x ... koppen x ... sectoren]")
cs( HEADER_CHS  = "PevnÏ disk..  ..... MBytÖ   [ .... cylindrÖ   x ... hlav   x ... sektorÖ ]")

en( HEADER_CHS2 = "                  File               Starting        Ending      Partition")
se( HEADER_CHS2 = "                  Fil                Startar         Slutar      Partition")
sp( HEADER_CHS2 = "                 Archivo            Comenzando     Finalizando   Partici¢n")
de( HEADER_CHS2 = "                 Datei-              Start            Ende      Partitions")
ru( HEADER_CHS2 = "                  í®Ø                è•‡¢Î©         èÆ·´•§≠®©       é°ÍÒ¨ ")
fr( HEADER_CHS2 = "             Type de systäme          DÇbut           Fin       Taille[Ko]")
it( HEADER_CHS2 = "                  File               Inizio           Fine      Partizione")
nl( HEADER_CHS2 = "              Type bestands-          Begin           Eind        Partitie")
cs( HEADER_CHS2 = "           Typ souborovÇho          Poü†teün°       KoncovÏ      Velikost ")

en( HEADER_CHS3 = "#  Active      System Type        Cyl Side Sect   Cyl Side Sect  Size [KB]")
se( HEADER_CHS3 = "#  Aktiv       System Typ         Cyl Sida Sekt   Cyl Sida Sekt  Strlk [K]")
sp( HEADER_CHS3 = "#  Activo    Tipo de Sistema      Cil Lado Secc   Cil Lado Secc  tama§o[K]")
de( HEADER_CHS3 = "# Startbar     system Typ         Zyl Kopf Sekt   Zyl Kopf Sekt  Grî·e[KB]")
ru( HEADER_CHS3 = "# í•™„È®©   î†©´Æ¢Æ© ë®·‚•¨Î      ñ®´ ë‚Æ‡ ë•™‚   ñ®´ ë‚Æ‡ ë•™‚    ‡†ß§•´†")
fr( HEADER_CHS3 = "#  Active      de fichier         Cyl Face Sect   Cyl Face Sect  partition")
it( HEADER_CHS3 = "#  Attivo    Tipo di Sistema      Cil Lato Sett   Cil Lato Sett  Dim.  [K]")
nl( HEADER_CHS3 = "#  Actief       systeem           Cyl Kop  Sect   Cyl Kop  Sect  Omvang[K]")
cs( HEADER_CHS3 = "# Aktivn°       systÇmu          cyl hlava sekt  cyl hlava sekt  obl. [KB]")

              /*  "1   Yes  123456789_123456789_123 9999 9999 9999  9999 9999 9999 11,111,111"  */


en( HEADER_LBA  = "Hard Disk ..  ..... Mbytes  [ Base 11,111,111  Total 11,111,111  sectors ]")
se( HEADER_LBA  = "HÜrd Disk ..  ..... Mbytes  [ Base 11,111,111  Total 11,111,111  sectors ]")
sp( HEADER_LBA  = "Disco Duro..  ..... Mbytes  [ Base 11,111,111  Total 11,111,111 sectores ]")
de( HEADER_LBA  = "Festplatte..  ..... Mbytes  [Start 11,111,111  Total 11,111,111 Sektoren ]")
ru( HEADER_LBA  = "Ç®≠Á•·‚•‡ ..  ..... å°†©‚   [ Å†ß† 11,111,111  Ç·•£Æ 11,111,111 ·•™‚Æ‡Æ¢ ]")
fr( HEADER_LBA  = "Disque Dur..  ..... Moctet  [ Base 11,111,111  Total 11,111,111 secteurs ]")
it( HEADER_LBA  = "Disco rig...  ..... Mbytes  [ Base 11,111,111 Totale 11,111,111 settori  ]")
nl( HEADER_LBA  = "Vaste sch...  ..... Mbyte   [ Base 11,111,111 Totaal 11,111,111 sectoren ]")
cs( HEADER_LBA  = "PevnÏ disk..  ..... MBytÖ   [Z†kl. 11,111,111  Celk. 11,111,111 sektorÖ  ]")

en( HEADER_LBA2 = "                  File           Starting  Number of     Ending  Partition")
se( HEADER_LBA2 = "                  Fil             Startar      Antal     Slutar  Partition")
sp( HEADER_LBA2 = "                 Archivo       Comenzando  N£mero de Finalizando Partici¢n")
de( HEADER_LBA2 = "                 Datei-           Start    Anzahl der    Ende   Partitions")
ru( HEADER_LBA2 = "                  í®Ø              è•‡¢Î© äÆ´®Á•·‚¢Æ  èÆ·´•§≠®©     é°ÍÒ¨ ")
fr( HEADER_LBA2 = "             Type de systäme      DÇbut    Nombre de     Fin    Taille[Ko]")
it( HEADER_LBA2 = "                  File            Inizio   Numero di     Fine   Partizione")
nl( HEADER_LBA2 = "              Type bestands-       Begin    Aantal       Eind     Partitie")  
cs( HEADER_LBA2 = "            Typ souborovÇho     Poü†teün°      Poüet    KoncovÏ   Velikost")

en( HEADER_LBA3 = "#  Active      System Type         sector    sectors     sector  Size [KB]")
se( HEADER_LBA3 = "#  Aktiv       System Typ          sektor   sektorer     sektor  Strlk [K]")
sp( HEADER_LBA3 = "#  Activo    Tipo de Sistema       sector   sectores     sector  Tama§o[K]")
de( HEADER_LBA3 = "# Startbar     system Typ          Sektor   Sektoren     Sektor  Grî·e[KB]")
ru( HEADER_LBA3 = "# í•™„È®©   î†©´Æ¢Æ© ë®·‚•¨Î       ·•™‚Æ‡   ·•™‚Æ‡Æ¢     ·•™‚Æ‡    ‡†ß§•´†")
fr( HEADER_LBA3 = "#  Active      de fichier         secteur   secteurs    secteur  partition")
it( HEADER_LBA3 = "#  Attivo    Tipo di Sistema      settore   settori      settor  Dim.  [K]")
nl( HEADER_LBA3 = "#  Actief       systeem            sector   sectoren     sector  Omvang[K]")
cs( HEADER_LBA3 = "# Aktivn°        systÇmu           sektor    sektorÖ     sektor  obl. [KB]")

              /*  "1   Yes  123456789_123456789_123123456789 1234567890 1234567890 11,111,111"  */


                /*  Hard Disk 1  12345 MB  [ 1023 cyl x 255 heads x 63 sects = 11,111,111 sects ] */
en( HEADER_CMD = "  Hard Disk %d  %5lu MB  [ %4d cyl x %3d heads x %2d sects = %s sects ]\n\n")
se( HEADER_CMD = "HÜrd Disk %d  %5lu MB  [ %4d cyl x %3d huvuden x %2d sekts = %s sekts ]\n\n")
sp( HEADER_CMD = " Disco Duro %d  %5lu MB  [ %4d cil x %3d lados x %2d sects = %s sects ]\n\n")
de( HEADER_CMD = " Festplatte %d  %5lu MB  [ %4d Zyl x %3d Kîpfe x %2d Sekt. = %s Sekt. ]\n\n")
ru( HEADER_CMD = "  Ç®≠Á•·‚•‡ %d  %5lu å°  [ %4d ñ®´ x %3d ë‚Æ‡ x %2d ë•™‚ = %s ë•™‚ ]\n\n")
fr( HEADER_CMD = " Disque Dur %d  %5lu Mo  [ %4d cyl x %3d tàtes x %2d sect = %s sect ]\n\n")
it( HEADER_CMD = "Disco rigido %d  %5lu MB  [ %4d cil x %3d testine x %2d sett = %s sett ]\n\n")
nl( HEADER_CMD = "Vast schijf %d  %5lu MB  [ %4d cyl x %3d koppen x %2d sect = %s sect  ]\n\n")
cs( HEADER_CMD = " PevnÏ disk %d  %5lu MB  [ %4d cyl x %3d hlav  x %2d sekt. = %s sekt. ]\n\n")

en( HEADER_SYSTYPE = "  ID     File System Type          Setup  Format  PrintInfo")
se( HEADER_SYSTYPE = "  ID     Fil System Typ          UppsÑtt  Format  SkrivInfo")
sp( HEADER_SYSTYPE = "  ID   Archivo Sistema Tipo     Organizar Formatear ImprimirInf")
de( HEADER_SYSTYPE = "  ID     Dateisystem Typ           Setup  Format  ZeigeInfo")
ru( HEADER_SYSTYPE = "  ID   í®Ø ‰†©´Æ¢Æ© ·®·‚•¨Î  äÆ≠‰®£„‡†Ê®Ô îÆ‡¨†‚ à≠‰Æ‡¨†Ê®Ô")
fr( HEADER_SYSTYPE = "  ID   Type systäme fichier       Config  Format  Impr.Info")
it( HEADER_SYSTYPE = "  ID     File Sistema Tipo  Impostazioni Formattare StampaInfo")
nl( HEADER_SYSTYPE = "  ID   Bestandssysteem type  Configureren  Formatteren  Info")
cs( HEADER_SYSTYPE = "  ID     Typ datovÇho systÇmu      Nastav Form†t  Ukaß info")

en( HEADER_BOOT_MENU = "  # Dev Partitions PP  Name                       Keys")
se( HEADER_BOOT_MENU = "  # Dev Partitions PP  Namn                  Tangenter")
sp( HEADER_BOOT_MENU = "  # Dev Particiones PC Nombre                   Llaves")
de( HEADER_BOOT_MENU = "  # Dev Partitions PP  Name                      Tast.")
ru( HEADER_BOOT_MENU = "  # Dev Partitions PP  Name                       Keys")
fr( HEADER_BOOT_MENU = "  # Dev Partitions PP  Nom                     Touches")
it( HEADER_BOOT_MENU = "  # Dev Partizioni PC  Numero                    Tasti")
nl( HEADER_BOOT_MENU = "  # Sta Partitie   PP  Beschrijving            Toetsen")
cs( HEADER_BOOT_MENU = "  # za˝ Oblast     PP  JmÇno                     Kl†v.")

en( MENU_HIDE     = " Hide partition")
se( MENU_HIDE     = " Dîlj partition")
sp( MENU_HIDE     = " Esconder Partici¢n")
de( MENU_HIDE     = " Verstecke Partition")
ru( MENU_HIDE     = " ëØ‡Ô‚†‚Ï ‡†ß§•´")
fr( MENU_HIDE     = " Cacher partition")
it( MENU_HIDE     = " Nascondere Partizione")
nl( MENU_HIDE     = " Partitie verbergen")
cs( MENU_HIDE     = " Skryt° oblasti")

en( MENU_FORMAT   = " Format partition")
se( MENU_FORMAT   = " Formatera partition")
sp( MENU_FORMAT   = " Formatear Partici¢n")
de( MENU_FORMAT   = " Formatiere Partition")
ru( MENU_FORMAT   = " é‚‰Æ‡¨†‚®‡Æ¢†‚Ï ‡†ß§•´")
fr( MENU_FORMAT   = " Formatter partition")
it( MENU_FORMAT   = " Formattare Partizione")
nl( MENU_FORMAT   = " Partitie formatteren")
cs( MENU_FORMAT   = " Form†tov†n° oblasti")
 
en( MENU_VERIFY   = " Verify disk surface")
se( MENU_VERIFY   = " Verifiera disk yta")
sp( MENU_VERIFY   = " Verificar Cara de Disco")
de( MENU_VERIFY   = " OberflÑchentest")
ru( MENU_VERIFY   = " Ç•‡®‰®™†Ê®Ô ØÆ¢•‡Â≠Æ·‚®")
fr( MENU_VERIFY   = " VÇrifier partition")
it( MENU_VERIFY   = " Verifica superficie del disco")
nl( MENU_VERIFY   = " Oppervlaktecontrole")
cs( MENU_VERIFY   = " Test povrchu")
 
en( MENU_PREVIEW  = " Preview partition table")
se( MENU_PREVIEW  = " Titta pÜ partitions tabell")
sp( MENU_PREVIEW  = " Partici¢n Tabla Previa")
de( MENU_PREVIEW  = " Partitionstabelle anzeigen")
ru( MENU_PREVIEW  = " è‡Æ·¨Æ‚‡ ‚†°´®ÊÎ ‡†ß§•´Æ¢")
fr( MENU_PREVIEW  = " Voir table de partitions")
it( MENU_PREVIEW  = " Anteprima tabella partizione")
nl( MENU_PREVIEW  = " Partitietabel bekijken")
cs( MENU_PREVIEW  = " Zobrazen° tabulky oblast°")
 
en( MENU_INST_IPL = " Load custom IPL")
se( MENU_INST_IPL = " Ladda egen IPL")
sp( MENU_INST_IPL = " Cargar adaptado IPL")
de( MENU_INST_IPL = " Lade eigenen IPL")
ru( MENU_INST_IPL = " ì·‚†≠Æ¢®‚Ï ·¢Æ© IPL")
fr( MENU_INST_IPL = " Charger IPL perso")
it( MENU_INST_IPL = " Caricare IPL personale")
nl( MENU_INST_IPL = " Eigen IPL laden")
cs( MENU_INST_IPL = " Zaveden° ußivatelskÇho IPL")
 
en( MENU_SAVE_MBR = " Save MBR to file")
se( MENU_SAVE_MBR = " Spara MBR till en fil")
sp( MENU_SAVE_MBR = " Salvar MBR en un Archivo")
de( MENU_SAVE_MBR = " Speichere MBR in Datei")
ru( MENU_SAVE_MBR = " ëÆÂ‡†≠®‚Ï MBR ¢ ‰†©´")
fr( MENU_SAVE_MBR = " Sauver MBR dans un fichier")
it( MENU_SAVE_MBR = " Salva MBR su file")
nl( MENU_SAVE_MBR = " MBR naar bestand opslaan")
cs( MENU_SAVE_MBR = " Èschova MBR do souboru")

en( MENU_SAVE_ADV = " Save ADV data to file")
se( MENU_SAVE_ADV = " Save ADV data to file")
sp( MENU_SAVE_ADV = " Save ADV data to file")
de( MENU_SAVE_ADV = " Speichere ADV Daten in Datei")
ru( MENU_SAVE_ADV = " Save ADV data to file")
fr( MENU_SAVE_ADV = " Save ADV data to file")
it( MENU_SAVE_ADV = " Save ADV data to file")
nl( MENU_SAVE_ADV = " Save ADV data to file")
cs( MENU_SAVE_ADV = " Save ADV data to file")

en( MENU_LOAD_MBR = " Load MBR from file")
se( MENU_LOAD_MBR = " Ladda MBR frÜn en fil")
sp( MENU_LOAD_MBR = " Cargar MBR desde Archivo")
de( MENU_LOAD_MBR = " Lade MBR aus Datei")
ru( MENU_LOAD_MBR = " á†£‡„ß®‚Ï MBR ®ß ‰†©´†")
fr( MENU_LOAD_MBR = " Charger MBR d'un fichier")
it( MENU_LOAD_MBR = " Carica MBR da file")
nl( MENU_LOAD_MBR = " MBR uit bestand laden")
cs( MENU_LOAD_MBR = " Obnova MBR ze souboru")

en( MENU_LOAD_ADV = " Load ADV data from file")
se( MENU_LOAD_ADV = " Load ADV data from file")
sp( MENU_LOAD_ADV = " Load ADV data from file")
de( MENU_LOAD_ADV = " Lade ADV Daten von Datei")
ru( MENU_LOAD_ADV = " Load ADV data from file")
fr( MENU_LOAD_ADV = " Load ADV data from file")
it( MENU_LOAD_ADV = " Load ADV data from file")
nl( MENU_LOAD_ADV = " Load ADV data from file")
cs( MENU_LOAD_ADV = " Load ADV data from file")

en( MENU_ADV_UNINST = " Uninstall boot manager")
se( MENU_ADV_UNINST = " Avinstallera boot hanterare")
sp( MENU_ADV_UNINST = " Desinstalar boot director")
de( MENU_ADV_UNINST = " Boot Manager deinstallieren")
ru( MENU_ADV_UNINST = " ì§†´®‚Ï Ì‚Æ‚ ß†£‡„ßÁ®™")
fr( MENU_ADV_UNINST = " DÇsinstaller le boot manager")
it( MENU_ADV_UNINST = " Disinstalla Boot Manager")
nl( MENU_ADV_UNINST = " Boot manager de-installeren")
cs( MENU_ADV_UNINST = " Odinstalov†n° Boot Manageru")

en( MENU_INSTALL   = " Install compact boot manager, or A - Advanced")
se( MENU_INSTALL   = " Installera IPL boot hanterare el A - Avancerad")
sp( MENU_INSTALL   = " Instalar boot Director compacto, A - Avansado")
de( MENU_INSTALL   = " Installiere Boot Manager,  oder  A - Advanced")
ru( MENU_INSTALL   = " ì·‚†≠Æ¢®‚Ï ™Æ¨Ø†™‚≠Î© ß†£‡„ßÁ®™, . - Advanced")
fr( MENU_INSTALL   = " Installer boot manager compact / A - AvancÇ")
it( MENU_INSTALL   = " Installare BootManager compatto, A - Avanzato")
nl( MENU_INSTALL   = " Boot manager installeren    /    A - Geavanceerd")
cs( MENU_INSTALL   = " Instalace Boot Manageru,   nebo  A - rozÁ°˝enÇho")
 
en( MENU_UNINSTALL = " Uninstall boot manager by loading standard IPL")
se( MENU_UNINSTALL = " Avinstallera genom att installera en standard IPL")
sp( MENU_UNINSTALL = " Desinstalar boot Director para montar normal IPL")
de( MENU_UNINSTALL = " Deinstalliere Boot Manager durch Standard IPL")
ru( MENU_UNINSTALL = " ì§†´®‚Ï ß†£‡„ßÁ®™, „·‚†≠Æ¢®¢ ·‚†§†‡‚≠Î© DOS IPL")
fr( MENU_UNINSTALL = " Remplacer le boot manager par l'IPL standard")
it( MENU_UNINSTALL = " Disintalla Boot Manager per caricare IPL standard")
nl( MENU_UNINSTALL = " Bootmanager de-installeren door standaard IPL te laden")
cs( MENU_UNINSTALL = " Odstranÿn° Boot Manageru a zaps†n° standardn°ho IPL")

en( MENU_INST_EXT = " Install a \"non bootable\" message into EMBR")
se( MENU_INST_EXT = " Installera ett \"icke bootbar\" meddelande i EMBR")
sp( MENU_INST_EXT = " Instalar un \"non bootable\" mensage dentro EMBR")
de( MENU_INST_EXT = " Installiere eine \"kein System\" Meldung im EMBR")
ru( MENU_INST_EXT = " ì·‚†≠Æ¢®‚Ï Ø„·‚ÎË™„ \"non bootable\" ¢ EMBR")
fr( MENU_INST_EXT = " Installer un message \"non bootable\" dans l'EMBR")
it( MENU_INST_EXT = " Installare un \"non bootable\" messaggio nel EMBR")
nl( MENU_INST_EXT = " Een \"non bootable\" bericht in de EMBR installeren")
cs( MENU_INST_EXT = " Instalov†n° zpr†vy \"zde nen° systÇm\" do EMBR")

en( MENU_MAKE_PRI = " Read help on how to make logical drive bootable")
se( MENU_MAKE_PRI = " LÑs hjÑlpen hur man gîr en logisk drive bootbar")
sp( MENU_MAKE_PRI = " Leer ayuda sobre como hacer la l¢gica unidad bootable")
de( MENU_MAKE_PRI = " Von Logischer Platte booten? - Siehe Hilfetext")
ru( MENU_MAKE_PRI = " èÆ¨ÆÈÏ Æ ‚Æ¨ ™†™ ·§•´†‚Ï Ì‚Æ‚ ‡†ß§•´ ß†£‡„ßÆÁ≠Î¨")
fr( MENU_MAKE_PRI = " Lire l'aide pour rendre un disque logique bootable")
it( MENU_MAKE_PRI = " Leggere l'aiuto per rendere un disco logico bootstrappabile")
nl( MENU_MAKE_PRI = " Lees help over hoe een station bootable te maken")
cs( MENU_MAKE_PRI = " N†povÿda k zav†dÿn° z logickÇho za˝°zen°")


en( ERROR_MALLOC    = "Cannot allocate more memory.")
se( ERROR_MALLOC    = "Kunde inte allokera minne med malloc.")
sp( ERROR_MALLOC    = "No pudo asignar mas memoria.")
de( ERROR_MALLOC    = "Nicht genÅgend Speicher.")
ru( ERROR_MALLOC    = "ç•§Æ·‚†‚ÆÁ≠Æ Ø†¨Ô‚®.")
fr( ERROR_MALLOC    = "Impossible d'allouer plus de mÇmoire.")
it( ERROR_MALLOC    = "Impossibile assegnare piu' memoria.")
nl( ERROR_MALLOC    = "Er is onvoldoende geheugen beschikbaar.")
cs( ERROR_MALLOC    = "M†lo pamÿti.")

en( ERROR_DISK_INFO = "Error getting hard disk parameters.")
se( ERROR_DISK_INFO = "Fel vid lÑsning av hÜrd disk parametrar.")
sp( ERROR_DISK_INFO = "Error consiguiendo par†metros del disco duro.")
de( ERROR_DISK_INFO = "Fehler beim Zugiff auf Festplatte.")
ru( ERROR_DISK_INFO = "éË®°™† Ø‡® Á‚•≠®® Ø†‡†¨•‚‡Æ¢ §®·™†.")
fr( ERROR_DISK_INFO = "Impossible d'obtenir les paramätres du disque dur.")
it( ERROR_DISK_INFO = "Errore nel ricavare i parametri del disco.")
nl( ERROR_DISK_INFO = "Fout tijdens ophalen van vaste schijf parameters.")
cs( ERROR_DISK_INFO = "Chyba p˝i p˝°stupu na disk.")

en( ERROR_FIX_FIRST = "You should correct errors first.")
se( ERROR_FIX_FIRST = "Du borde fixa partitions fel fîrst.")
sp( ERROR_FIX_FIRST = "Debe corregir errores primero.")
de( ERROR_FIX_FIRST = "Bitte zuerst Fehler korrigieren.")
ru( ERROR_FIX_FIRST = "ë≠†Á†´† ≠•Æ°ÂÆ§®¨Æ ®·Ø‡†¢®‚Ï ÆË®°™®.")
fr( ERROR_FIX_FIRST = "Corrigez d'abord les erreurs.")
it( ERROR_FIX_FIRST = "Correggere gli errori prima di continuare.")
nl( ERROR_FIX_FIRST = "U moet eerst de fouten corrigeren.")
cs( ERROR_FIX_FIRST = "Nap˝ed pros°m odstraúe chybu.")

en( ERROR_INH_INVAL = "You cannot save object if one of its parents is invalid or is not saved.")
se( ERROR_INH_INVAL = "Du kan inte spara objektet om en av dens fîrÑldrar Ñr ogiltig eller inte sparad.")
sp( ERROR_INH_INVAL = "No puede salvar objeto si uno de sus parientes es inv†lido o no es salvado.")
de( ERROR_INH_INVAL = "Ein Åbergeordnetes Element ist (noch) ungÅltig - kann nicht speichern.")
ru( ERROR_INH_INVAL = "ç• ¨Æ£„ ·ÆÂ‡†≠®‚Ï - Æ§®≠ ®ß ‡Æ§®‚•´•© ≠• ·ÆÂ‡†≠•≠ ®´® ·Æ§•‡¶®‚ ÆË®°™®.")
fr( ERROR_INH_INVAL = "Impossible de sauver l'objet, un de ses parents est invalide ou non sauvÇ.")
it( ERROR_INH_INVAL = "Non e' possibile salvare oggetti se uno dei suoi collegati non e' valido o non salvato.")
nl( ERROR_INH_INVAL = "Een object kan niet opgeslagen worden, als het bovenliggende object ongeldig is.")
cs( ERROR_INH_INVAL = "Nelze uloßit, protoße nad˝°zenÏ objekt je vadnÏ nebo nen° dosud uschov†n.")

en( ERROR_READ_MBR  = "Error reading MBR from disk.")
se( ERROR_READ_MBR  = "Fel vid lÑsning av MBR frÜn disk.")
sp( ERROR_READ_MBR  = "Error leyendo MBR del disco.")
de( ERROR_READ_MBR  = "Fehler beim Lesen des MBR.")
ru( ERROR_READ_MBR  = "éË®°™† Ø‡® Á‚•≠®® MBR · §®·™†.")
fr( ERROR_READ_MBR  = "Erreur de lecture du MBR du disque.")
it( ERROR_READ_MBR  = "Errore nella lettura del MBR dal disco.")
nl( ERROR_READ_MBR  = "Fout tijdens het lezen van de MBR.")
cs( ERROR_READ_MBR  = "Chyba p˝i üten° MBR.")

en( ERROR_SAVE_MBR  = "Error writing MBR to disk.")
se( ERROR_SAVE_MBR  = "Fel vid skrivning av MBR till disk.")
sp( ERROR_SAVE_MBR  = "Error escribiendo MBR al disco.")
de( ERROR_SAVE_MBR  = "Fehler beim Schreiben des MBR.")
ru( ERROR_SAVE_MBR  = "éË®°™† ß†Ø®·® MBR ≠† §®·™.")
fr( ERROR_SAVE_MBR  = "Erreur d'Çcriture du MBR du disque.")
it( ERROR_SAVE_MBR  = "Errore nella scrittura del MBR sul disco.")
nl( ERROR_SAVE_MBR  = "Fout tijdens het schrijven van de MBR.")
cs( ERROR_SAVE_MBR  = "Chyba p˝i z†pisu MBR.")

en( ERROR_READ_ADV  = "Error reading Advanced MBR from disk.")
se( ERROR_READ_ADV  = "Fel vid lÑsning av Avancerad MBR frÜn disk.")
sp( ERROR_READ_ADV  = "Error leyendo Avansado MBR desde disco.")
de( ERROR_READ_ADV  = "Lesefehler beim Advanced MBR.")
ru( ERROR_READ_ADV  = "éË®°™† Ø‡® Á‚•≠®® Advanced MBR · §®·™†.")
fr( ERROR_READ_ADV  = "Erreur de lecture du MBR AvancÇ du disque.")
it( ERROR_READ_ADV  = "Errore nella lettura del MBR Avanzato dal disco.")
nl( ERROR_READ_ADV  = "Fout bij het lezen van de geavanceerde MBR.")
cs( ERROR_READ_ADV  = "Chyba p˝i üten° rozÁ°˝enÇho MBR.")

en( ERROR_SAVE_ADV  = "Error writing Advanced MBR to disk.")
se( ERROR_SAVE_ADV  = "Fel vid skrivning av Avancerad MBR till disk.")
sp( ERROR_SAVE_ADV  = "Error escribiendo Avansado MBR al diso.")
de( ERROR_SAVE_ADV  = "Schreibfehler beim Advanced MBR.")
ru( ERROR_SAVE_ADV  = "éË®°™† ß†Ø®·® Advanced MBR ≠† §®·™.")
fr( ERROR_SAVE_ADV  = "Erreur d'Çcriture du MBR AvancÇ du disque.")
it( ERROR_SAVE_ADV  = "Errore nella scrittura del MBR Avanzato sul disco.")
nl( ERROR_SAVE_ADV  = "Fout bij het schrijven van de geavanceerde MBR.")
cs( ERROR_SAVE_ADV  = "Chyba p˝i z†pisu rozÁ°˝enÇho MBR.")

en( ERROR_LOAD_FILE = "Error reading from file.")
se( ERROR_LOAD_FILE = "Fel vid lÑsning frÜn fil.")
sp( ERROR_LOAD_FILE = "Error leyendo desde archivo.")
de( ERROR_LOAD_FILE = "Fehler beim Lesen der Datei.")
ru( ERROR_LOAD_FILE = "éË®°™† Ø‡® Á‚•≠®® ®ß ‰†©´†.")
fr( ERROR_LOAD_FILE = "Erreur de lecture d'un fichier.")
it( ERROR_LOAD_FILE = "Errore nella lettura da file.")
nl( ERROR_LOAD_FILE = "Fout bij het lezen van het bestand.")
cs( ERROR_LOAD_FILE = "Chyba p˝i üten° souboru.")

en( ERROR_SAVE_FILE = "Error writing to file.")
se( ERROR_SAVE_FILE = "Fel vid skrivning till fil.")
sp( ERROR_SAVE_FILE = "Error escribiendo al archivo.")
de( ERROR_SAVE_FILE = "Fehler beim Schreiben in Datei.")
ru( ERROR_SAVE_FILE = "éË®°™† Ø‡® ß†Ø®·® ¢ ‰†©´.")
fr( ERROR_SAVE_FILE = "Erreur d'Çcriture d'un fichier.")
it( ERROR_SAVE_FILE = "Errore nella scrittura su file.")
nl( ERROR_SAVE_FILE = "Fout bij het schrijven naar het bestand.")
cs( ERROR_SAVE_FILE = "Chyba p˝i z†pisu souboru.")

en( WARN_INVALID    = "Some records in the partition table are invalid.")
se( WARN_INVALID    = "En eller flera poster i partitions tabellen Ñr ogiltiga.")
sp( WARN_INVALID    = "Alg£nos records en la tabla de partici¢n son invalidos.")
de( WARN_INVALID    = "Einige EintrÑge in der Tabelle sind ungÅltig.")
ru( WARN_INVALID    = "è†‡†¨•‚‡Î Æ§≠Æ£Æ ®´® °Æ´•• ‡†ß§•´Æ¢ ÆË®°ÆÁ≠Î.")
fr( WARN_INVALID    = "Des zones de la table de partition sont invalides.")
it( WARN_INVALID    = "Alcuni records nella tabella delle partizioni non sono validi.")
nl( WARN_INVALID    = "Sommige records in de partitie tabel zijn ongeldig.")
cs( WARN_INVALID    = "NÿkterÇ £daje v tabulce oblast° jsou neplatnÇ.")

en( MESG_BIOS_CYL   = "Note that BIOS reports only %d cylinders.")
se( MESG_BIOS_CYL   = "Note that BIOS reports only %d cylinders.")
sp( MESG_BIOS_CYL   = "Recuerde que BIOS reporta unicamente %d cylindros.")
de( MESG_BIOS_CYL   = "Achtung! Das BIOS meldet nur %d Zylinder.")
ru( MESG_BIOS_CYL   = "ìÁ‚®‚• Á‚Æ BIOS ØÆ™†ßÎ¢†•‚ ‚Æ´Ï™Æ %d Ê®´®≠§‡†.")
fr( MESG_BIOS_CYL   = "Note: le BIOS ne reporte que %d cylindres.")
it( MESG_BIOS_CYL   = "Attenzione: il BIOS riporta solo %d cilindri.")
nl( MESG_BIOS_CYL   = "Let erop dat het BIOS slechts %d cylinders ondersteunt.")
cs( MESG_BIOS_CYL   = "Pozor! BIOS ozn†mil pouze %d cylindrÖ.")

en( ERROR_ADV_NOSPACE = "You don't have enough free space to install Boot Manager")
se( ERROR_ADV_NOSPACE = "Du har inte tillrÑckligt med ledigt diskutrymme fîr att installera BtMgr")
sp( ERROR_ADV_NOSPACE = "No hay suficiente espacio para instalar Boot Manager")
de( ERROR_ADV_NOSPACE = "Nicht genÅgend Platz fÅr den Boot Manager vorhanden.")
ru( ERROR_ADV_NOSPACE = "ç•§Æ·‚†‚ÆÁ≠Æ ·¢Æ°Æ§≠Æ£Æ ¨•·‚† Á‚Æ°Î „·‚†≠Æ¢®‚Ï ß†£‡„ßÁ®™")
fr( ERROR_ADV_NOSPACE = "Pas assez d'espace disque libre pour installer Boot Manager")
it( ERROR_ADV_NOSPACE = "Non c'e' spazio sufficiente per installare Boot Manager.")
nl( ERROR_ADV_NOSPACE = "Niet genoeg vrije ruimte om Boot Manager te installeren.")
cs( ERROR_ADV_NOSPACE = "Nen° dostatek prostoru pro instalaci Boot Manageru.")

en( ERROR_NO_ADV    = "You must have Boot Manager partition of sufficient size.")
se( ERROR_NO_ADV    = "Du mÜste ha en partition till boot hanteraren av tillrÑcklig storlek.")
sp( ERROR_NO_ADV    = "Usted debe tenere Boot Manager partici¢n de suficiente medida.")
de( ERROR_NO_ADV    = "Die Boot Manager Partition ist nicht gro· genug.")
ru( ERROR_NO_ADV    = "é‚·„‚·‚„•‚ §Æ·‚†‚ÆÁ≠Æ °Æ´ÏËÆ© ‡†ß§•´ §´Ô Boot Manager")
fr( ERROR_NO_ADV    = "La partition de Boot Manager n'est pas assez grande.")
it( ERROR_NO_ADV    = "Si deve avere la partizione di Boot Manager di dimensioni sufficienti.")
nl( ERROR_NO_ADV    = "De Boot Manager partitie is niet groot genoeg.")
cs( ERROR_NO_ADV    = "Oblast pro Boot Manager je p˝°liÁ mal†.")

en( ERROR_ADV_BAD   = "Advanced MBR doesn't have valid signature.")
se( ERROR_ADV_BAD   = "Avancerade MBR har inte en giltig signatur.")
sp( ERROR_ADV_BAD   = "Advansado MBR no tiene firma v†lida.")
de( ERROR_ADV_BAD   = "Der Advanced MBR hat keine gÅltige Signatur.")
ru( ERROR_ADV_BAD   = "Advanced MBR doesn't have valid signature.")
fr( ERROR_ADV_BAD   = "Le MBR AvancÇ a une signature invalide.")
it( ERROR_ADV_BAD   = "L'MBR Avanzato non ä valido.")
nl( ERROR_ADV_BAD   = "De geavanceerde MBR is niet geldig.")
cs( ERROR_ADV_BAD   = "RozÁ°˝enÏ MBR nem† platnou znaüku.")

en( ERROR_ADV_VER   = "Warning! Advanced MBR has newer version than this program.")
se( ERROR_ADV_VER   = "Warning! Avancerade MBR har nyare version Ñn detta program.")
sp( ERROR_ADV_VER   = "Warning! Advansado MBR tiene una versi¢n mas reciente.")
de( ERROR_ADV_VER   = "Achtung! Der Advanced MBR ist neuer als dieses Programm.")
ru( ERROR_ADV_VER   = "Warning! Advanced MBR °Î´ ·Æß§†≠ °Æ´•• ≠Æ¢Æ© ¢•‡·®•© Ì‚Æ© Ø‡Æ£‡†¨¨Î")
fr( ERROR_ADV_VER   = "Attention! Le MBR AvancÇ est plus rÇcent que ce programme.")
it( ERROR_ADV_VER   = "L'MBR Avanzato attuale ä pió recente di questo programma.")
nl( ERROR_ADV_VER   = "Waarschuwing! De MBR heeft een nieuwere versie dan dit programma.")
cs( ERROR_ADV_VER   = "Pozor! RozÁ°˝enÏ MBR je novÿjÁ° neß tento program.")

en( ERROR_GR_FOUR   = "Cannot uninstall, because there are more than four partitions.")
se( ERROR_GR_FOUR   = "Kan ej avinstallera fîr att du har mer Ñn 4 partitioner.")
sp( ERROR_GR_FOUR   = "No puede desinstalar, porque hay mas de cuatros partici¢nes.")
de( ERROR_GR_FOUR   = "Deinstallieren unmîglich - es sind mehr als vier Partitionen.")
ru( ERROR_GR_FOUR   = "ì ¢†· °Æ´ÏË• Á•¨ Á•‚Î‡• ‡†ß§•´† - Æ≠® ≠• ¢´•ß„‚ ¢ MBR.")
fr( ERROR_GR_FOUR   = "Impossible de dÇsinstaller car il y a plus de 4 partitions.")
it( ERROR_GR_FOUR   = "Impossibile disinstallare: ci sono pió di quattro partizioni.")
nl( ERROR_GR_FOUR   = "Onmogelijk te installeren, omdat er meer dan 4 partities zijn.")
cs( ERROR_GR_FOUR   = "Odinstalov†n° nen° moßnÇ, protoße je zde v°ce jak üty˝i oblasti.")

en( ERROR_NO_UNUSED = "Not enough unused entries in the partition table!")
se( ERROR_NO_UNUSED = "Inte nog med oanvÑnda poster i partitions tabellen!")
sp( ERROR_NO_UNUSED = "No hay suficientes entradas libres en tabla de particion!")
de( ERROR_NO_UNUSED = "Nicht genÅgend freie EintrÑge in der Tabelle!")
ru( ERROR_NO_UNUSED = "Not enough unused entries in the partition table!")
fr( ERROR_NO_UNUSED = "Pas assez d'entrÇes libres dans la table de partition!")
it( ERROR_NO_UNUSED = "Non ci sono sufficienti entrate libere nella tabella delle partizioni!")
nl( ERROR_NO_UNUSED = "Niet genoeg ongebruikte ingangen in de partitie tabel!")
cs( ERROR_NO_UNUSED = "V tabulce oblast° nen° dostatek volnÏch z†znamÖ!")

en( ERR_BOOTREC = "   boot record")
se( ERR_BOOTREC = "   boot record")
sp( ERR_BOOTREC = "   boot record")
de( ERR_BOOTREC = "   boot record")
ru( ERR_BOOTREC = "   boot record")
fr( ERR_BOOTREC = "   boot record")
it( ERR_BOOTREC = "   boot record")
nl( ERR_BOOTREC = "   boot record")
cs( ERR_BOOTREC = "   zav†dÿc° z†znam")

en( ERR_INCONS  = "  inconsistent")
se( ERR_INCONS  = "  inkonsistent")
sp( ERR_INCONS  = " inconsistente")
de( ERR_INCONS  = "  inkonsistent")
ru( ERR_INCONS  = "≠•·ÆÆ‚¢•‚·‚¢®•")
fr( ERR_INCONS  = "    incohÇrent")
it( ERR_INCONS  = " inconsistente")
nl( ERR_INCONS  = "  inconsistent")
cs( ERR_INCONS  = "nekonzistentn°")

en( ERR_OVERLAP = " overlapped")
se( ERR_OVERLAP = "îverlappande")
sp( ERR_OVERLAP = "    solapar")
de( ERR_OVERLAP = "Åberlappend")
ru( ERR_OVERLAP = "Ø•‡•·•Á•≠®•")
fr( ERR_OVERLAP = "  superposÇ")
it( ERR_OVERLAP = "sovrapposto")
nl( ERR_OVERLAP = "overlappend")
cs( ERR_OVERLAP = "p˝ekrÏvaj°c° se")

en( ERR_RANGE   = "   range")
se( ERR_RANGE   = "   range")
sp( ERR_RANGE   = "   rango")
de( ERR_RANGE   = " Bereich")
ru( ERR_RANGE   = " £‡†≠®Ê†")
fr( ERR_RANGE   = "   plage")
it( ERR_RANGE   = "interval")
nl( ERR_RANGE   = "  bereik")
cs( ERR_RANGE   = "  rozsah")

en( ERR_MBR     = "  mbr")
se( ERR_MBR     = "  mbr")
sp( ERR_MBR     = "  mbr")
de( ERR_MBR     = "  MBR")
ru( ERR_MBR     = "  mbr")
fr( ERR_MBR     = "  MBR")
it( ERR_MBR     = "  MBR")
nl( ERR_MBR     = "  MBR")
cs( ERR_MBR     = "  MBR")

en( HINT_ADV    = "Space - edit boot menu   Ins - select system type   Enter - setup")
se( HINT_ADV    = "Space - editera meny     Ins - vÑlj system typ      Enter - Konfigurera")
sp( HINT_ADV    = "Space - editar men£      Ins - escoje tipo sistema  Enter - instalar")
de( HINT_ADV    = "Space - Menu bearbeiten  Ins - Systemtyp wÑhlen     Enter - Setup")
ru( HINT_ADV    = "Space - ‡•§†™Ê®Ô ¨•≠Ó    Ins - ¢Î°Æ‡ ‚®Ø† ·®·‚•¨Î   Enter - ™Æ≠‰®£.")
fr( HINT_ADV    = "Espace- editer menu boot Ins - choix syst. fichiers EntrÇe- Config.")
it( HINT_ADV    = "Spazio - modifica menu   Ins - seleziona tipo sistema  Enter - Setup")
nl( HINT_ADV    = "Spatie - Menu bewerken   Ins - Systeem type sel.    Enter - Config.")
cs( HINT_ADV    = "<mezera> - nab°dka       <Ins> - volba soub. syst.  <Enter> - nastaven°")

en( HINT_INS    = "Space - toggle active    Ins - select system type   Enter - setup")
se( HINT_INS    = "Space - Ñndra aktiv      Ins - vÑlj system typ      Enter - Konfigurera")
sp( HINT_INS    = "Space - cambia activo    Ins - escoje tipo sistema  Enter - instalar")
de( HINT_INS    = "Space - Startbar ein/aus Ins - Systemtyp wÑhlen     Enter - Setup")
ru( HINT_INS    = "Space - ¢Î°‡†‚Ï ‚•™„È®©  Ins - ¢Î°Æ‡ ‚®Ø† ·®·‚•¨Î   Enter - ™Æ≠‰®£.")
fr( HINT_INS    = "Espace- active/inactive  Ins - choix syst. fichiers EntrÇe- Config.")
it( HINT_INS    = "Spazio - cambia attivo   Ins - seleziona tipo sistema  Enter - Setup")
nl( HINT_INS    = "Spatie - Actief/inactief Ins - Systeem type sel.    Enter - Config.")
cs( HINT_INS    = "<mezera> - mÿn° aktivn°  <Ins> - volba soub. syst.  <Enter> - nastaven°")

en( HINT_CHS    = "You can use + and - keys   Cyl=%lu..%lu  Side=0..%d  Sect=1..%d")
se( HINT_CHS    = "Du kan anvÑnda + och -     Cyl=%lu..%lu  Sida=0..%d  Sekt=1..%d")
sp( HINT_CHS    = "Puede usar + y - teclas    Cyl=%lu..%lu  Side=0..%d  Sect=1..%d")
de( HINT_CHS    = "Mit den Tasten + und -     Zyl=%lu..%lu  Kopf=0..%d  Sekt=1..%d")
ru( HINT_CHS    = "åÆ¶≠Æ ®·ØÆ´ÏßÆ¢†‚Ï + ® -   ñ®´=%lu..%lu  ë‚Æ‡=0..%d  ë•™‚=1..%d")
fr( HINT_CHS    = "Presser les touches + et -  Cyl=%lu..%lu  Face=0..%d  Sect=1..%d")
it( HINT_CHS    = "Usare tasti + e -          Cil=%lu..%lu  Test=0..%d  Sett=1..%d")
nl( HINT_CHS    = "Gebruik de + en - toetsen  Cyl=%lu..%lu  Kop=0..%d  Sect=1..%d")
cs( HINT_CHS    = "MÖßete pouß°t + a -       cyl=%lu..%lu  hlava=0..%d  sekt=1..%d")

en( HINT_LBA    = "You can use + and - keys   Starting=%lu..%lu   Sectors=1..%lu")
se( HINT_LBA    = "Du kan anvÑnda + och -     Starting=%lu..%lu   Sektorer=1..%lu")
sp( HINT_LBA    = "Puede usar + y - teclas    Comenzando=%lu..%lu   Sectores=1..%lu")
de( HINT_LBA    = "Mit den Tasten + und -     Start=%lu..%lu   Sektoren=1..%lu")
ru( HINT_LBA    = "åÆ¶≠Æ ®·ØÆ´ÏßÆ¢†‚Ï + ® -   ç†Á†´Æ=%lu..%lu   äÆ´®Á•·‚¢Æ=1..%lu")
fr( HINT_LBA    = "Presser les touches + et -  DÇbut=%lu..%lu   Secteurs=1..%lu")
it( HINT_LBA    = "Usare tasti + e -          Inizio=%lu..%lu   Settori=1..%lu")
nl( HINT_LBA    = "Gebruik de + en - toetsen  Begin=%lu..%lu    Sectoren=1..%lu")
cs( HINT_LBA    = "MÖßete pouß°t + a -       poü†teün°=%lu..%lu   sektory=1..%lu")

en( HINT_RETURN = "    Press ESC to return from preview mode")
se( HINT_RETURN = "    Tryck ESC fîr att ÜtervÑnda frÜn fîrhandsvisning")
sp( HINT_RETURN = "    Presione ESC para regresar al modo previo")
de( HINT_RETURN = "    ESC um den Anzeigemodus zu beenden")
ru( HINT_RETURN = "    ç†¶¨®‚• ESC §´Ô ¢Æß¢‡†‚†")
fr( HINT_RETURN = "    Presser Echap pour revenir au mode prÇcÇdent")
it( HINT_RETURN = "    Premere ESC per uscire dall'anteprima")
nl( HINT_RETURN = "    Druk ESC om de preview-modus te beeindigen.") 
cs( HINT_RETURN = "    Stisknut°m <ESC> ukonü°te zobrazen°")

en( MESG_MBR_SAVED  = "MBR was saved to hard disk.")
se( MESG_MBR_SAVED  = "MBR Ñr sparad till hÜrd disk.")
sp( MESG_MBR_SAVED  = "MBR fue guardado en el disco duro.")
de( MESG_MBR_SAVED  = "MBR wurde geschrieben.")
ru( MESG_MBR_SAVED  = "MBR ·ÆÂ‡†≠Ò≠ ≠† §®·™.")
fr( MESG_MBR_SAVED  = "MBR sauvÇ sur le disque dur.")
it( MESG_MBR_SAVED  = "L'MBR ä stato salvato sul disco.")
nl( MESG_MBR_SAVED  = "MBR is opgeslagen.")
cs( MESG_MBR_SAVED  = "MBR byl zaps†n na disk.")

en( MESG_ADV_SAVED  = "ADV data and IPL were saved to hard disk.")
se( MESG_ADV_SAVED  = "ADV data and IPL were saved to hard disk.")
sp( MESG_ADV_SAVED  = "ADV data and IPL were saved to hard disk.")
de( MESG_ADV_SAVED  = "ADV data and IPL were saved to hard disk.")
ru( MESG_ADV_SAVED  = "ADV data and IPL were saved to hard disk.")
fr( MESG_ADV_SAVED  = "ADV data and IPL were saved to hard disk.")
it( MESG_ADV_SAVED  = "ADV data and IPL were saved to hard disk.")
nl( MESG_ADV_SAVED  = "ADV data and IPL were saved to hard disk.")
cs( MESG_ADV_SAVED  = "ADV data and IPL were saved to hard disk.")

en( MESG_FILE_SAVED = "MBR was saved to file.")
se( MESG_FILE_SAVED = "MBR Ñr sparad till fil.")
sp( MESG_FILE_SAVED = "MBR fue guardado al archivo.")
de( MESG_FILE_SAVED = "MBR wurde gespeichert.")
ru( MESG_FILE_SAVED = "î†©´ ·ÆÂ‡†≠Ò≠ ≠† §®·™.")
fr( MESG_FILE_SAVED = "MBR sauvÇ dans un fichier.")
it( MESG_FILE_SAVED = "L'MBR ä stato salvato su file.")
nl( MESG_FILE_SAVED = "MBR is naar bestand geschreven.")
cs( MESG_FILE_SAVED = "MBR byl uschov†n do souboru.")

en( MESG_ADVFILE_SAVED = "ADV data was saved to file.")
se( MESG_ADVFILE_SAVED = "ADV data was saved to file.")
sp( MESG_ADVFILE_SAVED = "ADV data was saved to file.")
de( MESG_ADVFILE_SAVED = "ADV data was saved to file.")
ru( MESG_ADVFILE_SAVED = "ADV data was saved to file.")
fr( MESG_ADVFILE_SAVED = "ADV data was saved to file.")
it( MESG_ADVFILE_SAVED = "ADV data was saved to file.")
nl( MESG_ADVFILE_SAVED = "ADV data was saved to file.")
cs( MESG_ADVFILE_SAVED = "ADV data was saved to file.")

en( MESG_FILE_LOADD = "New MBR data was loaded from file. Press F2 to save changes.")
se( MESG_FILE_LOADD = "Ny MBR data Ñr laddad frÜn fil. Tryck F2 fîr att spara Ñndringen.")
sp( MESG_FILE_LOADD = "Nuevo MBR datos cargado del archivo. Presione F2 para guardar cambios.")
de( MESG_FILE_LOADD = "Neuer MBR wurde eingeladen. F2 um ihn zu schreiben.")
ru( MESG_FILE_LOADD = "çÆ¢Î© MBR ß†£‡„¶•≠ ¢ Ø†¨Ô‚Ï. ç†¶¨®‚• F2 Á‚Æ°Î ß†Ø®·†‚Ï ®ß¨•≠•≠®Ô.")
fr( MESG_FILE_LOADD = "Nouveau MBR chargÇ d'un fichier. Presser F2 pour enregister.")
it( MESG_FILE_LOADD = "I nuovi dati del MBR sono stati caricati dal file. Premere F2 per salvare le modifiche.")
nl( MESG_FILE_LOADD = "Nieuwe MBR is geladen. Druk op F2 om de veranderingen op te slaan.")
cs( MESG_FILE_LOADD = "NovÏ MBR byl zaveden ze souboru. Stisknut°m <F2> jej zap°Áete na disk.")

en( MESG_ADVFILE_LOADD = "New ADV data was loaded from file. Press F2 to save changes.")
se( MESG_ADVFILE_LOADD = "Ny ADV data Ñr laddad frÜn fil. Tryck F2 fîr att spara Ñndringen.")
sp( MESG_ADVFILE_LOADD = "Nuevo ADV datos cargado del archivo. Presione F2 para guardar cambios.")
de( MESG_ADVFILE_LOADD = "Neuer ADV wurde eingeladen. F2 um ihn zu schreiben.")
ru( MESG_ADVFILE_LOADD = "çÆ¢Î© MBR ß†£‡„¶•≠ ¢ Ø†¨Ô‚Ï. ç†¶¨®‚• F2 Á‚Æ°Î ß†Ø®·†‚Ï ®ß¨•≠•≠®Ô.")
fr( MESG_ADVFILE_LOADD = "Nouveau ADV chargÇ d'un fichier. Presser F2 pour enregister.")
it( MESG_ADVFILE_LOADD = "I nuovi dati del ADV sono stati caricati dal file. Premere F2 per salvare le modifiche.")
nl( MESG_ADVFILE_LOADD = "Nieuwe ADV is geladen. Druk op F2 om de veranderingen op te slaan.")
cs( MESG_ADVFILE_LOADD = "NovÏ ADV byl zaveden ze souboru. Stisknut°m <F2> jej zap°Áete na disk.")

en( MESG_MBR_SAVED2 = "New MBR data was loaded from file.\nMBR was saved to hard disk.")
se( MESG_MBR_SAVED2 = "Ny MBR data Ñr laddad frÜn fil.\nMBR Ñr sparad till hÜrd disk.")
sp( MESG_MBR_SAVED2 = "Nuevo MBR datos cargado del archivo.\nMBR fue guardado en el disco duro.")
de( MESG_MBR_SAVED2 = "Neuer MBR wurde eingeladen.\nMBR wurde geschrieben.")
ru( MESG_MBR_SAVED2 = "çÆ¢Î© MBR ß†£‡„¶•≠ ¢ Ø†¨Ô‚Ï.\nMBR ·ÆÂ‡†≠Ò≠ ≠† §®·™.")
fr( MESG_MBR_SAVED2 = "Nouveau MBR chargÇ d'un fichier.\nL'ancien MBR est sauvÇ sur le disque dur.")
it( MESG_MBR_SAVED2 = "Nuovi dati del MBR caricati da file.\nMBR salvato su disco rigido.")
nl( MESG_MBR_SAVED2 = "Nieuwe MBR is geladen.\nMBR is naar vaste schijf geschreven.")
cs( MESG_MBR_SAVED2 = "NovÏ MBR byl zaveden ze souboru.\nMBR byl zaps†n na disk.")

en( MESG_ADV_SAVED2 = "New ADV data and IPL were loaded from file.\nADV data and IPL were saved to hard disk.")
se( MESG_ADV_SAVED2 = "New ADV data and IPL were loaded from file.\nADV data and IPL were saved to hard disk.")
sp( MESG_ADV_SAVED2 = "New ADV data and IPL were loaded from file.\nADV data and IPL were saved to hard disk.")
de( MESG_ADV_SAVED2 = "New ADV data and IPL were loaded from file.\nADV data and IPL were saved to hard disk.")
ru( MESG_ADV_SAVED2 = "New ADV data and IPL were loaded from file.\nADV data and IPL were saved to hard disk.")
fr( MESG_ADV_SAVED2 = "New ADV data and IPL were loaded from file.\nADV data and IPL were saved to hard disk.")
it( MESG_ADV_SAVED2 = "New ADV data and IPL were loaded from file.\nADV data and IPL were saved to hard disk.")
nl( MESG_ADV_SAVED2 = "New ADV data and IPL were loaded from file.\nADV data and IPL were saved to hard disk.")
cs( MESG_ADV_SAVED2 = "New ADV data and IPL were loaded from file.\nADV data and IPL were saved to hard disk.")

en( MESG_INSTALL    = "New IPL code was installed in memory. Press F2 to save changes.")
se( MESG_INSTALL    = "Ny IPL kod Ñr installerad i minnet. Tryck F2 fîr att spara Ñndringen.")
sp( MESG_INSTALL    = "Nueva IPL codigo instalado en memoria. Presione F2 para guardar cambios.")
de( MESG_INSTALL    = "Neuer IPL Code wurde geladen. F2 um ihn zu schreiben.")
ru( MESG_INSTALL    = "çÆ¢Î© IPL „·‚†≠Æ¢´•≠ ¢ Ø†¨Ô‚®. ç†¶¨®‚• F2 Á‚Æ°Î ß†Ø®·†‚Ï ®ß¨•≠•≠®Ô.")
fr( MESG_INSTALL    = "Nouveau code IPL installÇ en mÇmoire. Presser F2 pour enregistrer.")
it( MESG_INSTALL    = "Nuovo codice per IPL installato in memoria. Premere F2 per salvare.")
nl( MESG_INSTALL    = "Nieuwe IPL code is geladen. Druk op F2 om de veranderingen op te slaan.")
cs( MESG_INSTALL    = "NovÏ IPL byl zaveden. Stisknut°m <F2> jej zap°Áete na disk.")

en( MESG_UNINSTALL  = "Standard IPL code was installed in memory. Press F2 to save changes.")
se( MESG_UNINSTALL  = "Standard IPL Ñr installerad i minnet. Tryck F2 fîr att spara Ñndringen.")
sp( MESG_UNINSTALL  = "Standard IPL instalado en memoria. Presione F2 para guardar cambios.")
de( MESG_UNINSTALL  = "Standard IPL Code wurde geladen. F2 um ihn zu schreiben.")
ru( MESG_UNINSTALL  = "ë‚†≠§†‡‚®Î© IPL „·‚†≠Æ¢´•≠ ¢ Ø†¨Ô‚®. ç†¶¨®‚• F2 Á‚Æ°Î ß†Ø®·†‚Ï ®ß¨•≠•≠®Ô.")
fr( MESG_UNINSTALL  = "Code IPL Standard installÇ en mÇmoire. Presser F2 pour enregistrer.")
it( MESG_UNINSTALL  = "IPL Standard installato in memoria. Premere F2 per salvare.")
nl( MESG_UNINSTALL  = "Standaard IPL code geladen. Druk op F2 om de veranderingen op te slaan.")
cs( MESG_UNINSTALL  = "Standardn° IPL byl zaveden. Stisknut°m <F2> jej zap°Áete na disk.")

en( MESG_NOT_SAVED  = "MBR was modified. You have to SAVE or UNDO changes first.")
se( MESG_NOT_SAVED  = "MBR Ñr modifierad. Du mÜste SPARA eller UNDO fîrÑndringarna fîrst.")
sp( MESG_NOT_SAVED  = "MBR fue modificado. Debe guardar or deshacer cambios primero.")
de( MESG_NOT_SAVED  = "MBR wurde verÑndert. Zuerst Sichern oder énderungen zurÅcknehmen.")
ru( MESG_NOT_SAVED  = "MBR ®ß¨•≠®´·Ô. èÆ¶†´„©·‚† ·≠†Á†´† SAVE ®´® UNDO.")
fr( MESG_NOT_SAVED  = "Le MBR a ÇtÇ modifiÇ. Sauvegardez ou annulez d'abord la modification.")
it( MESG_NOT_SAVED  = "MBR modificato. Si deve ora SALVARE o ANNULLARE le modifiche.")
nl( MESG_NOT_SAVED  = "MBR is veranderd. U moet de veranderingen eerst opslaan of ongedaan maken.") 
cs( MESG_NOT_SAVED  = "MBR byl zmÿnÿn. Uloßte nebo odvolejte zmÿny.")

en( MESG_LOGICAL = "Logical drives:\n\n")
se( MESG_LOGICAL = "Logiska enheter:\n\n")
sp( MESG_LOGICAL = "Logical drives:\n\n")
de( MESG_LOGICAL = "Logische Platten:\n\n")
ru( MESG_LOGICAL = "ãÆ£®Á•·™®• §®·™®:\n\n")
fr( MESG_LOGICAL = "Disques Logique:\n\n")
it( MESG_LOGICAL = "Dischi logici:\n\n")
nl( MESG_LOGICAL = "Logische stations:\n\n")
cs( MESG_LOGICAL = "LogickÇ disky:\n\n")

en( MESG_DIFFERENT      = "Differences found !!!")
se( MESG_DIFFERENT      = "Skillnad funnen !!!")
sp( MESG_DIFFERENT      = "Diferencias encontradas !!!")
de( MESG_DIFFERENT      = "Unterschiede gefunden !!!")
ru( MESG_DIFFERENT      = "ç†©§•≠Î ‡†ß´®Á®Ô !!!")
fr( MESG_DIFFERENT      = "Il y a des diffÇrences !!!")
it( MESG_DIFFERENT      = "Trovate differenze !!!")
nl( MESG_DIFFERENT      = "Verschillen gevonden !!!")
cs( MESG_DIFFERENT      = "Nalezeny rozd°ly !!!")

en( MESG_NO_DIFFERENCES = "No differences found.")
se( MESG_NO_DIFFERENCES = "Ingen skillnad funnen.")
sp( MESG_NO_DIFFERENCES = "No diferencias encontradas.")
de( MESG_NO_DIFFERENCES = "Keine Unterschiede gefunden.")
ru( MESG_NO_DIFFERENCES = "ê†ß´®Á®© ≠• ≠†©§•≠Æ.")
fr( MESG_NO_DIFFERENCES = "Pas de diffÇrences.")
it( MESG_NO_DIFFERENCES = "Nessuna differenza trovata.")
nl( MESG_NO_DIFFERENCES = "Geen verschillen gevonden.")
cs( MESG_NO_DIFFERENCES = "Nenalezeny rozd°ly.")

en( HTML_DOC_EMPTY = "\nDocument is empty.\n")
se( HTML_DOC_EMPTY = "\nDokumentet Ñr tomt.\n")
sp( HTML_DOC_EMPTY = "\nDocumento esta vacio.\n")
de( HTML_DOC_EMPTY = "\nDas Dokument ist leer.\n")
ru( HTML_DOC_EMPTY = "\nÑÆ™„¨•≠‚ Ø„·‚Æ©.\n")
fr( HTML_DOC_EMPTY = "\nLe document est vide.\n")
it( HTML_DOC_EMPTY = "\nIl documento ä vuoto.\n")
nl( HTML_DOC_EMPTY = "\nDocument is leeg.\n")
cs( HTML_DOC_EMPTY = "\nDokument je pr†zdnÏ.\n")

en( HTML_NOT_FOUND = "Reference %s not found")
se( HTML_NOT_FOUND = "Referens %s icke funnen")
sp( HTML_NOT_FOUND = "Referencia %s no encontrada")
de( HTML_NOT_FOUND = "Referenzmarke %s nicht gefunden")
ru( HTML_NOT_FOUND = "ê†ß§•´† %s ¢ ØÆ¨ÆÈ® ≠•‚")
fr( HTML_NOT_FOUND = "RÇfÇrence %s introuvable")
it( HTML_NOT_FOUND = "Riferimento %s non trovato")
nl( HTML_NOT_FOUND = "Referentie %s niet gevonden")
cs( HTML_NOT_FOUND = "Odkaz %s nebyl nalezen")

en( HTML_ERROR_READ = "Error reading file %s")
se( HTML_ERROR_READ = "Fel vid lÑsning av fil %s")
sp( HTML_ERROR_READ = "Error leyendo archivo %s")
de( HTML_ERROR_READ = "Fehler beim Lesen der Datei %s")
ru( HTML_ERROR_READ = "éË®°™† Ø‡® Á‚•≠®® ®ß ‰†©´† %s")
fr( HTML_ERROR_READ = "Erreur de lecture du fichier %s")
it( HTML_ERROR_READ = "Errore leggendo il file %s")
nl( HTML_ERROR_READ = "Fout tijdens lezen van het bestand %s")
cs( HTML_ERROR_READ = "Chyba p˝i üten° souboru %s")

en( ERROR_NO_SETUP  = "Setup module is not available for this file system.")
se( ERROR_NO_SETUP  = "Konfigurations modul Ñr inte tillgÑnglig fîr detta fil system.")
sp( ERROR_NO_SETUP  = "Modulo de instalaci¢n no esta disponible para este archivo.")
de( ERROR_NO_SETUP  = "Kein Setup Modul fÅr dieses Dateisystem vorhanden.")
ru( ERROR_NO_SETUP  = "åÆ§„´Ï §´Ô ™Æ≠‰®£„‡†Ê®® Ì‚Æ© ‰†©´Æ¢Æ© ·®·‚•¨Î ØÆ™† ≠• ®¨••‚·Ô.")
fr( ERROR_NO_SETUP  = "Module de configuration non disponible pour ce systäme de fichiers.")
it( ERROR_NO_SETUP  = "Modulo di installazione non disponibile per questo file system.")
nl( ERROR_NO_SETUP  = "Setup module is niet beschikbaar voor dit bestandssysteem.")
cs( ERROR_NO_SETUP  = "Pro tento systÇm nen° dostupnÏ nastavovac° modul.")

en( ERROR_NO_FORMAT = "Format module is not available for this file system.")
se( ERROR_NO_FORMAT = "Format modul Ñr inte tillgÑnglig fîr detta fil system.")
sp( ERROR_NO_FORMAT = "Modulo de formateo no esta disponible para este archivo de sistema.")
de( ERROR_NO_FORMAT = "Kein Formatierungsmodul fÅr dieses Dateisystem vorhanden.")
ru( ERROR_NO_FORMAT = "åÆ§„´Ï §´Ô ‰Æ‡¨†‚®‡Æ¢†≠®Ô Ì‚Æ© ‰†©´Æ¢Æ© ·®·‚•¨Î ØÆ™† ≠• ®¨••‚·Ô.")
fr( ERROR_NO_FORMAT = "Module de formatage non disponible pour ce systäme de fichiers.")
it( ERROR_NO_FORMAT = "Modulo di formattazione non disponibile per questo file system.")
nl( ERROR_NO_FORMAT = "Formatteer module is niet beschikbaar voor dit bestandssysteem.")
cs( ERROR_NO_FORMAT = "Pro tento systÇm nen° dostupnÏ form†tovac° modul.")
 
en( PROMPT_FILE    = "Enter file name: ")
se( PROMPT_FILE    = "Skriv fil namn: ")
sp( PROMPT_FILE    = "Entar nombre del archivo: ")
de( PROMPT_FILE    = "Dateiname: ")
ru( PROMPT_FILE    = "Ç¢•§®‚• ®¨Ô ‰†©´†: ")
fr( PROMPT_FILE    = "Entrez le nom du fichier: ")
it( PROMPT_FILE    = "Inserire nome file: ")
nl( PROMPT_FILE    = "Bestandsnaam: ")
cs( PROMPT_FILE    = "Zadejte jmÇno souboru: ")

en( PROMPT_FSTYPE  = "Enter file system id: ")
se( PROMPT_FSTYPE  = "Skriv fil system id: ")
sp( PROMPT_FSTYPE  = "Entrar el id del archivo del systema: ")
de( PROMPT_FSTYPE  = "Dateisystem ID: ")
ru( PROMPT_FSTYPE  = "Ç¢•§®‚• Ë•·≠†§Ê†‚®‡®Á≠Î© ™Æ§ ·®·‚•¨Î: ")
fr( PROMPT_FSTYPE  = "Entrez l'id du systäme de fichiers: ")
it( PROMPT_FSTYPE  = "Inserire ID del file system: ")
nl( PROMPT_FSTYPE  = "Bestandssysteem identificatie: ")
cs( PROMPT_FSTYPE  = "Zadejte ü°slo souborovÇho systÇmu: ")

en( PROMPT_FORMAT  = "Optional format arguments: ")
se( PROMPT_FORMAT  = "Ytterligare formaterings argument: ")
sp( PROMPT_FORMAT  = "Argumentos de formato opcional: ")
de( PROMPT_FORMAT  = "Optionale Formatparameter: ")
ru( PROMPT_FORMAT  = "Ç¢•§®‚• ™´ÓÁ® §´Ô ‰Æ‡¨†‚†, •·´® •·‚Ï: ")
fr( PROMPT_FORMAT  = "Arguments de formatage optionnels: ")
it( PROMPT_FORMAT  = "Argomenti opzionali di formattazione: ")
nl( PROMPT_FORMAT  = "Optionele formatteer parameters: ")
cs( PROMPT_FORMAT  = "VolitelnÇ parametry form†tov†n°: ")


en( MESG_CLEANING   = "^Cleaning...                                        Press ESC to cancel")
se( MESG_CLEANING   = "^Rensar...                                    Tryck ESC fîr att avbryta")
sp( MESG_CLEANING   = "^Borrando...                                 Presione ESC para cancelar")
de( MESG_CLEANING   = "^RÑume auf...                                         Abbrechen mit ESC")
ru( MESG_CLEANING   = "^ó®·‚®¨...                                       ç†¶¨®‚• ESC §´Ô Æ‚¨•≠Î")
fr( MESG_CLEANING   = "^Nettoyage...                                Pressez Echap pour annuler")
it( MESG_CLEANING   = "^Cancellazione...                             Premere ESC per annullare")
nl( MESG_CLEANING   = "^Bezig met schoonmaken...                   Druk op ESC om af te breken")
cs( MESG_CLEANING   = "^VÏmaz...                                      Odvolat stisknut°m <ESC>")

en( MESG_VERIFYING  = "^Verifying...                                       Press ESC to cancel")
se( MESG_VERIFYING  = "^Verifierar...                                Tryck ESC fîr att avbryta")
sp( MESG_VERIFYING  = "^Verificando...                              Presione ESC para cancelar")
de( MESG_VERIFYING  = "^Verifiziere...                                       Abbrechen mit ESC")
ru( MESG_VERIFYING  = "^Ç•‡®‰®Ê®‡„Ó...                                  ç†¶¨®‚• ESC §´Ô Æ‚¨•≠Î")
fr( MESG_VERIFYING  = "^VÇrification...                             Pressez Echap pour annuler")
it( MESG_VERIFYING  = "^Verifica...                                  Premere ESC per annullare")
nl( MESG_VERIFYING  = "^Bezig met controleren...                   Druk op ESC om af te breken")
cs( MESG_VERIFYING  = "^Kontrola...                                   Odvolat stisknut°m <ESC>")

en( MESG_FORMATTING = "^Formatting...                                      Press ESC to cancel")
se( MESG_FORMATTING = "^Formatterar...                               Tryck ESC fîr att avbryta")
sp( MESG_FORMATTING = "^Formatiando...                              Presione ESC para cancelar")
de( MESG_FORMATTING = "^Formatiere...                                        Abbrechen mit ESC")
ru( MESG_FORMATTING = "^îÆ‡¨†‚®‡„Ó...                                   ç†¶¨®‚• ESC §´Ô Æ‚¨•≠Î")
fr( MESG_FORMATTING = "^Formatage...                                Pressez Echap pour annuler")
it( MESG_FORMATTING = "^Formattazione...                             Premere ESC per annullare")
nl( MESG_FORMATTING = "^Bezig met formatteren...                   Druk op ESC om af te breken")
cs( MESG_FORMATTING = "^Form†tov†n°...                                Odvolat stisknut°m <ESC>")

en( MESG_FORMAT_OK       = "Format completed.")
se( MESG_FORMAT_OK       = "Formattering slutfîrd.")
sp( MESG_FORMAT_OK       = "Formato completo.")
de( MESG_FORMAT_OK       = "Formatieren beendet.")
ru( MESG_FORMAT_OK       = "îÆ‡¨†‚ ß†¢•‡Ë•≠.")
fr( MESG_FORMAT_OK       = "Formatage terminÇ.")
it( MESG_FORMAT_OK       = "Formattazione completata.")
nl( MESG_FORMAT_OK       = "formatteren gereed.")
cs( MESG_FORMAT_OK       = "Form†tov†n° dokonüeno.")

en( WARN_FORMAT_CANCEL   = "Format canceled by user.")
se( WARN_FORMAT_CANCEL   = "Formattering avbruten av anvÑndare.")
sp( WARN_FORMAT_CANCEL   = "Formato cancelado por usuario.")
de( WARN_FORMAT_CANCEL   = "Formatierung abgebrochen.")
ru( WARN_FORMAT_CANCEL   = "îÆ‡¨†‚ Æ‚¨•≠Ò≠ ØÆ´ÏßÆ¢†‚•´•¨.")
fr( WARN_FORMAT_CANCEL   = "Formatage interrompu par l'utilisateur.")
it( WARN_FORMAT_CANCEL   = "Formattazione annullata dall'utente.")
nl( WARN_FORMAT_CANCEL   = "Formatteren is afgebroken.")
cs( WARN_FORMAT_CANCEL   = "Form†tov†n° p˝eruÁeno ußivatelem.")

en( ERROR_FORMAT_FAILED  = "Format failed.")
se( ERROR_FORMAT_FAILED  = "Formattering misslyckades.")
sp( ERROR_FORMAT_FAILED  = "Formato fallo.")
de( ERROR_FORMAT_FAILED  = "Formatierung fehlgeschlagen.")
ru( ERROR_FORMAT_FAILED  = "îÆ‡¨†‚®‡Æ¢†≠®• ≠• ·Æ·‚ÆÔ´Æ·Ï.")
fr( ERROR_FORMAT_FAILED  = "Formatage ratÇ.")
it( ERROR_FORMAT_FAILED  = "Formattazione fallita.")
nl( ERROR_FORMAT_FAILED  = "Formatteren mislukt.")
cs( ERROR_FORMAT_FAILED  = "Form†tov†n° ukonüeno chybou.")

en( MESG_VERIFY_OK       = "Verification completed. No bad sectors found.")
se( MESG_VERIFY_OK       = "Verifiering avslutad. Inga dÜliga sektorer funna.")
sp( MESG_VERIFY_OK       = "Verificaci¢n completa. No encontro sectores malos.")
de( MESG_VERIFY_OK       = "OberflÑchentest beendet. Keine Fehler gefunden.")
ru( MESG_VERIFY_OK       = "Ç•‡®‰®™†Ê®Ô ß†¢•‡Ë•≠†. ë°Æ©≠ÎÂ ·•™‚Æ‡Æ¢ ≠•‚. ç® Æ§≠Æ£Æ.")
fr( MESG_VERIFY_OK       = "VÇrification terminÇe. Aucun secteur dÇfectueux.")
it( MESG_VERIFY_OK       = "Verifica completata. Nessun settore difettoso.")
nl( MESG_VERIFY_OK       = "Controle voltooid. Geen slechte sectoren gevonden.")
cs( MESG_VERIFY_OK       = "Test povrchu dokonüen, vadnÇ sektory nebyly nalezeny.")

en( WARN_VERIFY_CANCEL   = "Verification canceled by user.")
se( WARN_VERIFY_CANCEL   = "Verifiering avbruten av anvÑndare.")
sp( WARN_VERIFY_CANCEL   = "Verificaci¢n cancelada por usuario.")
de( WARN_VERIFY_CANCEL   = "OberflÑchentest abgebrochen.")
ru( WARN_VERIFY_CANCEL   = "Ç•‡®‰®™†Ê®Ô Æ‚¨•≠•≠† ØÆ´ÏßÆ¢†‚•´•¨.")
fr( WARN_VERIFY_CANCEL   = "VÇrification interompue par l'utilisateur.")
it( WARN_VERIFY_CANCEL   = "Verifica annullata dall'utente.")
nl( WARN_VERIFY_CANCEL   = "Controle afgebroken.")
cs( WARN_VERIFY_CANCEL   = "Test povrchu p˝eruÁen ußivatelem.")

en( ERROR_VERIFY_FAILED  = "One or more bad sectors found on disk.")
se( ERROR_VERIFY_FAILED  = "En eller flera dÜliga sektorer funna pÜ disken.")
sp( ERROR_VERIFY_FAILED  = "Uno ¢ mas sectores malos encontrados en el disco.")
de( ERROR_VERIFY_FAILED  = "Ein oder mehrere fehlerhafte Sektoren gefunden.")
ru( ERROR_VERIFY_FAILED  = "ä†™ ¨®≠®¨„¨ Æ§®≠ ·•™‚Æ‡ ·°Æ©≠Î©.")
fr( ERROR_VERIFY_FAILED  = "Un ou plusieurs secteurs du disque sont dÇfectueux.")
it( ERROR_VERIFY_FAILED  = "Uno o pió settori difettosi trovati sul disco.")
nl( ERROR_VERIFY_FAILED  = "1 of meerdere slechte sectoren gevonden.")
cs( ERROR_VERIFY_FAILED  = "Nalezen alespoÂ jeden sektor s chybou.")
 

en( ERROR_FORMAT_GEN     = "General failure formatting the disk.")
se( ERROR_FORMAT_GEN     = "Generellt misslyckande att formattera disken.")
sp( ERROR_FORMAT_GEN     = "Error general formatiando el dico.")
de( ERROR_FORMAT_GEN     = "Schwerer Fehler beim Formatieren.")
ru( ERROR_FORMAT_GEN     = "General failure formatting the disk.")
fr( ERROR_FORMAT_GEN     = "Echec gÇnÇral du formatage du disque dur.")
it( ERROR_FORMAT_GEN     = "Errore generale di formattazione.")
nl( ERROR_FORMAT_GEN     = "Algemene storing tijdens het formatteren.")
cs( ERROR_FORMAT_GEN     = "V†ßn† chyba p˝i form†tov†n° disku.")

en( ERROR_FORMAT_WIN95   = "You must exit to Windows 95's DOS Prompt for destructive format.")
se( ERROR_FORMAT_WIN95   = "Du mÜste gÜ ut i DOS Prompt fîr en destruktiv formattering.")
sp( ERROR_FORMAT_WIN95   = "Ud. debe salir de Windows 95 DOS Prompt para destrucci¢n de formato.")
de( ERROR_FORMAT_WIN95   = "Nur im Windows 95 Eingabemodus kann vollstÑndig Formatiert werden.")
ru( ERROR_FORMAT_WIN95   = "ÇÎ §Æ´¶≠Î ¢Î©‚® ®ß Windows 95 ¢ DOS Prompt §´Ô ‚†™Æ£Æ ‚®Ø† ‰Æ‡¨†‚†.")
fr( ERROR_FORMAT_WIN95   = "Vous devez redÇmarrer Windows 95 en mode MS-DOS pour le formatage.")
it( ERROR_FORMAT_WIN95   = "Si deve riavviare il sistema in modalitÖ MS-DOS per formattazione distruttiva.")
nl( ERROR_FORMAT_WIN95   = "Formatteren kan alleen in Windows 95 - MS-DOS modus.")
cs( ERROR_FORMAT_WIN95   = "Pro destruktivn° form†tov†n° mus°te p˝ej°t do DOS reßimu Windows 95.")

en( ERROR_FORMAT_EMPTY   = "You cannot format or verify empty partition.")
se( ERROR_FORMAT_EMPTY   = "Du kan inte formattera eller verifiera tom partition.")
sp( ERROR_FORMAT_EMPTY   = "Ud. no puede formatear ¢ verificar partici¢n vacia.")
de( ERROR_FORMAT_EMPTY   = "Befehl fÅr leere Partitionen nicht ausfÅhrbar.")
ru( ERROR_FORMAT_EMPTY   = "è„·‚Î• ‡†ß§•´Î ≠• ‰Æ‡¨†‚®‡„Ó‚·Ô ® ≠† §Æ¨ ≠• §†Ó‚·Ô.")
fr( ERROR_FORMAT_EMPTY   = "Impossible de formater ou vÇrifier une partition vide.")
it( ERROR_FORMAT_EMPTY   = "Non si puï formattare o verificare una partizione vuota.")
nl( ERROR_FORMAT_EMPTY   = "Formatteren of controleren van lege partitie onmogelijk.")
cs( ERROR_FORMAT_EMPTY   = "NemÖßete form†tovat nebo kontrolovat pr†zdnou oblast.")

en( ERROR_FORMAT_FRACTION = "Cannot format partition with fractional sides on the ends.")
se( ERROR_FORMAT_FRACTION = "Kan inte formattera partition med splittrade sidor pÜ slutet.")
sp( ERROR_FORMAT_FRACTION = "No puede formatear partici¢n con lados fracionales en el final.")
de( ERROR_FORMAT_FRACTION = "Partition mit losen Enden kann nicht formatiert werden.")
ru( ERROR_FORMAT_FRACTION = "ê†ß§•´ ≠• §Æ´¶•≠ ®¨•‚Ï §‡Æ°≠ÎÂ ·‚Æ‡Æ≠ ≠† ™Æ≠Ê†Â.")
fr( ERROR_FORMAT_FRACTION = "Impossible de formater une partition dont la fin est fractionnÇe.")
it( ERROR_FORMAT_FRACTION = "Impossibile formattare partizioni con valori frazionari.")
nl( ERROR_FORMAT_FRACTION = "Partitie met gebroken einden kan niet geformatteerd worden.")
cs( ERROR_FORMAT_FRACTION = "Oblast s ne£plnÏm koncem nelze form†tovat.")
 

en( IPL_VIRUS   = "Check for MBR viruses ..")
se( IPL_VIRUS   = "Kolla MBR virusar ......")
sp( IPL_VIRUS   = "Verificar MBR virus ....")
de( IPL_VIRUS   = "Pr¸fe MBR auf Viren ....")
ru( IPL_VIRUS   = "è‡Æ¢•‡™† ≠† ¢®‡„·Î .....")
fr( IPL_VIRUS   = "VÇrifie les virus MBR ..")
it( IPL_VIRUS   = "Controllo virus MBR ....")
nl( IPL_VIRUS   = "Controleer op MBR virus.")
cs( IPL_VIRUS   = "Hled†m viry v MBR   ....")

en( IPL_DOTS    = "Number of running dots .")
se( IPL_DOTS    = "Prickig prompt lÑngd ...")
sp( IPL_DOTS    = "Puntos sucesivos .......")
de( IPL_DOTS    = "Zahl der Eingabepunkte .")
ru( IPL_DOTS    = "äÆ´®Á•·‚¢Æ ‚ÆÁ•Á•™ .....")
fr( IPL_DOTS    = "Nombre de points .......")
it( IPL_DOTS    = "Numero puntini .........")
nl( IPL_DOTS    = "Aantal punten...........")
cs( IPL_DOTS    = "Poüet bodÖ .............")

en( IPL_DEFAULT = "Default partition name .")
se( IPL_DEFAULT = "Standard part. namn ....")
sp( IPL_DEFAULT = "Partici¢n inicial ......")
de( IPL_DEFAULT = "Default Partitionsname .")
ru( IPL_DEFAULT = "Ç·•£§† Ø‡•§´†£†‚Ï ......")
fr( IPL_DEFAULT = "Partition par dÇfaut....")
it( IPL_DEFAULT = "Partizione iniziale ....")
nl( IPL_DEFAULT = "Standaard partitienaam..")
cs( IPL_DEFAULT = "Implicitn° jmÇno oblasti")

en( IPL_NOACTV  = "If no prt active boot ..")
se( IPL_NOACTV  = "Om inga prt aktiva boota")
sp( IPL_NOACTV  = "Si no prt boot activo ..")
de( IPL_NOACTV  = "Wenn kein Prt, starte ..")
ru( IPL_NOACTV  = "Å•ß ‚•™„È•£Æ £‡„ß®‚Ï ...")
fr( IPL_NOACTV  = "Pas de prt active, boot.")
it( IPL_NOACTV  = "Se non bootstrappa .....")
nl( IPL_NOACTV  = "Indien geen Prt. start..")
cs( IPL_NOACTV  = "Start nen°-li aktivn° ob")

en( MENU_ADV_OPT_TITLE    = "Menu title:")
se( MENU_ADV_OPT_TITLE    = "Meny titel:")
sp( MENU_ADV_OPT_TITLE    = "Titulo:"    )
de( MENU_ADV_OPT_TITLE    = "Menu Titel:")
ru( MENU_ADV_OPT_TITLE    = "á†£Æ´Æ¢Æ™:" )
fr( MENU_ADV_OPT_TITLE    = "Titre du menu:")
it( MENU_ADV_OPT_TITLE    = "Titolo:"    )
nl( MENU_ADV_OPT_TITLE    = "Menu titel:")
cs( MENU_ADV_OPT_TITLE    = "N†zev nab°dky:")

en( MENU_ADV_OPT_TIMEOUT  = "Boot menu timeout:" )
se( MENU_ADV_OPT_TIMEOUT  = "Boot meny avbrott:" )
sp( MENU_ADV_OPT_TIMEOUT  = "Finalizo boot menu:")
de( MENU_ADV_OPT_TIMEOUT  = "Boot Menu Timeout:" )
ru( MENU_ADV_OPT_TIMEOUT  = "Ç‡•¨Ô Æ¶®§†≠®Ô:"    )
fr( MENU_ADV_OPT_TIMEOUT  = "Limite de temps:"   )
it( MENU_ADV_OPT_TIMEOUT  = "Scadenza boot menu:")
nl( MENU_ADV_OPT_TIMEOUT  = "Boot menu time-out:")
cs( MENU_ADV_OPT_TIMEOUT  = "Timeout zav. nab.:" )

en( MENU_ADV_OPT_VIRCHECK = "Check for viruses:" )
se( MENU_ADV_OPT_VIRCHECK = "Kolla fîr virus:"   )
sp( MENU_ADV_OPT_VIRCHECK = "Buscar virus:"      )
de( MENU_ADV_OPT_VIRCHECK = "Auf Viren prÅfen:"  )
ru( MENU_ADV_OPT_VIRCHECK = "è‡Æ¢•‡™† ≠† ¢®‡„·Î:")
fr( MENU_ADV_OPT_VIRCHECK = "Antivirus MBR:"     )
it( MENU_ADV_OPT_VIRCHECK = "Controllo virus:"   )
nl( MENU_ADV_OPT_VIRCHECK = "Viruscontrole:"     )
cs( MENU_ADV_OPT_VIRCHECK = "Kontrolovat viry:"  )

en( MENU_ADV_OPT_CLEARSCR = "Clear screen:"      )
se( MENU_ADV_OPT_CLEARSCR = "Rensa skÑrmen:"     )
sp( MENU_ADV_OPT_CLEARSCR = "Limpiar pantalla:"  )
de( MENU_ADV_OPT_CLEARSCR = "Bildschirm lîschen:")
ru( MENU_ADV_OPT_CLEARSCR = "ó®·‚®‚Ï Ì™‡†≠:"     )
fr( MENU_ADV_OPT_CLEARSCR = "Effacer l'Çcran:"   )
it( MENU_ADV_OPT_CLEARSCR = "Cancella schermo:"  )
nl( MENU_ADV_OPT_CLEARSCR = "Maak scherm leeg:"  )
cs( MENU_ADV_OPT_CLEARSCR = "Smazat obrazovku:"  )

en( MENU_ADV_OPT_DEFAULT  = "Default boot menu:" )
se( MENU_ADV_OPT_DEFAULT  = "Standard boot meny:")
sp( MENU_ADV_OPT_DEFAULT  = "Inicial menu boot:" )
de( MENU_ADV_OPT_DEFAULT  = "Default Boot Menu:" )
ru( MENU_ADV_OPT_DEFAULT  = "ç†Á†´Ï≠Æ• ¨•≠Ó:"    )
fr( MENU_ADV_OPT_DEFAULT  = "Menu par dÇfaut:"   )
it( MENU_ADV_OPT_DEFAULT  = "Menu boot standard:")
nl( MENU_ADV_OPT_DEFAULT  = "Standaard menu:"    )
cs( MENU_ADV_OPT_DEFAULT  = "Impl. zav. nab°dka:")

en( MENU_ADV_OPT_PASSWORD = "Menu password:"     )
se( MENU_ADV_OPT_PASSWORD = "Menu password:"     )
sp( MENU_ADV_OPT_PASSWORD = "Contrasena menu:"   )
de( MENU_ADV_OPT_PASSWORD = "Menu password:"     )
ru( MENU_ADV_OPT_PASSWORD = "è†‡Æ´Ï:"            )
fr( MENU_ADV_OPT_PASSWORD = "Mot de passe:"      )
it( MENU_ADV_OPT_PASSWORD = "Menu password:"     )
nl( MENU_ADV_OPT_PASSWORD = "Menu wachtwoord:"   )
cs( MENU_ADV_OPT_PASSWORD = "Heslo nab°dky:"     )

en( MENU_ADV_OPT_IGN_UNUSED = "Ignore unused part:")
se( MENU_ADV_OPT_IGN_UNUSED = "Ignore unused part:")
sp( MENU_ADV_OPT_IGN_UNUSED = "Ignore unused part:")
de( MENU_ADV_OPT_IGN_UNUSED = "Ignore unused part:")
ru( MENU_ADV_OPT_IGN_UNUSED = "Ignore unused part:")
fr( MENU_ADV_OPT_IGN_UNUSED = "Ignore unused part:")
it( MENU_ADV_OPT_IGN_UNUSED = "Ignore unused part:")
nl( MENU_ADV_OPT_IGN_UNUSED = "Ignore unused part:")

en( NAME_OS_UNUSED = "Unused")
se( NAME_OS_UNUSED = "Unused")
sp( NAME_OS_UNUSED = "Sin uso")
de( NAME_OS_UNUSED = "Leer")
ru( NAME_OS_UNUSED = "ç• ®·Ø.")
fr( NAME_OS_UNUSED = "InutilisÇ")
it( NAME_OS_UNUSED = "Non usato")
nl( NAME_OS_UNUSED = "Ongebruikt")
cs( NAME_OS_UNUSED = "Nevyußito")

en( NAME_OS_ADV    = "Advanced Boot Manager")
se( NAME_OS_ADV    = "Advanced Boot Manager")
sp( NAME_OS_ADV    = "Boot Manager Avansado")
de( NAME_OS_ADV    = "Advanced Boot Manager")
ru( NAME_OS_ADV    = "Advanced Boot Manager")
fr( NAME_OS_ADV    = "Boot Manager AvancÇ"  )
it( NAME_OS_ADV    = "Boot Manager Avanzato")
nl( NAME_OS_ADV    = "Advanced Boot Manager")
cs( NAME_OS_ADV    = "RozÁ°˝enÏ Boot Manager")

en( NAME_OS_HIDDEN = "Hidden (0xFF)")
se( NAME_OS_HIDDEN = "Gîmd (0xFF)")
sp( NAME_OS_HIDDEN = "Escondido (0xFF)")
de( NAME_OS_HIDDEN = "Versteckt (0xFF)")
ru( NAME_OS_HIDDEN = "ëØ‡Ô‚†≠ (0xFF)")
fr( NAME_OS_HIDDEN = "CachÇ (0xFF)")
it( NAME_OS_HIDDEN = "Nascosto (0xFF)")
nl( NAME_OS_HIDDEN = "Verborgen (0xFF)")
cs( NAME_OS_HIDDEN = "Skryto (0xFF)")

en( NAME_OS_UNKN   = "Unknown (0x%02X)         ")
se( NAME_OS_UNKN   = "OkÑnd (0x%02X)           ")
sp( NAME_OS_UNKN   = "No conocido (0x%02X)     ")
de( NAME_OS_UNKN   = "Unbekannt (0x%02X)       ")
ru( NAME_OS_UNKN   = "ç•®ß¢•·‚≠Î© (0x%02X)     ")
fr( NAME_OS_UNKN   = "Inconnu (0x%02X)         ")
it( NAME_OS_UNKN   = "Sconosciuto (0x%02X)     ")
nl( NAME_OS_UNKN   = "Onbekend (0x%02X)        ")
cs( NAME_OS_UNKN   = "Nezn†mo (0x%02X)         ")


en( MESG_NON_SYSTEM = "\r\nSystem is not installed."
                      "\r\nHit a key to reboot...") 
se( MESG_NON_SYSTEM = "\r\nSystem Ñr inte installerat."
                      "\r\nTryck en tangent fîr reboot...")
sp( MESG_NON_SYSTEM = "\r\nSistema no esta instalado."
                      "\r\nPresione una tecla para reiniciar...")
de( MESG_NON_SYSTEM = "\r\nKein System installiert."
                      "\r\nTaste um das System neu zu starten...") 
ru( MESG_NON_SYSTEM = "\r\në®·‚•¨≠Î• ‰†©´Î Æ‚·„‚·‚¢„Ó‚."
                      "\r\nç†¶¨®‚• ´Ó°„Ó ™´†¢®Ë„ ...")
fr( MESG_NON_SYSTEM = "\r\nPas de systäme installÇ."
                      "\r\nPressez une touche pour redÇmarrer...")
it( MESG_NON_SYSTEM = "\r\nSistema non installato."
                      "\r\nPremere un tasto per riavviare...")
nl( MESG_NON_SYSTEM = "\r\nSysteem niet geinstalleerd."
                      "\r\nDruk een toets om te herstarten...")
cs( MESG_NON_SYSTEM = "\r\nSystÇm nen° instalov†n."
                      "\r\nPro restartov†n° stisknÿte kl†vesu...") 


en( MESG_EXT_NONBOOT ="\r\nExtended partition is not bootable."
                      "\r\nHit a key to reboot...")
se( MESG_EXT_NONBOOT ="\r\nUtîkade partitionrn Ñr inte bootbar."
                      "\r\nTryck en tangent fîr att boota om...")
sp( MESG_EXT_NONBOOT ="\r\nPartici¢n extendida no es bootable."
                      "\r\nPresionar una tecla para reiniciar...")
de( MESG_EXT_NONBOOT ="\r\nExtended Partition ist nicht startbar."
                      "\r\nTaste um das System neu zu starten...") 
ru( MESG_EXT_NONBOOT ="\r\nê†·Ë®‡•≠≠Î© ‡†ß§•´ ≠• ß†£‡„ßÆÁ≠Î©."
                      "\r\nç†¶¨®‚• ´Ó°„Ó ™´†¢®Ë„ ...")
fr( MESG_EXT_NONBOOT ="\r\nPartition Etendue non bootable."
                      "\r\nPressez une touche pour redÇmarrer...")
it( MESG_EXT_NONBOOT ="\r\nPartizione estesa non avviabile."
                      "\r\nPremere un tasto per riavviare...")
nl( MESG_EXT_NONBOOT ="\r\nUitgebreide partitie niet startbaar."
                      "\r\nDruk een toets om te herstarten.")
cs( MESG_EXT_NONBOOT ="\r\nZ rozÁ°˝enÇ oblasti nelze zavÇst systÇm."
                      "\r\nPro restartov†n° stisknÿte kl†vesu...") 

en( HELP_FILE_NAME = "part.htm")
se( HELP_FILE_NAME = "part.htm")
sp( HELP_FILE_NAME = "part.htm")
de( HELP_FILE_NAME = "part.htm")
ru( HELP_FILE_NAME = "part.htm")
fr( HELP_FILE_NAME = "part.htm")
it( HELP_FILE_NAME = "part.htm")
nl( HELP_FILE_NAME = "part.htm")
cs( HELP_FILE_NAME = "part.htm")

en( HELP_CMD_LINE = 

"Ranish Partition Manager    Version "VERSION"\n\n"

"Usage: part [-q] [-d disk] [-command ...]\n\n"

" part          - Interactive mode\n"
" part -i       - Print IDE disk info\n"
" part -p       - Print partition table\n"
" part -p -r    - Print info recursively\n"
" part -a n     - Activate n-th partition\n"
" part -h n     - Hide/unhide n-th partition\n"
" part -v n     - Verify n-th partition\n"
" part -f n ... - Format n-th partition\n"
" part -s file  - Save ADV data to file if ADV is installed, else MBR\n"
" part -l file  - Load ADV data from file if ADV is installed, else MBR\n"
" part -c file  - Compare ADV data with file if ADV is installed, else MBR\n"
" part -Reboot  - Reboot computer\n\n"

"Homepage: "WEBSITE_URL"\n")
se( HELP_CMD_LINE      =  

"Ranish Partition Manager    Version "VERSION"\n\n"

"AnvÑndning: part [-q] [-d disk] [-kommand ...]\n\n"

" part          - Interaktivt lÑge\n"
" part -i       - Skriv ut IDE disk info\n"
" part -p       - Skriv ut partition tabell\n"
" part -p -r    - Skriv ut info rekursivt\n"
" part -a n     - Aktivera n-th partition\n"
" part -h n     - Hide/unhide n-th partition\n"
" part -v n     - Verifiera n-th partition\n"
" part -f n ... - Formattera n-th partition\n"
" part -s fil   - Spara MBR till en fil\n"
" part -l fil   - Ladda MBR frÜn en fil\n"
" part -c file  - JÑmfîr MBR med fil\n"
" part -Reboot  - Boota om datorn\n\n"

"Produkt hem: "WEBSITE_URL"\n")
sp( HELP_CMD_LINE = 

"Ranish Partition Manager    Versi¢n "VERSION"\n\n"

"Usage: part [-q] [-d disk] [-command ...]\n\n"

" part          - Modo Interactivo \n"
" part -i       - Imprimir disco info IDE\n"
" part -p       - Imprimir tabla de partici¢n\n"
" part -p -r    - Imprimir info recursivamente\n"
" part -a n     - Activar n-ava partici¢n\n"
" part -h n     - Hide/unhide n-th partition\n"
" part -v n     - Verificar n-ava partici¢n\n"
" part -f n ... - Formatear n-ava partici¢n\n"
" part -s file  - Guardar MBR en un archivo\n"
" part -l file  - Cargar MBR de un archivo\n"
" part -c file  - Compare MBR con archivo\n"
" part -Reboot  - Reboot computadora\n\n"

"Homepage: "WEBSITE_URL"\n")
de( HELP_CMD_LINE = 

"Ranish Partition Manager    Version "VERSION"\n\n"

"Aufruf: part [-q] [-d Platte] [-Kommando ...]\n\n"

" part          - Interaktiver Modus\n"
" part -i       - IDE Infos anzeigen\n"
" part -p       - Partitionstabelle anzeigen\n"
" part -p -r    - Alle Partitionstabellen anzeigen\n"
" part -a n     - Mache n-te Partition startbar\n"
" part -h n     - Hide/unhide n-th partition\n"
" part -v n     - öberprÅfe n-te Partition\n"
" part -f n ... - Formatiere n-te Partition\n"
" part -s Datei - Speichere MBR in eine Datei\n"
" part -l Datei - Lade MBR aus einer Datei\n"
" part -c Datei - Vergleiche MBR mit Datei\n"
" part -Reboot  - Rechner neu starten\n\n"

"Bezugsquelle: "WEBSITE_URL"\n")
ru( HELP_CMD_LINE = 

"Ranish Partition Manager    Version "VERSION"\n\n"

"Usage: part [-q] [-d disk] [-command ...]\n\n"

" part          - à≠‚•‡†™‚®¢≠Î© ‡•¶®¨"
" part -i       - ç†Ø•Á†‚†‚Ï IDE disk info\n"
" part -p       - ç†Ø•Á†‚†‚Ï ‚†°´®Ê„ ‡†ß§•´Æ¢\n"
" part -p -r    - ç†Ø•Á†‚†‚Ï ¢·Ò, Á‚Æ ¨Æ¶≠Æ\n"
" part -a n     - ë§•´†‚Ï ‡†ß§•´ n ‚•™„È®¨ (†™‚®¢≠Î¨)\n"
" part -a n     - Ä™‚®¢®ß®‡Æ¢†‚Ï n-‚Î© ‡†ß§•´\n"
" part -h n     - Hide/unhide n-th partition\n"
" part -v n     - Ç•‡®‰®Ê®‡Æ¢†‚Ï n-‚Î© ‡†ß§•´\n"
" part -f n ... - é‚‰Æ‡¨†‚®‡Æ¢†‚Ï n-‚Î© ‡†ß§•´\n"
" part -s file  - ëÆÂ‡†≠®‚Ï MBR ¢ ‰†©´\n"
" part -l file  - á†£‡„ß®‚Ï MBR ®ß ‰†©´†\n"
" part -c file  - ë‡†¢≠®‚Ï MBR · ‰†©´Æ¨\n"
" part -Reboot  - è•‡•£‡„ß®‚Ï ™Æ¨ØÏÓ‚•‡\n\n"

"Homepage: "WEBSITE_URL"\n")
fr( HELP_CMD_LINE = 

"Ranish Partition Manager    Version "VERSION"\n\n"

"Utilisation: part [-q] [-d disque] [-commande ...]\n\n"

" part          - Mode interactif\n"
" part -i       - Affiche les informations du disque IDE\n"
" part -p       - Affiche la table de partition\n"
" part -p -r    - Affiche les informations de maniäre rÇcursive\n"
" part -a n     - Active la partition n¯ n\n"
" part -h n     - Hide/unhide n-th partition\n"
" part -v n     - Verifie la partition n¯ n\n"
" part -f n ... - Formate la partition n¯ n\n"
" part -s file  - Sauve le MBR dans un fichier\n"
" part -l file  - Charge le MBR Ö partir d'un fichier\n"
" part -c file  - Compare le MBR avec un fichier\n"
" part -Reboot  - RedÇmarre l'ordinateur\n\n"

"Site du produit: "WEBSITE_URL"\n")
it( HELP_CMD_LINE = 

"Ranish Partition Manager    Version "VERSION"\n\n"

"Usage: part [-q] [-d disk] [-command ...]\n\n"

" part          - Modo Interattivo \n"
" part -i       - Stampa informazioni disco IDE\n"
" part -p       - Stampa tabella delle partizioni\n"
" part -p -r    - Stampa continua informazioni\n"
" part -a n     - Attiva n-esima partizione\n"
" part -h n     - Hide/unhide n-th partition\n"
" part -v n     - Verifica n-esima partizione\n"
" part -f n ... - Formatta n-esima partizione\n"
" part -s file  - Salva MBR su file\n"
" part -l file  - Carica MBR da file\n"
" part -c file  - Confronta MBR con file\n"
" part -Reboot  - Riavvia computer\n\n"

"Homepage: "WEBSITE_URL"\n")
nl( HELP_CMD_LINE = 

"Ranish Partition Manager    Versie "VERSION"\n\n"

"Gebruik: part [-q] [-d disk] [-command ...]\n\n"

" part          - Interactieve modus\n"
" part -i       - Toon informatie over de IDE-schijf\n"
" part -p       - Toon partitie informatie\n"
" part -p -r    - Toon alle partitie tabellen\n"
" part -a n     - Activeer de n-ste partitie\n"
" part -h n     - Hide/unhide n-th partition\n"
" part -v n     - Controleer de n-ste partitie\n"
" part -f n ... - Formatteer de n-ste partitie\n"
" part -s file  - Sla de MBR op naar een bestand\n"
" part -l file  - Laad de MBR vanuit een bestand\n"
" part -c file  - Vergelijk de MBR met een bestand\n"
" part -Reboot  - Herstart de computer\n\n"

"Homepage: "WEBSITE_URL"\n")
cs( HELP_CMD_LINE = 

"Ranish Partition Manager    Verze "VERSION"\n\n"

"SpuÁtÿn°: part [-q] [-d disk] [-p˝°kaz ...]\n\n"

" part           - Interaktivn° reßim\n"
" part -i        - Zobrazit informace o IDE\n"
" part -p        - Zobrazit tabulku oblast°\n"
" part -p -r     - Zobrazit vÁechny oblasti\n"
" part -a n      - Uüinit n-tou oblast aktivn°\n"
" part -h n      - Hide/unhide n-th partition\n"
" part -v n      - Kontrolovat n-tou oblast\n"
" part -f n ...  - Form†tovat n-tou oblast\n"
" part -s soubor - Uschovat MBR do souboru\n"
" part -l soubor - Obnovit MBR ze souboru\n"
" part -c soubor - Porovnat MBR se souborem\n"
" part -Reboot   - Restartovat poü°taü\n\n"

"Dom†c° adresa: "WEBSITE_URL"\n")

 i=0;
 os_desc[i].name = NAME_OS_UNUSED;
 while( os_desc[i].os_id!=OS_HIDDEN ) i++;
 os_desc[i].name = NAME_OS_HIDDEN;
 while( os_desc[i].os_id!=OS_ADV ) i++;
 os_desc[i].name = NAME_OS_ADV;
 while( os_desc[i].os_id!=OS_UNKN ) i++;
 os_desc[i].name = NAME_OS_UNKN;

}/* set_messages */
