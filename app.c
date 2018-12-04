#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <time.h>
#include <termios.h>
#include <omp.h>
#include <SDL2/SDL_mixer.h>

#define NotesNUM    5
#define DISPLAY_L   1
#define DISPLAY_R   2
#define SWITCHES    3
#define PUSHBUTTON  4
#define GREENLEDS   5
#define REDLEDS     6

const uint32_t empty = 0x0, HEX_P = 0xFFFFFF0C, HEX_CLEAN = 0xFFFFFFFF, HEX_B = 0xFFFFFF00; // Si
const uint32_t HEX_C = 0xFFFFFF46, HEX_D = 0xFFFFFF40, HEX_E = 0xFFFFFF06; // Dó Ré Mi
const uint32_t HEX_F = 0xFFFFFF0E, HEX_G = 0xFFFFFF42, HEX_A = 0xFFFFFF08; //Fá Sol Lá
const uint32_t HEX_1 = 0xFFFFFF79, HEX_2 = 0xFFFFFF24, HEX_3 = 0xFFFFFF30, HEX_4 = 0xFFFFFF19;
const uint32_t HEX_A2= 0xFFFF0824, HEX_A3= 0xFFFF0830, HEX_C4= 0xFFFF4619, HEX_E2= 0xFFFF0624, HEX_E3= 0xFFFF0619;

/* Functions */
void LoadGuitar(Mix_Chunk **Notes);
void LoadDrums(Mix_Chunk **Notes);
void LoadBass(Mix_Chunk **Notes);
void FreeAudio(Mix_Chunk **Notes);
void SNA(Mix_Chunk **Notes);
void LIB(Mix_Chunk **Notes);
void LoadDrumsFPGA(Mix_Chunk **Notes);
void FreeAudioFPGA(Mix_Chunk **Notes);

void delay(int num_of_mili);
void red_led_on(int fpga, int n);
void green_led_on(int fpga, int n);
void led_animation(int fpga, int x, void (*FuncPtr)(int,int));


int main() {

    /* Initialzing SDL Mixer, frequency, Channels & Chunks */
    Mix_Init(MIX_INIT_MID);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    /* General Purpose Variables */
    char red_something = 0, animation = 0, animation_2 = 0, instrument = 2;
    Mix_Chunk *Notes[NotesNUM];
    Mix_Chunk *FPGAdrums[4];
    // loading audios chuncks into the memory
    LoadDrumsFPGA(FPGAdrums);
    LoadBass(Notes);
    
	/* Variables for Serial Device */
	int fd, i = 0;
    struct termios config;
    char buffer[5];
    unsigned char aux;
	const char *arduino = "/dev/ttyACM0";

    /* Variables for Altera FPGA */
    int fpga;
    const char *altera = "/dev/de2i150_altera";
    // read variables
    uint32_t  pbuttons_rd = 0, switches_rd = 0;

    /* Opening & Setting Up FPGA */
    printf("Opening FPGA...\n");
    fpga = open(altera, O_RDWR);
    if(fpga == -1)
        printf("Failed to Open Device: %s\n", altera);
    //first right is buggy so we write something to skip it
    printf("Cleaning FPGA memory...\n");
    write(fpga, &empty, DISPLAY_L);
    write(fpga, &HEX_3, DISPLAY_L);
    write(fpga, &empty, GREENLEDS);
    write(fpga, &HEX_CLEAN, DISPLAY_R);
    write(fpga, &empty, REDLEDS);

	/* Opening Serial Device, Flags & Checking for Errors.

	   O_RDWR   = Read and Write
       O_NOCTTY = The port never becomes the controlling terminal of the process 
       O_NDELAY = Use non-blocking I/O */

    printf("Opening Arduino...\n");
	fd = open(arduino, O_RDWR | O_NOCTTY | O_NDELAY);
	if(fd == -1)
		printf("Failed to open Device: %s\n", arduino);
    if(!isatty(fd))
        printf("%s is not a Serial Device\n", arduino);

    /* Configuration of a Serial Device by tcgetattr() & tcserattr(). 
       Structure of a Serial Config variable:

       struct termios {
          tcflag_t c_iflag;     input specific flags (bitmask)
          tcflag_t c_oflag;     output specific flags (bitmask)
          tcflag_t c_cflag;     control flags (bitmask)
          tcflag_t c_lflag;     local flags (bitmask)
          cc_t     c_cc[NCCS];  special characters
       };      

       More information about Serial Flags in: wikibooks.org/wiki/Serial_Programming/termios */

    if(tcgetattr(fd, &config) < 0)
        printf("Error while getting Device Configuration\n");

    config.c_iflag      &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
    config.c_oflag      = 0;
    config.c_lflag      &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
    config.c_cflag      &= ~(CSIZE | PARENB);
    config.c_cflag      |= CS8;
    config.c_cc[VMIN]   = 1;     // One input byte is enough to return from read()
    config.c_cc[VTIME]  = 0;

    if(cfsetispeed(&config, B9600) < 0 || cfsetospeed(&config, B9600) < 0)
        printf("Error while setting Communication Speed\n");
    if(tcsetattr(fd, TCSAFLUSH, &config) < 0)
        printf("Error while updating Device Configuration\n");

    printf("Done. Ready to go!\n");
    // Threads main loop
    #pragma omp parallel sections num_threads(3)
    {
        // FPGA Thread
        #pragma omp section
        {
            while(switches_rd != 1) {
                if(read(fpga, &switches_rd, SWITCHES) > 0){
                    red_something = 1;
                } else {
                    red_something = 0;
                }
                if(read(fpga, &pbuttons_rd, PUSHBUTTON)){
                    if(pbuttons_rd == 14){
                        Mix_PlayChannel(2, FPGAdrums[3], 0);
                        animation_2 = 1;
                    }
                    if(pbuttons_rd == 13){
                        Mix_PlayChannel(2, FPGAdrums[2], 0);
                        animation_2 = 1;
                    }
                    if(pbuttons_rd == 11){
                        Mix_PlayChannel(2, FPGAdrums[1], 0);
                        animation_2 = 1;
                    }
                    if(pbuttons_rd == 7){
                        Mix_PlayChannel(2, FPGAdrums[0], 0);
                        animation_2 = 1;
                    }
                }
                if(red_something == 1 && switches_rd == 2){
                    printf("Loading Guitar...\n");
                    FreeAudio(Notes);
                    LoadGuitar(Notes);
                    printf("Done loading\n");
                    write(fpga, &HEX_1, DISPLAY_L);
                    instrument = 1;
                }
                if(red_something == 1 && switches_rd == 4){
                    printf("Loading Drums...\n");
                    FreeAudio(Notes);
                    LoadDrums(Notes);
                    printf("Done Loading\n");
                    write(fpga, &HEX_2, DISPLAY_L);
                    write(fpga, &HEX_P, DISPLAY_R);
                    instrument = 2;
                }
                if(red_something == 1 && switches_rd == 8){
                    printf("Loading Bass...\n");
                    FreeAudio(Notes);
                    LoadBass(Notes);
                    printf("Done Loading\n");
                    write(fpga, &HEX_3, DISPLAY_L);
                    instrument = 3;
                }
                if(red_something == 1 && switches_rd == 16){
                    printf("Loading Bass 2...\n");
                    FreeAudio(Notes);
                    SNA(Notes);
                    printf("Done Loading\n");
                    write(fpga, &HEX_4, DISPLAY_L);
                    instrument = 4;
                }
                if(red_something == 1 && switches_rd == 32){
                    printf("Loading Guitar...\n");
                    FreeAudio(Notes);
                    LIB(Notes);
                    printf("Done loading\n");
                    write(fpga, &HEX_1, DISPLAY_L);
                    instrument = 5;
                }
            }
        }
        // Animation Thread
        #pragma omp section
        {
            while(switches_rd != 1) {
                if(animation == 1) {
                    if(buffer[0] == '1'){
                        led_animation(fpga, 9, &red_led_on);
                    }
                    if(buffer[1] == '1'){
                        led_animation(fpga, 12, &red_led_on);
                    }
                    if(buffer[2] == '1'){
                        led_animation(fpga, 15, &red_led_on);
                    }
                    if(buffer[3] == '1'){
                        led_animation(fpga, 8, &red_led_on);
                    }
                    if(buffer[4] == '1'){
                        led_animation(fpga, 10, &red_led_on);
                    }
                    animation = 0;
                }
                if(animation_2 == 1){
                    if(pbuttons_rd == 14){
                        //led_animation(fpga, 5, &green_led_on);
                    }
                    if(pbuttons_rd == 13){
                        //led_animation(fpga, 7, &green_led_on);
                    }
                    if(pbuttons_rd == 11){
                        //led_animation(fpga, 3, &green_led_on);
                    }
                    if(pbuttons_rd == 7){
                        //led_animation(fpga, 4, &green_led_on);
                    }
                    animation_2 = 0;
                }
            }
        }
        // Arduino Thread
        #pragma omp section
        {
            while(switches_rd != 1) {
                if(read(fd, &aux, 1) > 0)
                    buffer[i++] = aux;
                if(i == 5) {
                    i = 0;
                    animation = 1;
                    if(buffer[0] == '1'){
                        Mix_PlayChannel(1, Notes[0], 0);
                        if(instrument == 1){
                            write(fpga, &HEX_C, DISPLAY_R);
                        } else if(instrument == 3){
                            write(fpga, &HEX_A2, DISPLAY_R);
                        } else if(instrument == 4){
                            write(fpga, &HEX_E, DISPLAY_R);
                        } else if(instrument == 5){
                            write(fpga, &HEX_C, DISPLAY_R);
                        }
                    }
                    if(buffer[1] == '1'){
                        Mix_PlayChannel(1, Notes[1], 0);
                        if(instrument == 1){
                            write(fpga, &HEX_D, DISPLAY_R);
                        } else if(instrument == 3){
                            write(fpga, &HEX_A3, DISPLAY_R);
                        } else if(instrument == 4){
                            write(fpga, &HEX_G, DISPLAY_R);
                        } else if(instrument == 5){
                            write(fpga, &HEX_G, DISPLAY_R);
                        }
                    }
                    if(buffer[2] == '1'){
                        Mix_PlayChannel(1, Notes[2], 0);
                        if(instrument == 1){
                            write(fpga, &HEX_E, DISPLAY_R);
                        } else if(instrument == 3){
                            write(fpga, &HEX_C4, DISPLAY_R);
                        } else if(instrument == 4){
                            write(fpga, &HEX_D, DISPLAY_R);
                        } else if(instrument == 5){
                            write(fpga, &HEX_A, DISPLAY_R);
                        }
                    }
                    if(buffer[3] == '1'){
                        Mix_PlayChannel(1, Notes[3], 0);
                        if(instrument == 1){
                            write(fpga, &HEX_F, DISPLAY_R);
                        } else if(instrument == 3){
                            write(fpga, &HEX_E2, DISPLAY_R);
                        } else if(instrument == 4){
                            write(fpga, &HEX_C, DISPLAY_R);             
                        } else if(instrument == 5){
                            write(fpga, &HEX_F, DISPLAY_R);
                        }
                    }
                    if(buffer[4] == '1'){
                        Mix_PlayChannel(1, Notes[4], 0);
                        if(instrument == 1){
                            write(fpga, &HEX_G, DISPLAY_R);
                        } else if(instrument == 3){
                            write(fpga, &HEX_E3, DISPLAY_R);
                        } else if(instrument == 4){
                            write(fpga, &HEX_B, DISPLAY_R);
                        } else if(instrument == 5){
                            write(fpga, &HEX_E, DISPLAY_R);   
                        }
                    }
                }
            }
        }
    }

    printf("Closing all threads...\n");
    /* Release Resources */
    printf("Cleaning memory...\n");
    FreeAudio(Notes);
    FreeAudioFPGA(FPGAdrums);
    Mix_CloseAudio();
    Mix_Quit();
    close(fpga);
    close(fd);
    printf("Closing main program...\n");
	return 0;
}

/* Functions Implementations */

void LoadGuitar(Mix_Chunk **Notes) {
    Notes[0] = Mix_LoadWAV("Samples/C.aif");
    Notes[1] = Mix_LoadWAV("Samples/D.aif");
    Notes[2] = Mix_LoadWAV("Samples/E.aif");
    Notes[3] = Mix_LoadWAV("Samples/F.aif");
    Notes[4] = Mix_LoadWAV("Samples/G.aif");
}

void LIB(Mix_Chunk **Notes){
    Notes[0] = Mix_LoadWAV("Samples/C.aif");
    Notes[1] = Mix_LoadWAV("Samples/G.aif");
    Notes[2] = Mix_LoadWAV("Samples/Am.aif");
    Notes[3] = Mix_LoadWAV("Samples/F.aif");
    Notes[4] = Mix_LoadWAV("Samples/Em.aif");
}

void LoadDrums(Mix_Chunk **Notes) {
    Notes[0] = Mix_LoadWAV("Samples/drum1.aif");
    Notes[1] = Mix_LoadWAV("Samples/drum2.aif");
    Notes[2] = Mix_LoadWAV("Samples/drum3.aif");
    Notes[3] = Mix_LoadWAV("Samples/drum4.aif");
    Notes[4] = Mix_LoadWAV("Samples/drum1.aif");
}

void LoadBass(Mix_Chunk **Notes) {
    Notes[0] = Mix_LoadWAV("Samples/upright_bass-a2.aif");
    Notes[1] = Mix_LoadWAV("Samples/upright_bass-a3.aif");
    Notes[2] = Mix_LoadWAV("Samples/upright_bass-c4.aif");
    Notes[3] = Mix_LoadWAV("Samples/upright_bass-e2.aif");
    Notes[4] = Mix_LoadWAV("Samples/upright_bass-e3.aif");
}

void SNA(Mix_Chunk **Notes){
    Notes[0] = Mix_LoadWAV("Samples/bass-e.wav");
    Notes[1] = Mix_LoadWAV("Samples/bass-g.wav");
    Notes[2] = Mix_LoadWAV("Samples/bass-d.wav");
    Notes[3] = Mix_LoadWAV("Samples/bass-c.wav");
    Notes[4] = Mix_LoadWAV("Samples/bass-b.wav");
}

void LoadDrumsFPGA(Mix_Chunk **Notes) {
    Notes[0] = Mix_LoadWAV("Samples/clap.aiff");
    Notes[1] = Mix_LoadWAV("Samples/hat.aiff");
    Notes[2] = Mix_LoadWAV("Samples/kick.aiff");
    Notes[3] = Mix_LoadWAV("Samples/snare.aiff");
}

void FreeAudio(Mix_Chunk **Notes) {
    int i;
    for(i = 0; i < NotesNUM; i++)
        Mix_FreeChunk(Notes[i]);
}
void FreeAudioFPGA(Mix_Chunk **Notes) {
    int i;
    for(i = 0; i < 4; i++)
        Mix_FreeChunk(Notes[i]);
}
void green_led_on(int fpga, int n){
    n = 8 - n;
    int a = 0xFF;
    a = a << n;
    write(fpga, &a, GREENLEDS);
}

void red_led_on(int fpga, int n){
    n = 18 - n;
    int a = 262143;
    a = a << n;
    write(fpga, &a, REDLEDS);
}

void delay(int num_of_mili) {
    int milli_sec = 1000*num_of_mili;
    clock_t start_time = clock();
    while(clock() < start_time + milli_sec);
}

void led_animation(int fpga, int x, void (*FuncPtr)(int, int)){
    int i;
    int delay_time = 800 / (2*(x));
    for (i=0; i<=x; i++){
        (*FuncPtr)(fpga, i);
        delay(delay_time);
    }
    for (i=(x-1); i>=0; i--){
        (*FuncPtr)(fpga, i);
        if(i == 0)
            delay(delay_time);
    }
}
