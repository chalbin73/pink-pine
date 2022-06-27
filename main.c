#include<stdio.h>
#include<SDL2/SDL.h>
#include<stdint.h>
#include<unistd.h>
#include<stdbool.h>
#include<stdlib.h>


// 3 bit number : 000
// Increment each audio frame += 1
// Thus : 000 001 010 011 100 101 110 111 000 .. (loop back)

#define DEFAULT_GEN_COUNT 25
#define DEFAULT_VOLUME 10 // Between 1 and 100


typedef struct {
    Sint16 volume;
    Uint16 gen_count;
} SoundContext;

//The state of the generator
typedef struct{
    //The counter, incremented each "audio frame" (see voss algo)
    Uint32 counter;
    
    //The values for each generator
    Sint16* gen_vals;
} GenState;

#define ALGORITHMS_COUNT 3

//The different implemented algorithms
typedef enum{
    VOSS,
    VOSS_MCCARTNEY,
    WHITE_NOISE
} AlgorithmType;

const char* const algo_names[] ={
    [VOSS] = "Voss algorithm : Pink noise",
    [VOSS_MCCARTNEY] = "Voss McCartney algorithm : Pink noise",
    [WHITE_NOISE] = "Simple random white noise"
};

//Global generator state
GenState* g_state;

//Counts the nuber of zeros at the end of bits
//This is needed for the voss mccartney algorithm
int count_trailing_zeros(Uint32 data){

    int i = 0;
    while(i < 32){
	if(!((data >> i) & 1))
	    i++;
	else
	    break;
    }

    return i;
}

// SDL2 Audio callback
// Algorithm from https://www.firstpr.com.au/dsp/pink-noise/#Voss
// Only voss algorithm implemented in this function
void noise_callback_voss(void *userdata, Uint8 *stream, int len) {
    GenState* cur_state_ptr = g_state;

    SoundContext scon = *(SoundContext*)userdata;
    
    for(int i = 0; i < len / sizeof(Uint16); i++){
	//Foreach sample requested by audio driver
	Uint32 last_count = cur_state_ptr->counter;
	cur_state_ptr->counter += 1;

	Sint32 sum = 0;
	
	if(cur_state_ptr->counter > UINT32_MAX >> (32 - scon.gen_count))
	    cur_state_ptr->counter = 0;

	Uint32 changed_bits = last_count ^ cur_state_ptr->counter; // XOR Only keeps bit that have changed
	
	for(int g = 0; g < scon.gen_count; g++){

	    if(changed_bits & (1 << g)){
		Sint16 value = (Sint16)rand() % (Sint16)scon.volume;
		Sint16 sign = (rand() % 2) * 2 - 1;
		//printf("S%d", sign);
		cur_state_ptr->gen_vals[g] = value * sign;
	    }
	    sum += cur_state_ptr->gen_vals[g];
	}

	sum = sum / scon.gen_count;
	((Sint16*)stream)[i] = sum;
    }
}

// SDL2 Audio callback
// Algorithm from https://www.firstpr.com.au/dsp/pink-noise/#Voss
// Only voss McCartney algorithm implemented in this function
void noise_callback_voss_mccartney(void *userdata, Uint8 *stream, int len) {
    GenState* cur_state_ptr = g_state;

    SoundContext scon = *(SoundContext*)userdata;
    
    for(int i = 0; i < len / sizeof(Uint16); i++){
	//Foreach sample requested by audio driver
	Uint32 last_count = cur_state_ptr->counter;
	cur_state_ptr->counter += 1;

	Sint32 sum = 0;

	//Put counter back to zero
	if(cur_state_ptr->counter > UINT32_MAX >> (32 - scon.gen_count))
	    cur_state_ptr->counter = 0;

	for(int g = 0; g < scon.gen_count; g++){
	    if(g == count_trailing_zeros(cur_state_ptr->counter)){

		//Generate new value
		Sint16 value = rand() % scon.volume;
		Sint16 sign = (rand() % 2) * 2 - 1;

		cur_state_ptr->gen_vals[g] = value * sign;
	    }
	    sum += cur_state_ptr->gen_vals[g];
	}

	sum = sum / scon.gen_count;
	((Sint16*)stream)[i] = sum;
    }
}


// SDL2 Audio callback
// Simple white noise generation
void noise_callback_white(void *userdata, Uint8 *stream, int len) {
  GenState *cur_state_ptr = g_state;

  SoundContext scon = *(SoundContext *)userdata;

  for (int i = 0; i < len / sizeof(Uint16); i++) {
    // Foreach sample requested by audio driver

    Sint16 value = (Sint16)rand() % (Sint16)scon.volume;
    Sint16 sign = (rand() % 2) * 2 - 1;

    ((Sint16 *)stream)[i] = value * sign;
  }
}

void print_algos(){
    printf("[-a number] number: choosen algorithm\nDefault is Voss-McCartney (2)\n");

    for(int i = 0; i < ALGORITHMS_COUNT; i++){
	printf("[%d] %s\n", i + 1, algo_names[i]);
    }
}

void print_audio_warning(const char* message){

    puts(message);
    puts("Are you sure you want to proceed ? (Y/N)");
    
    int c = getchar();
    
    if(c == 'y' || c == 'Y')
	return;
    else
	exit(0);
}

int main(int argc, char** argv)
{
    //Read command line arguments

    AlgorithmType algo = 0;
    int generator_count = DEFAULT_GEN_COUNT;
    int volume = DEFAULT_VOLUME;
    
    int out = 0;

    bool warning = true;
    
    while ((out = getopt(argc, argv, "a:v:g:hw")) != -1) {
      switch (out) {
      case ':':
        printf("Please provide input ! \n");
        exit(1);
        break;

      case '?':
        printf("Unknown option %c\n", optopt);
        exit(1);
        break;

      case 'a':
        if (atoi(optarg) < 1 || atoi(optarg) > ALGORITHMS_COUNT) {
          print_algos();
          exit(1);
        } else {
          algo = atoi(optarg) - 1;
        }
        break;

      case 'v':
        if (atoi(optarg) < 1 || atoi(optarg) > 100) {
          printf("Usage [-v volume] volume must be between 1 and 100 !\n");
          exit(1);
        } else {
          volume = atoi(optarg);
        }
        break;

      case 'g':
        if (atoi(optarg) < 1 || atoi(optarg) > 32) {
          printf("Usage [-g generator_count] generator_count must be between 1 and 32 !\n");
          exit(1);
        } else {
          generator_count = atoi(optarg);
        }
        break;

      case 'h':
        printf("Help:\n");
        printf("Usage : pink-pine [-v volume] [-g generator_count] [-a algorithm]\n");
        exit(0);
        break;

      case 'w':
	  warning = false;
	  break;
      }
    }
    
    printf("Volume : %d %%  \n", volume);
    printf("Generator count : %d\n", generator_count);
    printf("Algorithm : %s\n", algo_names[algo]);


    
    
    SDL_Init(SDL_INIT_AUDIO);
    
    SDL_AudioCallback algo_callback = NULL;
    if(algo == VOSS)
	algo_callback = noise_callback_voss;
    else if(algo == VOSS_MCCARTNEY)
	algo_callback = noise_callback_voss_mccartney;
    else if(algo == WHITE_NOISE)
	algo_callback = noise_callback_white;
    else{puts("Unreachable"); exit(1);}

    if(warning && algo == WHITE_NOISE && volume >= 50)
	print_audio_warning("\033[1;31m WARNING,loud white noise may cause earing injuries, (ignore this warning with [-w])\033[0m");
    
    Sint16 actual_volume = (Sint16)floorf(((float)volume / 100.0) * (float)(UINT16_MAX / 2));
    
    SoundContext scon = {.volume = actual_volume, .gen_count = generator_count}; 

    GenState gen_state = {.counter = 0, .gen_vals = (Sint16*)malloc(sizeof(Sint16) * generator_count)};
    g_state = &gen_state;

    SDL_AudioSpec audio_spec = {.freq = 48000, .channels = 2, .format = AUDIO_S16LSB, .callback = algo_callback, .userdata = &scon};

    SDL_OpenAudio(&audio_spec, NULL);
    
    SDL_Event event;

    //SDL Audio is paused by default
    SDL_PauseAudio(0);


    //Simple event loop
    bool exit = false;
    while(!exit)
    {
	if(SDL_PollEvent(&event))
	{
	    switch(event.type)
	    {
	    case SDL_QUIT:
		exit = true;
		break;
	    }
	}
    }
  
    SDL_Quit();

    free(gen_state.gen_vals);
    
    return 0;
}
