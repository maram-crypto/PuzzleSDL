#ifndef HEADER_H
#define HEADER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* ─── Window ─────────────────────────────────────────── */
#define WIN_W        1000
#define WIN_H        700
#define FPS          60

/* ─── Puzzle layout ──────────────────────────────────── */
#define PUZZLE_X     80
#define PUZZLE_Y     80
#define PUZZLE_W     500
#define PUZZLE_H     400

#define PIECE_W      150
#define PIECE_H      120
#define PIECE_GAP    30

#define HOLE_MARGIN  20

/* ─── Timer ──────────────────────────────────────────── */
#define TIMER_TOTAL  45
#define TIMER_CX     860
#define TIMER_CY     200
#define TIMER_R      70

/* ─── Puzzle count ───────────────────────────────────── */
#define NUM_PUZZLES  5

/* ─── Game states ────────────────────────────────────── */
typedef enum
{
    STATE_PLAYING,
    STATE_SUCCESS,
    STATE_FAIL
} GameState;

/* ─── One selectable piece ───────────────────────────── */
typedef struct
{
    SDL_Texture *tex;
    SDL_Rect     dst;
    SDL_Rect     origin;
    int          correct;
    int          dragging;
    int          placed;
    int          dx;
    int          dy;
} Piece;

/* ─── Full game context ──────────────────────────────── */
typedef struct
{
    SDL_Window   *window;
    SDL_Renderer *renderer;

    SDL_Texture  *puzzle_tex;
    SDL_Rect      hole_src;
    SDL_Rect      hole_dst;

    Piece         pieces[3];
    int           correct_idx;

    Uint32        start_ticks;
    float         time_left;

    GameState     state;

    float         anim_angle;
    float         anim_scale;
    float         anim_alpha;
    Uint32        result_start;

    int           stars_x[200];
    int           stars_y[200];
    int           stars_r[200];
    float         stars_speed[200];

    int           puzzle_idx;
} Game;

/* ─── Prototypes ─────────────────────────────────────── */
int          initialisation      (Game *g);
void         quitter             (Game *g);
int          load_puzzle         (Game *g, const char *path);
void         generate_pieces     (Game *g);
SDL_Texture *create_false_piece  (Game *g, SDL_Texture *src, SDL_Rect region);
void         handle_events       (Game *g, SDL_Event *e, int *running);
void         update              (Game *g);
void         render              (Game *g);
void         render_starfield    (Game *g);
void         render_puzzle       (Game *g);
void         render_hole         (Game *g);
void         render_pieces       (Game *g);
void         render_timer        (Game *g);
void         render_result       (Game *g);
void         render_hud          (Game *g);

#endif /* HEADER_H */
