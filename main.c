#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <stdbool.h>

#ifdef WIN32
#define PATH_MAX 260 // bruhhhhh
#include "Windows.h"
#else
#include <linux/limits.h>
#endif

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

// Textures
static SDL_Texture* flashbangTexture = NULL;
// Audio
static MIX_Audio *bang = NULL;
static MIX_Audio *bounce = NULL;
static MIX_Mixer* device = NULL;

static int screenWidth, screenHeight;

// Delta time
static Uint64 NOW = 0;
static Uint64 LAST = 0;
static double deltaTime = 0;

#define FLASH_LENGTH 400
#define WIN_SIZE 150
#define TICK 0.05f

float angle = 0;
float x, y;
float xVel, yVel;
int bounces = 0;
bool flash = false;
float flashProgression = 0;

static int deltatimeMultiplier = 500; // I hate everything about programming. Every. single. aspect

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
	ShowWindow(GetConsoleWindow(),0);
	if (argc > 1)
	{
		deltatimeMultiplier = SDL_atoi(argv[1]);
	}

	const char* wcd = SDL_GetBasePath();
	// Init sdl
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        	SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        	return SDL_APP_FAILURE;
    	}
	/* Create the window */
	if (!SDL_CreateWindowAndRenderer("Flashbang!", WIN_SIZE, WIN_SIZE, SDL_WINDOW_TRANSPARENT | SDL_WINDOW_BORDERLESS, &window, &renderer)) {
	    SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
	    return SDL_APP_FAILURE;
	}
	SDL_SetRenderVSync(renderer, 1);
	// Load texture
	char* pathFlash = SDL_malloc(PATH_MAX);
	SDL_snprintf(pathFlash, PATH_MAX, "%sflashbang.png", wcd);
	flashbangTexture = IMG_LoadTexture(renderer, pathFlash);
	SDL_free(pathFlash);
	// Audio
	if (!MIX_Init()) {
        SDL_Log("MIX_Init failed: %s\n", SDL_GetError());
	}
	device = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
	if (!device)
	{
        SDL_Log("Audio device failed: %s\n", SDL_GetError());
	}
	// Wav files
	char* pathBang = SDL_malloc(PATH_MAX);
	SDL_snprintf(pathBang, PATH_MAX, "%sbang.wav", wcd);
	bang = MIX_LoadAudio(device,pathBang, true);
	if (bang == NULL)
		SDL_Log("Couldn't load \"bang.wav\": %s\n", SDL_GetError());
	SDL_free(pathBang);

	char* pathBounce = SDL_malloc(PATH_MAX);
	SDL_snprintf(pathBounce, PATH_MAX, "%sbounce.wav", wcd);
	bounce = MIX_LoadAudio(device, pathBounce, true);
	if (bounce == NULL)
		SDL_Log("Couldn't load \"bounce.wav\": %s\n", SDL_GetError());
	SDL_free(pathBounce);

	// Screen size
	screenWidth = 1920;
	screenHeight = 1080;
	// Position window
	x = y = -WIN_SIZE;
	xVel = 4;
	yVel = 0.5;
	return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	if (event->type == SDL_EVENT_QUIT) {
	    return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
	}
	return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
	// Deltatime
	LAST = NOW;
    NOW = SDL_GetPerformanceCounter();

    deltaTime = (double)((NOW - LAST) * deltatimeMultiplier / (double)SDL_GetPerformanceFrequency());
    deltaTime = SDL_clamp(deltaTime, 0.5, 5);

	// Flashbang code
	if (!flash)
	{
		//SDL_Log("%f %f %f %f ", x, xVel, y, yVel);
		angle += deltaTime;
		x += xVel * deltaTime;
		y += yVel * deltaTime;
		if (xVel > 0)
			xVel -= (TICK/12) * deltaTime;
		if (y < screenHeight -(screenHeight/3)) // a bit below screen halfway y
			yVel += TICK * deltaTime;
		else
		{
			MIX_PlayAudio(device, bounce);
			bounces++;
			yVel = ((float)-7/(bounces));
			y += yVel * 3;
		}
		// Position window
		SDL_SetWindowPosition(window, x, y);
		// Draw the flashbang
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);
		SDL_FRect dstrect = {0,0, ((float)142/334)*WIN_SIZE, WIN_SIZE};
		dstrect.x = (WIN_SIZE/2) - (dstrect.w/2);
		dstrect.y = (WIN_SIZE/2) - (dstrect.h/2);
		SDL_RenderTextureRotated(renderer, flashbangTexture, NULL, &dstrect, angle, NULL, SDL_FLIP_NONE);
		SDL_RenderPresent(renderer);

		if (bounces > 3)
		{
			MIX_StopAllTracks(device, 0);
			MIX_PlayAudio(device, bang);
			SDL_HideCursor();
			SDL_SetWindowFullscreen(window, true);
			SDL_SetWindowAlwaysOnTop(window, true);
			flash = true;
		}
	}
	else
	{
		flashProgression += deltaTime/10; // Never doing a single project like this ever again
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		if (flashProgression > FLASH_LENGTH/3)
			SDL_SetWindowOpacity(window, 1-((1.0f/(FLASH_LENGTH/3))*(flashProgression-(FLASH_LENGTH/3))));
		SDL_RenderClear(renderer);
		SDL_RenderPresent(renderer);
		if (flashProgression > FLASH_LENGTH)
			return SDL_APP_SUCCESS; // Finished
	}
	return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	MIX_DestroyAudio(bang);
	MIX_DestroyAudio(bounce);
	MIX_DestroyMixer(device);
	MIX_Quit();
}

