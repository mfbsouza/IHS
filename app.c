#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <SDL2/SDL_mixer.h>

int main() {

    /* General Purpose Variables */
    unsigned char teste = 'd';

    /* Initialzing SDL Mixer, frequency, Channels & Chunks */
    Mix_Init(MIX_INIT_MID);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    Mix_Chunk *C  = Mix_LoadWAV("Samples/C.aif");
    Mix_Chunk *G  = Mix_LoadWAV("Samples/G.aif");
    Mix_Chunk *Am = Mix_LoadWAV("Samples/Am.aif");
    Mix_Chunk *F  = Mix_LoadWAV("Samples/F.aif");

	/* Variables for Serial Device */
	int fd;
    struct termios config;
	const char *arduino = "/dev/ttyACM0";

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
    while(teste != 'q') {
        if(read(fd,&teste,1) > 0) {
            //printf("received char: %c", teste);
            if(teste == 'a')
                Mix_PlayChannel(1, C, 0);
            if(teste == 'b')
                Mix_PlayChannel(1, G, 0);
            if(teste == 'c')
                Mix_PlayChannel(1, Am, 0);
            if(teste == 'd')
                Mix_PlayChannel(1, F, 0);
        }
    }

    /* Release Resources */
    Mix_FreeChunk(C);
    Mix_FreeChunk(G);
    Mix_FreeChunk(Am);
    Mix_FreeChunk(F);
    Mix_CloseAudio();
    Mix_Quit();
    close(fd);
	return 0;
}