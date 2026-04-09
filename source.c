#include "header.h"

/* ═══════════════════════════════════════════════════════
    INITIALISATION
═══════════════════════════════════════════════════════ */
int initialisation(Game *g)
{
    int i;

    memset(g, 0, sizeof(*g));
    srand((unsigned)time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 0;
    }
    if (!(IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) & (IMG_INIT_JPG | IMG_INIT_PNG)))
    {
        fprintf(stderr, "IMG_Init: %s\n", IMG_GetError());
        return 0;
    }
    if (TTF_Init() < 0)
    {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        return 0;
    }

    g->window = SDL_CreateWindow(
        "INTERSTELLAR  -  Final Edition",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H, SDL_WINDOW_SHOWN
    );
    if (!g->window) return 0;

    g->renderer = SDL_CreateRenderer(g->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g->renderer) return 0;

    SDL_SetRenderDrawBlendMode(g->renderer, SDL_BLENDMODE_BLEND);

    i = 0;
    while (i < 200)
    {
        g->stars_x[i]     = rand() % WIN_W;
        g->stars_y[i]     = rand() % WIN_H;
        g->stars_r[i]     = (rand() % 3) + 1;
        g->stars_speed[i] = 0.1f + (rand() % 10) * 0.05f;
        i++;
    }

    return 1;
}

/* ═══════════════════════════════════════════════════════
    QUITTER
═══════════════════════════════════════════════════════ */
void quitter(Game *g)
{
    int i;
    i = 0;
    while (i < 3)
    {
        if (g->pieces[i].tex)
            SDL_DestroyTexture(g->pieces[i].tex);
        i++;
    }
    if (g->puzzle_tex) SDL_DestroyTexture(g->puzzle_tex);
    if (g->renderer) SDL_DestroyRenderer(g->renderer);
    if (g->window) SDL_DestroyWindow(g->window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

/* ═══════════════════════════════════════════════════════
    LOAD PUZZLE
═══════════════════════════════════════════════════════ */
int load_puzzle(Game *g, const char *path)
{
    SDL_Surface *surf;
    SDL_Surface *scaled;
    int i;

    if (g->puzzle_tex)
    {
        SDL_DestroyTexture(g->puzzle_tex);
        g->puzzle_tex = NULL;
    }

    i = 0;
    while (i < 3)
    {
        if (g->pieces[i].tex)
        {
            SDL_DestroyTexture(g->pieces[i].tex);
            g->pieces[i].tex = NULL;
        }
        i++;
    }

    surf = IMG_Load(path);
    if (!surf) return 0;

    scaled = SDL_CreateRGBSurface(0, PUZZLE_W, PUZZLE_H, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_BlitScaled(surf, NULL, scaled, NULL);
    SDL_FreeSurface(surf);

    g->puzzle_tex = SDL_CreateTextureFromSurface(g->renderer, scaled);
    SDL_FreeSurface(scaled);

    return (g->puzzle_tex != NULL);
}

/* ═══════════════════════════════════════════════════════
    CREATE FALSE PIECE
═══════════════════════════════════════════════════════ */
SDL_Texture *create_false_piece(Game *g, SDL_Texture *src, SDL_Rect region)
{
    SDL_Texture *target;
    SDL_Texture *piece;
    SDL_Rect alt;
    SDL_Rect dst;

    target = SDL_CreateTexture(g->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, PUZZLE_W, PUZZLE_H);
    SDL_SetRenderTarget(g->renderer, target);
    SDL_RenderCopy(g->renderer, src, NULL, NULL);
    SDL_SetRenderTarget(g->renderer, NULL);

    piece = SDL_CreateTexture(g->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, PIECE_W, PIECE_H);
    SDL_SetRenderTarget(g->renderer, piece);
    
    alt = region;
    alt.x = rand() % (PUZZLE_W - PIECE_W);
    alt.y = rand() % (PUZZLE_H - PIECE_H);

    while (abs(alt.x - region.x) < PIECE_W / 2 && abs(alt.y - region.y) < PIECE_H / 2)
    {
        alt.x = rand() % (PUZZLE_W - PIECE_W);
        alt.y = rand() % (PUZZLE_H - PIECE_H);
    }

    dst.x = 0; dst.y = 0; dst.w = PIECE_W; dst.h = PIECE_H;
    SDL_RenderCopy(g->renderer, target, &alt, &dst);

    SDL_SetRenderDrawColor(g->renderer, 180, 30, 30, 40);
    SDL_RenderFillRect(g->renderer, NULL);

    SDL_SetRenderTarget(g->renderer, NULL);
    SDL_DestroyTexture(target);
    return piece;
}

/* ═══════════════════════════════════════════════════════
    GENERATE PIECES
═══════════════════════════════════════════════════════ */
void generate_pieces(Game *g)
{
    SDL_Texture *correct_tex;
    int hx, hy, total_w, start_x, piece_y, i;

    hx = HOLE_MARGIN + rand() % (PUZZLE_W - PIECE_W - HOLE_MARGIN * 2);
    hy = HOLE_MARGIN + rand() % (PUZZLE_H - PIECE_H - HOLE_MARGIN * 2);

    g->hole_src = (SDL_Rect){hx, hy, PIECE_W, PIECE_H};
    g->hole_dst = (SDL_Rect){PUZZLE_X + hx, PUZZLE_Y + hy, PIECE_W, PIECE_H};

    correct_tex = SDL_CreateTexture(g->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, PIECE_W, PIECE_H);
    SDL_SetRenderTarget(g->renderer, correct_tex);
    SDL_RenderCopy(g->renderer, g->puzzle_tex, &g->hole_src, NULL);
    SDL_SetRenderTarget(g->renderer, NULL);

    g->correct_idx = rand() % 3;
    total_w = 3 * PIECE_W + 2 * PIECE_GAP;
    start_x = (WIN_W - total_w) / 2;
    piece_y = PUZZLE_Y + PUZZLE_H + 40;

    i = 0;
    while (i < 3)
    {
        g->pieces[i].dragging = 0;
        g->pieces[i].placed = 0;
        g->pieces[i].origin = (SDL_Rect){start_x + i * (PIECE_W + PIECE_GAP), piece_y, PIECE_W, PIECE_H};
        g->pieces[i].dst = g->pieces[i].origin;

        if (i == g->correct_idx)
        {
            g->pieces[i].tex = correct_tex;
            g->pieces[i].correct = 1;
        }
        else
        {
            g->pieces[i].tex = create_false_piece(g, g->puzzle_tex, g->hole_src);
            g->pieces[i].correct = 0;
        }
        i++;
    }
}

/* ═══════════════════════════════════════════════════════
    HANDLE EVENTS
═══════════════════════════════════════════════════════ */
void handle_events(Game *g, SDL_Event *e, int *running)
{
    char path[64];
    int mx, my, cx, cy, hcx, hcy, i;
    Piece *p;

    while (SDL_PollEvent(e))
    {
        if (e->type == SDL_QUIT) { *running = 0; return; }
        
        if (g->state != STATE_PLAYING)
        {
            if (e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_KEYDOWN)
            {
                g->puzzle_idx = (g->puzzle_idx + 1) % NUM_PUZZLES;
                g->state = STATE_PLAYING;
                g->start_ticks = SDL_GetTicks();
                g->time_left = TIMER_TOTAL;
                snprintf(path, sizeof(path), "assets/puzzle%d.jpg", g->puzzle_idx + 1);
                if (!load_puzzle(g, path)) *running = 0;
                else generate_pieces(g);
            }
            return;
        }

        if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT)
        {
            mx = e->button.x; my = e->button.y;
            i = 2;
            while (i >= 0)
            {
                p = &g->pieces[i];
                if (!p->placed && mx >= p->dst.x && mx < p->dst.x + PIECE_W && my >= p->dst.y && my < p->dst.y + PIECE_H)
                {
                    p->dragging = 1;
                    p->dx = mx - p->dst.x; p->dy = my - p->dst.y;
                    break;
                }
                i--;
            }
        }

        if (e->type == SDL_MOUSEMOTION)
        {
            i = 0;
            while (i < 3)
            {
                if (g->pieces[i].dragging)
                {
                    g->pieces[i].dst.x = e->motion.x - g->pieces[i].dx;
                    g->pieces[i].dst.y = e->motion.y - g->pieces[i].dy;
                }
                i++;
            }
        }

        if (e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT)
        {
            i = 0;
            while (i < 3)
            {
                p = &g->pieces[i];
                if (p->dragging)
                {
                    p->dragging = 0;
                    cx = p->dst.x + PIECE_W / 2; cy = p->dst.y + PIECE_H / 2;
                    hcx = g->hole_dst.x + PIECE_W / 2; hcy = g->hole_dst.y + PIECE_H / 2;

                    if (abs(cx - hcx) < PIECE_W / 2 && abs(cy - hcy) < PIECE_H / 2)
                    {
                        p->dst = g->hole_dst; p->placed = 1;
                        g->state = p->correct ? STATE_SUCCESS : STATE_FAIL;
                        g->result_start = SDL_GetTicks();
                    }
                    else p->dst = p->origin;
                }
                i++;
            }
        }
    }
}

/* ═══════════════════════════════════════════════════════
    UPDATE (Floating Effect Included)
═══════════════════════════════════════════════════════ */
void update(Game *g)
{
    float floating, t;
    int i;

    if (g->state == STATE_PLAYING)
    {
        g->time_left = TIMER_TOTAL - (SDL_GetTicks() - g->start_ticks) / 1000.0f;
        
        // Animation Floating
        floating = sin(SDL_GetTicks() / 400.0f) * 8.0f;
        i = 0;
        while (i < 3)
        {
            if (!g->pieces[i].dragging && !g->pieces[i].placed)
                g->pieces[i].dst.y = g->pieces[i].origin.y + (int)floating;
            i++;
        }

        if (g->time_left <= 0.0f) { g->time_left = 0; g->state = STATE_FAIL; g->result_start = SDL_GetTicks(); }
    }

    i = 0;
    while (i < 200)
    {
        g->stars_y[i] += (int)g->stars_speed[i];
        if (g->stars_y[i] > WIN_H) g->stars_y[i] = 0;
        i++;
    }

    if (g->state != STATE_PLAYING)
    {
        t = (SDL_GetTicks() - g->result_start) / 1000.0f;
        g->anim_scale = (t > 1.0f) ? 1.0f : 0.3f + t * 0.7f;
        g->anim_alpha = (t > 1.0f) ? 1.0f : t;
    }
}

/* ═══════════════════════════════════════════════════════
    RENDER SUB-FUNCTIONS (Starfield, Puzzle, Hole, Pieces)
═══════════════════════════════════════════════════════ */
void render_starfield(Game *g)
{
    int i, b;
    SDL_Rect sr;
    i = 0;
    while (i < 200)
    {
        b = 120 + g->stars_r[i] * 40;
        SDL_SetRenderDrawColor(g->renderer, b, b, b + 30, 200);
        sr = (SDL_Rect){g->stars_x[i], g->stars_y[i], g->stars_r[i], g->stars_r[i]};
        SDL_RenderFillRect(g->renderer, &sr);
        i++;
    }
}

void render_puzzle(Game *g)
{
    SDL_Rect dst = {PUZZLE_X, PUZZLE_Y, PUZZLE_W, PUZZLE_H};
    SDL_RenderCopy(g->renderer, g->puzzle_tex, NULL, &dst);
}

void render_hole(Game *g)
{
    int i, alpha;
    i = 0;
    while(i < 3) { if(g->pieces[i].placed && g->pieces[i].correct) return; i++; }
    alpha = 160 + (int)(50 * sin(SDL_GetTicks() / 400.0));
    SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, alpha);
    SDL_RenderFillRect(g->renderer, &g->hole_dst);
}

void render_pieces(Game *g)
{
    int i;
    i = 0;
    while (i < 3)
    {
        if (!g->pieces[i].placed) SDL_RenderCopy(g->renderer, g->pieces[i].tex, NULL, &g->pieces[i].dst);
        i++;
    }
}

/* ═══════════════════════════════════════════════════════
    RENDER TIMER (Vibration Effect Included)
═══════════════════════════════════════════════════════ */
void render_timer(Game *g)
{
    int vx = 0, vy = 0, a, r, x, y;
    float ratio = g->time_left / (float)TIMER_TOTAL;
    float rad, start_rad = -(float)M_PI / 2.0f;

    if (g->time_left < 5.0f && g->state == STATE_PLAYING)
    {
        vx = (int)(sin(SDL_GetTicks() * 0.06f) * 3.0f);
        vy = (int)(cos(SDL_GetTicks() * 0.06f) * 3.0f);
    }

    a = 0;
    while (a < (int)(360 * ratio))
    {
        rad = start_rad + a * 2.0f * (float)M_PI / 360.0f;
        r = TIMER_R - 8;
        while (r <= TIMER_R)
        {
            x = TIMER_CX + vx + (int)(r * cos(rad));
            y = TIMER_CY + vy + (int)(r * sin(rad));
            SDL_SetRenderDrawColor(g->renderer, (int)(255*(1-ratio)), (int)(255*ratio), 100, 255);
            SDL_RenderDrawPoint(g->renderer, x, y);
            r++;
        }
        a++;
    }
}

/* ═══════════════════════════════════════════════════════
    RENDER HUD & RESULT
═══════════════════════════════════════════════════════ */
void render_hud(Game *g)
{
    SDL_Rect bar = {PUZZLE_X, PUZZLE_Y + PUZZLE_H + 12, PUZZLE_W, 22};
    SDL_SetRenderDrawColor(g->renderer, 60, 60, 100, 120);
    SDL_RenderFillRect(g->renderer, &bar);
}

void render_result(Game *g)
{
    if (g->state == STATE_PLAYING) return;
    SDL_Rect res = {(WIN_W - 400) / 2, (WIN_H - 150) / 2, 400, 150};
    SDL_SetRenderDrawColor(g->renderer, g->state == STATE_SUCCESS ? 0 : 200, g->state == STATE_SUCCESS ? 200 : 0, 0, (int)(g->anim_alpha * 200));
    SDL_RenderFillRect(g->renderer, &res);
}

void render(Game *g)
{
    SDL_SetRenderDrawColor(g->renderer, 3, 3, 12, 255);
    SDL_RenderClear(g->renderer);
    render_starfield(g);
    render_puzzle(g);
    render_hole(g);
    render_pieces(g);
    render_timer(g);
    render_hud(g);
    render_result(g);
    SDL_RenderPresent(g->renderer);
}
