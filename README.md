# MusicBox
 MusicBox is a music instrument simulator using Arduino Uno and Altera FPGA.

## Requirements
 * Linux Distro
 * SDL2 & SDL2_mixer C libraries
 * Arduino (with firmware.io code)
 * DE2I-150 Altera (with driver/altera_driver.c PCI driver)

## Compiling & runing
 enter in the project folder and run (you will need super user permission):
 ```bash
 cd driver
 make
 ./init_driver.sh
 cd ..
 make
 sudo ./app
 ```
## Group
 the creators of this project are the following Computer Engineering students:
 * [Matheus Ferreira](https://github.com/PunishedBois) (email: mfbs@cin.ufpe.br)
 * [Gabriela Leal](https://github.com/gabrielaleal) (email: gmfl@cin.ufpe.br)
 * [Beatiz Alves](https://github.com/biaalves) (email: babs@cin.ufpe.br)
 * [Marcos Monteiro](https://github.com/marcosmmb) (email: mmmb@cin.ufpe.br)
