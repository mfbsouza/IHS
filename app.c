#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
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

void LoadGuitar(Mix_Chunk **Notes);
void LoadDrums(Mix_Chunk **Notes);
void FreeAudio(Mix_Chunk **Notes);

int main() {

    /* General Purpose Variables */
    unsigned char aux;
    char buffer[5];
    char instrument = '0';
    int i = 0;

    /* Initialzing SDL Mixer, frequency, Channels & Chunks */
    Mix_Init(MIX_INIT_MID);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    Mix_Chunk *Notes[NotesNUM];
    //LoadGuitar(Notes);
    LoadDrums(Notes);
    
	/* Variables for Serial Device */
	int fd;
    struct termios config;
	const char *arduino = "/dev/ttyACM0";

    /* Variables for Altera FPGA */
    int fpga;
    const char *altera = "/dev/de2i150_altera";
    //fpga = open(altera, O_RDWR);
    //if(fpga == -1)
    //    printf("Failed to Open Device: %s\n", altera);

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
    #pragma omp parallel sections num_threads(2)
    {
        #pragma omp section
        {
            while(buffer[4] != '1') {
                scanf("%c", &instrument);
                if(instrument == 'g'){
                    FreeAudio(Notes);
                    LoadGuitar(Notes);
                }
                if(instrument == 'd'){
                    FreeAudio(Notes);
                    LoadDrums(Notes);
                }
            }
        }
        #pragma omp section
        {
            while(buffer[4] != '1') {
                if(read(fd, &aux, 1) > 0)
                buffer[i++] = aux;
                if(i == 5) {
                    i = 0;
                    if(buffer[0] == '1')
                        Mix_PlayChannel(1, Notes[0], 0);
                    if(buffer[1] == '1')
                        Mix_PlayChannel(1, Notes[1], 0);
                    if(buffer[2] == '1')
                        Mix_PlayChannel(1, Notes[2], 0);
                    if(buffer[3] == '1')
                        Mix_PlayChannel(1, Notes[3], 0);
                }
            }
        }
    }

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
}

void LoadDrums(Mix_Chunk **Notes) {
    Notes[0] = Mix_LoadWAV("Samples/drum1.aif");
    Notes[1] = Mix_LoadWAV("Samples/drum2.aif");
    Notes[2] = Mix_LoadWAV("Samples/drum3.aif");
    Notes[3] = Mix_LoadWAV("Samples/drum4.aif");
}

void FreeAudio(Mix_Chunk **Notes) {
    for(int i = 0; i < NotesNUM; i++)
        Mix_FreeChunk(Notes[i]);
}