set yy=%date:~0,4%
set mm=%date:~5,2%
set dd=%date:~8,2%
set hh=%time:~0,2%
set mn=%time:~3,2%
set ss=%time:~6,2%
Copy .\bootloader_build\bootloader.axf/b .\output\BOOTLOEAD_EASYCORE.axf
Copy .\bootloader_build\bootloader.hex/b .\output\BOOTLOEAD_EASYCORE.hex
Copy .\bootloader_build\bootloader.axf/b .\output\BOOTLOEAD_EASYCORE_%yy%%mm%%dd%.axf
Copy .\bootloader_build\bootloader.hex/b .\output\BOOTLOEAD_EASYCORE_%yy%%mm%%dd%.hex
Copy .\output\EasyCORE.bin/b .\output\EasyCORE_%yy%%mm%%dd%.bin