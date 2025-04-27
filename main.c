#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <stdbool.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

// Textures
static SDL_Texture* flashbangTexture = NULL;
// Audio
static Mix_Chunk *bang = NULL;
static Mix_Chunk *bounce = NULL;

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
int flashProgression = 0;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
	// Init sdl
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        	SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        	return SDL_APP_FAILURE;
    	}
	/* Create the window */
	if (!SDL_CreateWindowAndRenderer("Hello World", WIN_SIZE, WIN_SIZE, SDL_WINDOW_TRANSPARENT | SDL_WINDOW_BORDERLESS, &window, &renderer)) {
	    SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
	    return SDL_APP_FAILURE;
	}
	SDL_SetRenderVSync(renderer, 1);
	// Load texture
	flashbangTexture = IMG_LoadTexture(renderer, "flashbang.png");
	// Audio
	if (!Mix_OpenAudio(0, NULL)) {
        	SDL_Log("Couldn't open audio: %s\n", SDL_GetError());
	}
	// Wav files
	bang = Mix_LoadWAV("bang.wav");
	if (bang == NULL)
		SDL_Log("Couldn't load \"bang.wav\": %s\n", SDL_GetError());
	bounce = Mix_LoadWAV("bounce.wav");
	if (bounce == NULL)
		SDL_Log("Couldn't load \"bounce.wav\": %s\n", SDL_GetError());

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

    	deltaTime = (double)((NOW - LAST) * 4000 / (double)SDL_GetPerformanceFrequency());
    	deltaTime = SDL_clamp(deltaTime, 0.1, 9);

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
			Mix_PlayChannel(0, bounce, 0);
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
			Mix_HaltChannel(0);
			Mix_PlayChannel(0, bang, 0);
			SDL_HideCursor();
			SDL_SetWindowFullscreen(window, true);
			SDL_SetWindowAlwaysOnTop(window, true);
			flash = true;
		}
	}
	else
	{
		flashProgression++;
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
	Mix_FreeChunk(bang);
	Mix_FreeChunk(bounce);
	Mix_CloseAudio();
}

