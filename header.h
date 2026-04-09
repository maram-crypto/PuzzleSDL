#ifndef HEADER_H
#define HEADER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* ═══════════════════════════════════════════════════════
   CONSTANTES
═══════════════════════════════════════════════════════ */
#define WIN_W 1000
#define WIN_H 700

#define PUZZLE_W 600
#define PUZZLE_H 400
#define PUZZLE_X ((WIN_W - PUZZLE_W) / 2)
#define PUZZLE_Y 60

#define PIECE_W 140
#define PIECE_H 100
#define PIECE_GAP 25
#define HOLE_MARGIN 30

#define NUM_PUZZLES 3
#define TIMER_TOTAL 20.0f
#define TIMER_CX (WIN_W - 80)
#define TIMER_CY 80
#define TIMER_R 45

/* ═══════════════════════════════════════════════════════
   ENUMS & STRUCTURES
═══════════════════════════════════════════════════════ */
typedef enum {
    STATE_PLAYING,
    STATE_SUCCESS,
    STATE_FAIL
} GameState;

typedef struct {
    SDL_Texture *tex;
    SDL_Rect     origin;  // El blassa el aslya (fixe) besh na3mlou el Floating
    SDL_Rect     dst;     // El blassa elli tet7arek
    int          dragging;
    int          placed;
    int          correct;
    int          dx, dy;  // Offset mtaa el souris
} Piece;

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;

    // Puzzle & Pieces
    SDL_Texture  *puzzle_tex;
    Piece         pieces[3];
    int           correct_idx;
    SDL_Rect      hole_src;
    SDL_Rect      hole_dst;

    // Game Logic
    GameState     state;
    int           puzzle_idx;
    Uint32        start_ticks;
    float         time_left;

    // Starfield Background
    int           stars_x[200];
    int           stars_y[200];
    int           stars_r[200];
    float         stars_speed[200];

    // Animations (Success/Fail)
    Uint32        result_start;
    float         anim_angle;
    float         anim_scale;
    float         anim_alpha;
} Game;

/* ═══════════════════════════════════════════════════════
   PROTOTYPES
═══════════════════════════════════════════════════════ */
int  initialisation(Game *g);
void quitter(Game *g);
int  load_puzzle(Game *g, const char *path);
void generate_pieces(Game *g);
void handle_events(Game *g, SDL_Event *e, int *running);
void update(Game *g);
void render(Game *g);

// Fonctions de dessin internes
void render_starfield(Game *g);
void render_puzzle(Game *g);
void render_hole(Game *g);
void render_pieces(Game *g);
void render_timer(Game *g);
void render_hud(Game *g);
void render_result(Game *g);

#endif
