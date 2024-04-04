#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string>
#include <sstream>

/* constants */
/* default screen dimensions */
const int SCREEN_WIDTH = 1080;
const int SCREEN_HEIGHT = 720;

SDL_Window* gWindow = NULL; //the window we will be rendering to
SDL_Renderer* gRenderer = NULL; //the window renderer
TTF_Font* gFont = NULL; //global font

typedef struct Entity
{
        SDL_Rect rect = { 0, 0, 0, 0 };
        bool hasBounce = false;
        int xVel = 0;
        int yVel = 0;
};

Entity puck, lPaddle, rPaddle;
unsigned int lScore = 0, rScore = 0;

/* Texture wrapper class */
/* source from lazyfoo.net */
class LTexture
{
        public:
                /* initializes variables */
                LTexture();

                /* deallocates memory */
                ~LTexture();

                /* loads image at specified path */
                bool loadFromFile(std::string path);

                /* creates image from font string */
                bool loadFromRenderedText(std::string textureText, SDL_Color textColor);

                /* deallocates texture */
                void free();

                /* set color modulation */
                void setColor(Uint8 red, Uint8 green, Uint8 blue);

                /* set blending */
                void setBlendMode(SDL_BlendMode blending);

                /* set alpha modulation */
                void setAlpha(Uint8 alpha);

                /* Renders texture at given point */
                void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

                /* gets image dimensions */
                int getWidth();
                int getHeight();

        private:
                /* the actual hardware texture */
                SDL_Texture *mTexture;

                /* image dimensions */
                int mWidth;
                int mHeight;
};

LTexture::LTexture()
{
        /* initialize */
        mTexture = NULL;
        mWidth = 0;
        mHeight = 0;
}

LTexture::~LTexture()
{
        /* deallocate */
        free();
}

bool LTexture::loadFromFile(std::string path)
{
        /* get rid of preexisting texture */
        free();

        /* the final texture */
        SDL_Texture *newTexture = NULL;
        /* load image at specified path */
        SDL_Surface *loadedSurface = IMG_Load(path.c_str());
        if (loadedSurface == NULL) {
                printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
        }
        else {
                /* color key image */
                SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0Xff, 0xFF));

                /* create texture from surface pixels */
                newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
                if (newTexture == NULL) {
                        printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
                }
                else {
                        /* get image dimensions */
                        mWidth = loadedSurface->w;
                        mHeight = loadedSurface->h;
                }

                /* get rid of old loaded surface */
                SDL_FreeSurface(loadedSurface);
        }

        /* return success */
        mTexture = newTexture;
        return mTexture != NULL;
}

bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor)
{
        /* get rid of preexisting texture */
        free();

        /* render text surface */
        SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
        if (textSurface == NULL) {
                printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        }
        else {
                /* create texture from surface pixels */
                if ((mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface)) == NULL) {
                        printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
                }
                else {
                        /* get image dimensions */
                        mWidth = textSurface->w;
                        mHeight = textSurface->h;
                }

                /* get rid of old surface */
                SDL_FreeSurface(textSurface);
        }
        /* return success */
        return mTexture != NULL;
}

void LTexture::free()
{
        /* free texture if it exists */
        if (mTexture != NULL) {
                SDL_DestroyTexture(mTexture);
                mTexture = NULL;
                mWidth = 0;
                mHeight = 0;
        }
}

void LTexture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
        /* set rendering space and render to screen */
        SDL_Rect renderQuad = { x, y, mWidth, mHeight };

        /* set clip rendering dimensions */
        if (clip != NULL) {
                renderQuad.w = clip->w;
                renderQuad.h = clip->h;
        }

        /* render to screen */
        SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

int LTexture::getWidth()
{
        return mWidth;
}

int LTexture::getHeight()
{
        return mHeight;
}

LTexture gScoreTexture; //the texture used to display the score




/* function declarations */
int init(); //initializes libraries
bool loadMedia(); //loads textures, fonts, &c.
void moveEntity(Entity *en); //moves entity on rendered screen
void close(); //frees memory from SDL libraries

/* function definitions */
int init()
{
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
                printf("SDL could not be initialized. SDL Error:%s\n", SDL_GetError());
                return 0;
        }
        if ((gWindow = SDL_CreateWindow("pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN)) == NULL) {
                printf("Window could not be created. SDL Error:%s\n", SDL_GetError());
                return 0;
        }
        if ((gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)) == NULL) {
                printf("Renderer could not be created. SDL Error:%s\n", SDL_GetError());
                return 0;
        }

        /* initialize png loading */
        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
                printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
                return 0;
        }

        /*initialize SDL_ttf */
        if (TTF_Init() == -1) {
                printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
                return 0;
        }

        return 1;
}

bool loadMedia()
{
        /* laoding success flag */
        bool success = true;

        /* open the font */
        /* Copyright (c) 2011 by Sorkin Type Co (www.sorkintype.com), with Reserved Font Name "Basic". */
        if ((gFont = TTF_OpenFont("font/Basic-Regular.ttf", 64)) == NULL) { 
                printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
                success = false;
        }

        return success;
}


void moveEntity(Entity *en)
{
        en->rect.x += en->xVel;
        if (en->rect.x > SCREEN_WIDTH || en->rect.x < 0 - en->rect.w) {
                en->rect.x -= en->xVel;
                if (en->hasBounce) {
                        en->xVel *= -1;
                }
        }

        en->rect.y += en->yVel;
        if (en->rect.y + en->rect.h > SCREEN_HEIGHT || en->rect.y < 0) {
                en->rect.y -= en->yVel;
                if (en->hasBounce) {
                        en->yVel *= -1;
                }
        }
}

void close()
{
        lScore = rScore = 0;

        gScoreTexture.free();

        TTF_CloseFont(gFont);
        gFont = NULL;

        SDL_DestroyRenderer(gRenderer);
        SDL_DestroyWindow(gWindow);
        gWindow = NULL;
        gRenderer = NULL;

        TTF_Quit();
        IMG_Quit();
        SDL_Quit();

}

/* main function */
int main()
{
        /* setup */
        lScore = rScore = 0;

        int i;
        if ((i=init()) == 0) {
                perror("pong could not initialize\n");
                return -1;
        }
        if (!loadMedia()) {
                perror("failed to load media\n");
                return -1;
        }

        puck.rect = { 530, 350, 20, 20 };
        puck.xVel = 10;
        puck.yVel = 10;
        puck.hasBounce = true;

        lPaddle.rect = { 24, 270, 20, 90 };
        lPaddle.xVel = 0;
        lPaddle.yVel = 0;
        lPaddle.hasBounce = false;

        rPaddle.rect = { 1036, 270, 20, 90 };
        rPaddle.xVel = 0;
        rPaddle.yVel = 0;
        rPaddle.hasBounce = false;

        SDL_Event e; //event handling
        SDL_Color textColor = { 0, 0, 0, 255 }; // color for displayed score
        std::stringstream scoreDisplayText; //text to display score on screen

        /* main loop */
        bool quit = false;
        while (!quit) {
                /* move entities */
                moveEntity(&puck);
                moveEntity(&lPaddle);
                moveEntity(&rPaddle);
                /* key state array */
                const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
                /* handle events */
                while (SDL_PollEvent(&e) != 0) {
                        /* user requests quit */
                        if (e.type == SDL_QUIT) {
                                quit = true;
                        }
                }

                /* keystate handling */
                if (currentKeyStates[SDL_SCANCODE_Q] && !currentKeyStates[SDL_SCANCODE_A])
                        lPaddle.yVel = -20;
                else if (currentKeyStates[SDL_SCANCODE_A])
                        lPaddle.yVel = 20;
                else
                        lPaddle.yVel = 0;

                if (currentKeyStates[SDL_SCANCODE_UP] && !currentKeyStates[SDL_SCANCODE_DOWN])
                        rPaddle.yVel = -20;
                else if (currentKeyStates[SDL_SCANCODE_DOWN])
                        rPaddle.yVel = 20;
                else
                        rPaddle.yVel = 0;

                /* puck collision with paddles */
                if (SDL_HasIntersection(&puck.rect, &lPaddle.rect)) {
                        puck.rect.x = lPaddle.rect.x + lPaddle.rect.w;
                        puck.xVel *= -1;
                }
                if (SDL_HasIntersection(&puck.rect, &rPaddle.rect)) {
                        puck.rect.x = rPaddle.rect.x - puck.rect.w;
                        puck.xVel *= -1;
                }

                /* score */

                if (puck.rect.x > rPaddle.rect.x + rPaddle.rect.w) {
                        lScore++;
                        puck.rect.x = 530;
                        puck.rect.y = 350;
                        printf("Left: %u, Right: %u\n", lScore, rScore);
                }
                if (puck.rect.x + puck.rect.w < lPaddle.rect.x) {
                        rScore++;
                        puck.rect.x = 530;
                        puck.rect.y = 350;
                        printf("Left: %u, Right: %u\n", lScore, rScore);
                }
                scoreDisplayText.str("");
                scoreDisplayText << lScore << "        " << rScore;
                /* render score on screen */
                if (!gScoreTexture.loadFromRenderedText(scoreDisplayText.str().c_str(), textColor)) {
                        printf("Unable to render score texture.\n");
                }

                if (rScore >= 10) {
                        printf("Right Wins!\n");
                        quit = true;
                }
                if (lScore >= 10) {
                        printf("Left Wins!\n");
                        quit = true;
                }

                /* clear screen */
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(gRenderer);

                /* render puck */
                SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
                SDL_RenderFillRect(gRenderer, &(puck.rect));

                /* render left paddle */
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);
                SDL_RenderFillRect(gRenderer, &(lPaddle.rect));

                /* render right paddle */
                SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF);
                SDL_RenderFillRect(gRenderer, &(rPaddle.rect));

                /* render score */
                gScoreTexture.render((SCREEN_WIDTH - gScoreTexture.getWidth()) / 2, 24);

                /* update screen */
                SDL_RenderPresent(gRenderer);
        }

        close();

        return 0;
}

