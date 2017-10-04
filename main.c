/*
 * Tetris
 *
 * Coded by Calin Pescaru
 *
 * 2017
 *
 * Grid size 24 x 16
 * Unit size 32
 *
 */

// Includes
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Definitions
#define BORDER 256 // left margin of grid
#define M 24 // grid y axis
#define N 16 // grid x axis
#define MAX_LEVEL 5 // max level
#define MAX_SCORE 1000000 // max score
#define SIZE 4 // tetromino size
#define WIN_W 768 // window width
#define WIN_H 768  // window height

// Global functions
void check_lines(void);
int count_digits(int number);
void create_text(char* label, SDL_Texture** texture);
void draw(void);
void game_loop(void);
void gravity(int i);
bool hit(void);
bool init(void);
void level_up(void);
void lock(void);
void move_down(void);
void move_left(void);
void move_right(void);
void rotate(void);
void select_tetromino(void);
void update_score(void);
void quit(void);

// Global variables
SDL_Renderer* renderer = NULL;
SDL_Surface* text = NULL;
SDL_Texture* text_lbl_level = NULL;
SDL_Texture* text_lbl_score = NULL;
SDL_Texture* text_level = NULL;
SDL_Texture* text_score = NULL;
SDL_Window* window = NULL;
TTF_Font *font = NULL;

// basic rectangle unit
typedef struct
{
	int red;
	int green;
	int blue;

	bool hasRect;
	SDL_Rect rect;
} unit;

// one tetromino has 4 rectangles
unit tetromino[SIZE];

// playing grid size
unit grid[M][N];

// check if tetromino can move
bool canMove = true;

// temporary rect for recording tetromino position
SDL_Rect temp[SIZE];

// falling speed
int speed = 1000;

// record the score and level
int score = 0;
int level = 1;

/*
 * Main function
 */
int main(void)
{
	// initialize
	if (!init())
	{
		atexit(quit);
		exit(1);
	}

	// init grid
	for (int i = 0; i < M; i++)
	{
		for (int j = 0; j < N; j++)
		{
			grid[i][j].red = 0;
			grid[i][j].green = 0;
			grid[i][j].blue = 0;
			grid[i][j].hasRect = false;
			grid[i][j].rect.x = 0;
			grid[i][j].rect.y = 0;
			grid[i][j].rect.w = 0;
			grid[i][j].rect.h = 0;
		}
	}

	create_text("Score:", &text_lbl_score);
	create_text("Level:", &text_lbl_level);
	update_score();
	level_up();

	// run game
	game_loop();

	// run quit() when game exits
	atexit(quit);

	return 0;
}

/*
 * Check lines function
 *
 * Check whether each line in the grid is full;
 * if it is erase the line and let "gravity" do it's
 * job of bringing the other lines down; also update the score
 * and level if necessary
 */
void check_lines(void)
{
	for (int i = 0; i < M; i++)
	{
		// count each rectangle
		int count = 0;

		for (int j = 0; j < N; j++)
		{
			if (grid[i][j].hasRect)
				count++;
		}

		// if line full
		if (count == N)
		{
			// go to the next line and bring it down
			gravity(i+1);

			// come back to the previous line
			i -= 1;
			
			// update score
			score += 100;
			update_score();

			// update level every 1000th mark
			if (!(score % 1000) && level < MAX_LEVEL)
			{
				speed -= 184;
				level += 1;
				level_up();
			}
		}
	}

	return;
}

/*
 * Count digits function
 *
 * Get the amount of digits in a number
 */
int count_digits(int number)
{
	int count = 0;

	if (number == 0)
		count = 1;
	else
	{
		while (number != 0)
		{
			// divide by 10 to "lose" a digit every iteration
			number /= 10;
			count++;
		}
	}

	return count;
}

/*
 * Create text function
 *
 * Use SDL_ttf library to display text
 */
void create_text(char* label, SDL_Texture** texture)
{
	// get font
	font = TTF_OpenFont("FreeSans.ttf", 24);

	if (font == NULL)
	{
		SDL_Log("Error creating font: %s\n", TTF_GetError());
		atexit(quit);
		exit(1);
	}

	// text color - white
	SDL_Color text_color = { 255, 255, 255, 255 };

	// create text
	text = TTF_RenderText_Solid(font, label, text_color);

	if (text == NULL)
	{
		SDL_Log("Error creating text: %s\n", TTF_GetError());
		atexit(quit);
		exit(1);
	}

	// create texture from text
	*texture = SDL_CreateTextureFromSurface(renderer, text);
	SDL_FreeSurface(text);
	text = NULL;

	if (*texture == NULL)
	{
		SDL_Log("Error creating texture: %s\n", SDL_GetError());
		atexit(quit);
		exit(1);
	}

	return;
}

/*
 * Draw function
 */
void draw(void)
{
	// background color - black
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	// draw background
	SDL_RenderClear(renderer);

	// divider line color - pink
	SDL_SetRenderDrawColor(renderer, 255, 0, 80, 255);

	// draw divider line
	SDL_RenderDrawLine(renderer, 255, 0, 255, WIN_H);

	// grid line color - gray
	SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);

	int x = 288;
	int y = 32;

	// draw grid vertical lines
	for (int i = 0; i < M; i++)
	{
		SDL_RenderDrawLine(renderer, BORDER, y, WIN_W, y);
		y += 32;
	}

	// draw grid horizontal lines
	for (int i = 0; i < N; i++)
	{
		SDL_RenderDrawLine(renderer, x, 0, x, WIN_H);
		x += 32;
	}

	// current tetromino color
	SDL_SetRenderDrawColor(renderer, tetromino[0].red, tetromino[0].green, tetromino[0].blue, 255);

	// draw current tetromino
	for (int i = 0; i < SIZE; i++)
		SDL_RenderFillRect(renderer, &tetromino[i].rect);

	// draw the grid setting the color accordingly
	for (int i = 0; i < M; i++)
	{
		for (int j = 0; j < N; j++)
		{
			if (grid[i][j].hasRect)
			{
				SDL_SetRenderDrawColor(renderer, grid[i][j].red, grid[i][j].green, grid[i][j].blue, 255);
				SDL_RenderFillRect(renderer, &grid[i][j].rect);
			}
		}
	}

	// draw text
	SDL_Rect dest;

	dest.x = 67;
	dest.y = 200;
	dest.w = 120;
	dest.h = 30;

	SDL_RenderCopy(renderer, text_lbl_score, NULL, &dest);

	dest.x = 60;
	dest.y = 250;
	dest.w = count_digits(score) * 20;
	dest.h = 25;

	SDL_RenderCopy(renderer, text_score, NULL, &dest);

	dest.x = 67;
	dest.y = 300;
	dest.w = 120;
	dest.h = 30;

	SDL_RenderCopy(renderer, text_lbl_level, NULL, &dest);

	dest.x = 110;
	dest.y = 350;
	dest.w = count_digits(level) * 20;
	dest.h = 25;

	SDL_RenderCopy(renderer, text_level, NULL, &dest);

	// show frame
	SDL_RenderPresent(renderer);

	return;
}

/*
 * Main game loop function
 */
void game_loop(void)
{
	int currentTime = 0, lastTime = 0;

	draw();
	SDL_Delay(1000);

	// game loop
	while (1)
	{
		canMove = true;

		// select current tetromino
		select_tetromino();

		while (canMove)
		{
			// events structure
			SDL_Event event;

			// listen for incoming events
			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
					// quit game
					case SDL_QUIT:
						atexit(quit);
						exit(0);
						break;
					case SDL_KEYDOWN:
						switch (event.key.keysym.scancode)
						{
							// quit game
							case SDL_SCANCODE_ESCAPE:
								atexit(quit);
								exit(0);
								break;
							case SDL_SCANCODE_SPACE:
								rotate();
								break;
							case SDL_SCANCODE_LEFT:
								move_left();
								break;
							case SDL_SCANCODE_RIGHT:
								move_right();
								break;
							case SDL_SCANCODE_DOWN:
								move_down();
								break;
							default:
								break;
						}
				}
			}

			// draw current frame
			draw();

			// max score is 1 million
			if (score >= MAX_SCORE)
			{
				SDL_Delay(1000);
				atexit(quit);
				exit(0);
			}

			// move tetromino down at the rate of speed variable
			currentTime = SDL_GetTicks();

			if (currentTime > lastTime + speed)
			{
				move_down();
				lastTime = currentTime;
			}
		}
	}

	return;
}

/*
 * Gravity function
 *
 * Bring every line down one row
 */
void gravity(int i)
{
	for (; i < M; i++)
	{
		for (int j = 0; j < N; j++)
		{
			grid[i-1][j] = grid[i][j];
			grid[i-1][j].rect.y += 32;
		}
	}

	return;
}

/*
 * Hit function
 *
 * Check if the tetromino hit the boundaries of the screen,
 * or hit a rectangle already locked to the grid
 *
 * Return true if yes, or false otherwise
 */
bool hit(void)
{
	for (int i = 0; i < SIZE; i++)
	{
		// check screen boundaries
		if (tetromino[i].rect.x < BORDER || tetromino[i].rect.x >= WIN_W || tetromino[i].rect.y >= WIN_H)
			return true;
		// check collision with other rectangles
		else
		{
			// calculate indices
			int iX = (tetromino[i].rect.x - BORDER) / 32;
			int iY = abs((tetromino[i].rect.y / 32) - (M - 1));

			if (grid[iY][iX].hasRect)
				return true;
		}
	}

	return false;
}

/*
 * Init function
 *
 * Initializez all systems
 * Creates main window
 */
bool init(void)
{
	// init systems
	if (SDL_Init(SDL_INIT_VIDEO != 0))
	{
		SDL_Log("Error initializing SDL: %s\n", SDL_GetError());
		return false;
	}

	// create window
	window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_CENTERED,
								SDL_WINDOWPOS_CENTERED, WIN_W, WIN_H, 0);

	if (window == NULL)
	{
		SDL_Log("Error creating main window: %s\n", SDL_GetError());
		return false;
	}

	// create renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

	if (renderer == NULL)
	{
		SDL_Log("Error creating renderer: %s\n", SDL_GetError());
		return false;
	}

	// init text library
	if (TTF_Init() != 0)
	{
		SDL_Log("Error initializing TTF: %s\n", TTF_GetError());
		return false;
	}

	return true;
}

/*
 * Level up function
 *
 * Update level
 */
void level_up(void)
{
	char str[3] = { 0 };

	sprintf(str, "%d", level);

	create_text(str, &text_level);

	return;
}

/*
 * Lock function
 *
 * Locks the current tetromino to the screen by adding every
 * rectangle to the grid array
 *
 * Also checks if the player failed the game
 */
void lock(void)
{
	// failed game
	for (int i = 0; i < SIZE; i++)
	{
		if (temp[i].y == 0)
		{
			atexit(quit);
			exit(0);
		}

		// compute the indices and add to grid
		int iX = (temp[i].x - BORDER) / 32;
		int iY = abs((temp[i].y / 32) - (M - 1));

		grid[iY][iX].red = tetromino[i].red;
		grid[iY][iX].green = tetromino[i].green;
		grid[iY][iX].blue = tetromino[i].blue;
		grid[iY][iX].rect.x = temp[i].x;
		grid[iY][iX].rect.y = temp[i].y;
		grid[iY][iX].rect.w = temp[i].w;
		grid[iY][iX].rect.h = temp[i].h;
		grid[iY][iX].hasRect = true;

	}

	return;
}

/*
 * Move down function
 *
 * Moves current tetromino one step down
 */
void move_down(void)
{
	for (int i = 0; i < SIZE; i++)
	{
		temp[i] = tetromino[i].rect;
		tetromino[i].rect.y += 32;
	}

	if (hit())
	{
		lock();
		check_lines();
		canMove = false;
	}

	return;
}

/*
 * Move left function
 *
 * Moves current tetromino one step left
 */
void move_left(void)
{
	for (int i = 0; i < SIZE; i++)
	{
		temp[i] = tetromino[i].rect;
		tetromino[i].rect.x -= 32;
	}

	if (hit())
	{
		for (int i = 0; i < SIZE; i++)
			tetromino[i].rect = temp[i];
	}

	return;
}

/*
 * Move right function
 *
 * Moves current tetromino one step right
 */
void move_right(void)
{
	for (int i = 0; i < SIZE; i++)
	{
		temp[i] = tetromino[i].rect;
		tetromino[i].rect.x += 32;
	}

	if (hit())
	{
		for (int i = 0; i < SIZE; i++)
			tetromino[i].rect = temp[i];
	}

	return;
}

/*
 * Rotate function
 *
 * Rotates current tetromino 90 degrees at a time
 */
void rotate(void)
{
	// pivot point
	SDL_Rect p = tetromino[1].rect;

	for (int i = 0; i < SIZE; i++)
	{
		// rotation formula
		int x = tetromino[i].rect.y - p.y;
		int y = tetromino[i].rect.x - p.x;

		tetromino[i].rect.x = p.x - x;
		tetromino[i].rect.y = p.y + y;
	}

	if (hit())
	{
		for (int i = 0; i < SIZE; i++)
			tetromino[i].rect = temp[i];
	}

	return;
}

/*
 * Select the current tetromino
 *
 * There are 7 tetrominos in total
 * First randomly select a number between 0 and 6
 * Then set the color accordingly and position the
 * tetromino at the top-center of the playing grid
 *
 * I - cyan
 * J - blue
 * L - orange
 * O - yellow
 * S - green
 * T - purple
 * Z - red
 */
void select_tetromino(void)
{
	// select tetromino piece
	srand(time(0));
	int choice = rand();
	choice %= 7;

	// center of playing grid
	int x = (6 * 32) + BORDER;
	int y = 0;

	switch (choice)
	{
		// I
		case 0:
			for (int i = 0; i < SIZE; i++)
			{
				tetromino[i].red = 0;
				tetromino[i].green = 255;
				tetromino[i].blue = 255;
				tetromino[i].rect.x = x;
				tetromino[i].rect.y = y;
				tetromino[i].rect.w = 32;
				tetromino[i].rect.h = 32;

				x += 32;
			}
			break;

		// J
		case 1:
			for (int i = 0; i < SIZE; i++)
			{
				tetromino[i].red = 0;
				tetromino[i].green = 0;
				tetromino[i].blue = 255;
				tetromino[i].rect.x = x;
				tetromino[i].rect.y = y;
				tetromino[i].rect.w = 32;
				tetromino[i].rect.h = 32;

				x += 32;
			}

			tetromino[3].rect.x = tetromino[2].rect.x;
			tetromino[3].rect.y = y + 32;

			break;

		// L
		case 2:
			for (int i = 0; i < SIZE; i++)
			{
				tetromino[i].red = 255;
				tetromino[i].green = 140;
				tetromino[i].blue = 0;
				tetromino[i].rect.x = x;
				tetromino[i].rect.y = y;
				tetromino[i].rect.w = 32;
				tetromino[i].rect.h = 32;

				x += 32;
			}

			tetromino[3].rect.x = tetromino[0].rect.x;
			tetromino[3].rect.y = y + 32;

			break;

		// O
		case 3:
			for (int i = 0; i < SIZE; i++)
			{
				tetromino[i].red = 255;
				tetromino[i].green = 255;
				tetromino[i].blue = 0;
				tetromino[i].rect.x = x;
				tetromino[i].rect.y = y;
				tetromino[i].rect.w = 32;
				tetromino[i].rect.h = 32;

				x += 32;
			}

			tetromino[2].rect.x = tetromino[0].rect.x;
			tetromino[2].rect.y = y + 32;
			tetromino[3].rect.x = tetromino[1].rect.x;
			tetromino[3].rect.y = y + 32;

			break;

		// S
		case 4:
			for (int i = 0; i < SIZE; i++)
			{
				tetromino[i].red = 0;
				tetromino[i].green = 255;
				tetromino[i].blue = 0;
				tetromino[i].rect.x = x;
				tetromino[i].rect.y = y;
				tetromino[i].rect.w = 32;
				tetromino[i].rect.h = 32;

				x += 32;
			}

			tetromino[2].rect.x = tetromino[0].rect.x - 32;
			tetromino[2].rect.y = y + 32;
			tetromino[3].rect.x = tetromino[0].rect.x;
			tetromino[3].rect.y = y + 32;

			break;

		// T
		case 5:
			for (int i = 0; i < SIZE; i++)
			{
				tetromino[i].red = 128;
				tetromino[i].green = 0;
				tetromino[i].blue = 128;
				tetromino[i].rect.x = x;
				tetromino[i].rect.y = y;
				tetromino[i].rect.w = 32;
				tetromino[i].rect.h = 32;

				x += 32;
			}

			tetromino[3].rect.x = tetromino[1].rect.x;
			tetromino[3].rect.y = y + 32;

			break;

		// Z
		case 6:
			for (int i = 0; i < SIZE; i++)
			{
				tetromino[i].red = 255;
				tetromino[i].green = 0;
				tetromino[i].blue = 0;
				tetromino[i].rect.x = x;
				tetromino[i].rect.y = y;
				tetromino[i].rect.w = 32;
				tetromino[i].rect.h = 32;

				x += 32;
			}

			tetromino[2].rect.x = tetromino[1].rect.x;
			tetromino[2].rect.y = y + 32;
			tetromino[3].rect.x = tetromino[1].rect.x + 32;
			tetromino[3].rect.y = y + 32;

			break;
	}

	return;
}

/*
 * Update score function
 *
 * Update score
 */
void update_score(void)
{
	char str[8] = { 0 };

	sprintf(str, "%d", score);

	create_text(str, &text_score);

	return;
}

/*
 * Quit function
 *
 * Return back all of the resources used
 */
void quit(void)
{
	if (font)
	{
		TTF_CloseFont(font);
		font = NULL;
	}

	if (text_score)
	{
		SDL_DestroyTexture(text_score);
		text_score = NULL;
	}

	if (text_lbl_score)
	{
		SDL_DestroyTexture(text_lbl_score);
		text_lbl_score = NULL;
	}

	if (text_level)
	{
		SDL_DestroyTexture(text_level);
		text_level = NULL;
	}

	if (text_lbl_level)
	{
		SDL_DestroyTexture(text_lbl_level);
		text_lbl_level = NULL;
	}

	if (renderer)
	{
		SDL_DestroyRenderer(renderer);
		renderer = NULL;
	}

	if (window)
	{
		SDL_DestroyWindow(window);
		window = NULL;
	}

	TTF_Quit();
	SDL_Quit();

	return;
}
