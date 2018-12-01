#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <time.h>
#include <termios.h>
#include <omp.h>
#include <SDL2/SDL_mixer.h>

#define NotesNUM    4
#define DISPLAY_L   1
#define DISPLAY_R   2
#define SWITCHES    3
#define PUSHBUTTON  4
#define GREENLEDS   5
#define REDLEDS     6

// define hex codes for 7-segments display
#define d7_0 0x40
#define d7_1 0x79
#define d7_2 0x24
#define d7_3 0x30
#define d7_4 0x19
#define d7_5 0x12
#define d7_6 0x02
#define d7_7 0x78
#define d7_8 0x00
#define d7_9 0x18
#define d7_A 0x08
#define d7_B 0x00
#define d7_C 0x46
#define d7_D 0x40
#define d7_E 0x06
#define d7_F 0x0E
#define d7_G 0x42
#define d7_empty 0x7F


void LoadGuitar(Mix_Chunk **Notes);
void LoadDrums(Mix_Chunk **Notes);
void FreeAudio(Mix_Chunk **Notes);
void delay(int num_of_secs);

int main() {

    /* Initialzing SDL Mixer, frequency, Channels & Chunks */
    Mix_Init(MIX_INIT_MID);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    /* General Purpose Variables */
    Mix_Chunk *Notes[NotesNUM];
    Mix_Chunk *FPGAdrums[NotesNUM];
    LoadDrums(FPGAdrums);
    LoadGuitar(Notes);
    //LoadDrums(Notes);
    
	/* Variables for Serial Device */
	int fd, i = 0, delay_msec = 500;
    struct termios config;
    char buffer[5];
    unsigned char aux;
	const char *arduino = "/dev/ttyACM0";

    /* Variables for Altera FPGA */
    int fpga;
    const char *altera = "/dev/de2i150_altera";
    const uint32_t mem_trash = 0, hex_c = 0xFFFFFF46, hex_a = 0xFFFFFF08, hex_g = 0xFFFFFF42, hex_f = 0xFFFFFF0E;
    const uint32_t led_1 = 3 , led_2 = 12, led_3 = 48, led_4 = 192;
    uint32_t teste = 1, read_var = 0;

    /* Opening & Setting Up FPGA */
    fpga = open(altera, O_RDWR);
    if(fpga == -1)
        printf("Failed to Open Device: %s\n", altera);
    //first right is buggy so we write something to skip it
    write(fpga, &mem_trash, DISPLAY_L);
    write(fpga, &mem_trash, GREENLEDS);

	/* Opening Serial Device, Flags & Checking for Errors.

	   O_RDWR   = Read and Write
       O_NOCTTY = The port never becomes the controlling terminal of the process 
       O_NDELAY = Use non-blocking I/O */

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

    //testando
    #pragma omp parallel sections num_threads(3)
    {
        #pragma omp section
        {
            while(read_var != 1) {
                if(read(fpga, &read_var, PUSHBUTTON)){
                    if(read_var == 14){
                        Mix_PlayChannel(2, FPGAdrums[3], 0);
                        write(fpga, &led_1, GREENLEDS);
                    }
                    if(read_var == 13){
                        Mix_PlayChannel(2, FPGAdrums[2], 0);
                        write(fpga, &led_2, GREENLEDS);
                    }
                    if(read_var == 11){
                        Mix_PlayChannel(2, FPGAdrums[1], 0);
                        write(fpga, &led_3, GREENLEDS);
                    }
                    if(read_var == 7){
                        Mix_PlayChannel(2, FPGAdrums[0], 0);
                        write(fpga, &led_4, GREENLEDS);
                    }
                }
                if(read(fpga, &read_var, SWITCHES) && read_var == 4){
                    printf("carregando Guitarra...\n");
                    FreeAudio(Notes);
                    LoadGuitar(Notes);
                    printf("Guitarra carregada com sucesso\n");
                }
                if(read(fpga, &read_var, SWITCHES) && read_var == 8){
                    printf("carregando bateria...\n");
                    FreeAudio(Notes);
                    LoadDrums(Notes);
                    printf("Bateria carregada com sucesso\n");
                }
            }
        }
        #pragma omp section
        {
            while(read_var != 1) {
                if(read(fd, &aux, 1) > 0)
                    buffer[i++] = aux;
                if(i == 5) {
                    i = 0;
                    if(buffer[0] == '1'){
                        Mix_PlayChannel(1, Notes[0], 0);
                        write(fpga, &hex_c, DISPLAY_R);
                    }
                    if(buffer[1] == '1'){
                        Mix_PlayChannel(1, Notes[1], 0);
                        write(fpga, &hex_g, DISPLAY_R);
                    }
                    if(buffer[2] == '1'){
                        Mix_PlayChannel(1, Notes[2], 0);
                        write(fpga, &hex_a, DISPLAY_R);
                    }
                    if(buffer[3] == '1'){
                        Mix_PlayChannel(1, Notes[3], 0);
                        write(fpga, &hex_f, DISPLAY_R);
                    }
                    if(buffer[4] == '1'){
                        delay_msec += 100;
                    }
                }
            }
        }
        #pragma omp section
        {
            while(read_var != 1) {
                if(delay_msec == 1100){
                    delay_msec = 100;
                }
                if(teste == 131072){
                        teste = 1;
                    } else {
                        teste = teste << 1;
                    }
                    write(fpga, &teste, REDLEDS);
                    delay(delay_msec);
            }
        }
    }

    printf("fechando o programa...\n");
    /* Release Resources */
    FreeAudio(Notes);
    Mix_CloseAudio();
    Mix_Quit();
    close(fd);
	return 0;
}

void LoadGuitar(Mix_Chunk **Notes) {
    Notes[0] = Mix_LoadWAV("Samples/C.aif");
    Notes[1] = Mix_LoadWAV("Samples/G.aif");
    Notes[2] = Mix_LoadWAV("Samples/Am.aif");
    Notes[3] = Mix_LoadWAV("Samples/F.aif");
    //Notes[4] = Mix_LoadWAV("Samples/G.aif"); // sol
    //Notes[5] = Mix_LoadWAV("Samples/A.aif"); // lá
    //Notes[6] = Mix_LoadWAV("Samples/B.aif"); // si
}

void LoadDrums(Mix_Chunk **Notes) {
    Notes[0] = Mix_LoadWAV("Samples/drum1.aif");
    Notes[1] = Mix_LoadWAV("Samples/drum2.aif");
    Notes[2] = Mix_LoadWAV("Samples/drum3.aif");
    Notes[3] = Mix_LoadWAV("Samples/drum4.aif");
}

void FreeAudio(Mix_Chunk **Notes) {
    int i;
    for(i = 0; i < NotesNUM; i++)
        Mix_FreeChunk(Notes[i]);
}

void delay(int num_of_secs) {
    int milli_sec = 1000*num_of_secs;
    clock_t start_time = clock();
    while(clock() < start_time + milli_sec)
        ;
}