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
        "INTERSTELLAR  -  Puzzle",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H, SDL_WINDOW_SHOWN
    );
    if (!g->window)
    {
        fprintf(stderr, "Window: %s\n", SDL_GetError());
        return 0;
    }

    g->renderer = SDL_CreateRenderer(g->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g->renderer)
    {
        fprintf(stderr, "Renderer: %s\n", SDL_GetError());
        return 0;
    }

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
    if (g->puzzle_tex)
        SDL_DestroyTexture(g->puzzle_tex);
    if (g->renderer)
        SDL_DestroyRenderer(g->renderer);
    if (g->window)
        SDL_DestroyWindow(g->window);
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
    int          i;

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
    if (!surf)
    {
        fprintf(stderr, "IMG_Load(%s): %s\n", path, IMG_GetError());
        return 0;
    }

    scaled = SDL_CreateRGBSurface(0, PUZZLE_W, PUZZLE_H, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_BlitScaled(surf, NULL, scaled, NULL);
    SDL_FreeSurface(surf);

    g->puzzle_tex = SDL_CreateTextureFromSurface(g->renderer, scaled);
    SDL_FreeSurface(scaled);

    if (!g->puzzle_tex)
    {
        fprintf(stderr, "Texture: %s\n", SDL_GetError());
        return 0;
    }
    return 1;
}

/* ═══════════════════════════════════════════════════════
   CREATE FALSE PIECE
═══════════════════════════════════════════════════════ */
SDL_Texture *create_false_piece(Game *g, SDL_Texture *src, SDL_Rect region)
{
    SDL_Texture *target;
    SDL_Texture *piece;
    SDL_Rect     alt;
    SDL_Rect     dst;

    target = SDL_CreateTexture(g->renderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
        PUZZLE_W, PUZZLE_H);
    SDL_SetRenderTarget(g->renderer, target);
    SDL_RenderCopy(g->renderer, src, NULL, NULL);
    SDL_SetRenderTarget(g->renderer, NULL);

    piece = SDL_CreateTexture(g->renderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
        PIECE_W, PIECE_H);
    SDL_SetRenderTarget(g->renderer, piece);
    SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 255);
    SDL_RenderClear(g->renderer);

    alt   = region;
    alt.x = rand() % (PUZZLE_W - PIECE_W);
    alt.y = rand() % (PUZZLE_H - PIECE_H);

    while (abs(alt.x - region.x) < PIECE_W / 2 && abs(alt.y - region.y) < PIECE_H / 2)
    {
        alt.x = rand() % (PUZZLE_W - PIECE_W);
        alt.y = rand() % (PUZZLE_H - PIECE_H);
    }

    dst.x = 0;
    dst.y = 0;
    dst.w = PIECE_W;
    dst.h = PIECE_H;
    SDL_RenderCopy(g->renderer, target, &alt, &dst);

    SDL_SetRenderDrawBlendMode(g->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g->renderer, 180, 30, 30, 30);
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
    int          hx;
    int          hy;
    int          total_w;
    int          start_x;
    int          piece_y;
    int          i;

    hx = HOLE_MARGIN + rand() % (PUZZLE_W - PIECE_W - HOLE_MARGIN * 2);
    hy = HOLE_MARGIN + rand() % (PUZZLE_H - PIECE_H - HOLE_MARGIN * 2);

    g->hole_src.x = hx;
    g->hole_src.y = hy;
    g->hole_src.w = PIECE_W;
    g->hole_src.h = PIECE_H;

    g->hole_dst.x = PUZZLE_X + hx;
    g->hole_dst.y = PUZZLE_Y + hy;
    g->hole_dst.w = PIECE_W;
    g->hole_dst.h = PIECE_H;

    correct_tex = SDL_CreateTexture(g->renderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, PIECE_W, PIECE_H);
    SDL_SetRenderTarget(g->renderer, correct_tex);
    SDL_RenderCopy(g->renderer, g->puzzle_tex, &g->hole_src, NULL);
    SDL_SetRenderTarget(g->renderer, NULL);

    g->correct_idx = rand() % 3;
    total_w        = 3 * PIECE_W + 2 * PIECE_GAP;
    start_x        = (WIN_W - total_w) / 2;
    piece_y        = PUZZLE_Y + PUZZLE_H + 40;

    i = 0;
    while (i < 3)
    {
        g->pieces[i].dragging  = 0;
        g->pieces[i].placed    = 0;
        g->pieces[i].origin.x  = start_x + i * (PIECE_W + PIECE_GAP);
        g->pieces[i].origin.y  = piece_y;
        g->pieces[i].origin.w  = PIECE_W;
        g->pieces[i].origin.h  = PIECE_H;
        g->pieces[i].dst       = g->pieces[i].origin;

        if (i == g->correct_idx)
        {
            g->pieces[i].tex     = correct_tex;
            g->pieces[i].correct = 1;
        }
        else
        {
            g->pieces[i].tex     = create_false_piece(g, g->puzzle_tex, g->hole_src);
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
    char   path[64];
    int    mx;
    int    my;
    int    cx;
    int    cy;
    int    hcx;
    int    hcy;
    int    i;
    Piece *p;

    while (SDL_PollEvent(e))
    {
        if (e->type == SDL_QUIT)
        {
            *running = 0;
            return;
        }
        if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_ESCAPE)
        {
            *running = 0;
            return;
        }

        if (g->state != STATE_PLAYING)
        {
            if (e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_KEYDOWN)
            {
                g->puzzle_idx  = (g->puzzle_idx + 1) % NUM_PUZZLES;
                g->state       = STATE_PLAYING;
                g->start_ticks = SDL_GetTicks();
                g->time_left   = TIMER_TOTAL;
                g->anim_angle  = 0;
                g->anim_scale  = 0;
                g->anim_alpha  = 0;

                snprintf(path, sizeof(path), "assets/puzzle%d.jpg", g->puzzle_idx + 1);
                if (!load_puzzle(g, path))
                    *running = 0;
                else
                    generate_pieces(g);
            }
            return;
        }

        if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT)
        {
            mx = e->button.x;
            my = e->button.y;
            i  = 2;
            while (i >= 0)
            {
                p = &g->pieces[i];
                if (!p->placed
                    && mx >= p->dst.x && mx < p->dst.x + PIECE_W
                    && my >= p->dst.y && my < p->dst.y + PIECE_H)
                {
                    p->dragging = 1;
                    p->dx       = mx - p->dst.x;
                    p->dy       = my - p->dst.y;
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
                    cx  = p->dst.x + PIECE_W / 2;
                    cy  = p->dst.y + PIECE_H / 2;
                    hcx = g->hole_dst.x + PIECE_W / 2;
                    hcy = g->hole_dst.y + PIECE_H / 2;

                    if (abs(cx - hcx) < PIECE_W / 2 && abs(cy - hcy) < PIECE_H / 2)
                    {
                        p->dst    = g->hole_dst;
                        p->placed = 1;
                        g->state        = p->correct ? STATE_SUCCESS : STATE_FAIL;
                        g->result_start = SDL_GetTicks();
                        g->anim_scale   = 0.1f;
                        g->anim_alpha   = 0.0f;
                    }
                    else
                    {
                        p->dst = p->origin;
                    }
                }
                i++;
            }
        }
    }
}

/* ═══════════════════════════════════════════════════════
   UPDATE
═══════════════════════════════════════════════════════ */
void update(Game *g)
{
    float elapsed;
    int   i;

    if (g->state == STATE_PLAYING)
    {
        elapsed      = (SDL_GetTicks() - g->start_ticks) / 1000.0f;
        g->time_left = TIMER_TOTAL - elapsed;
        if (g->time_left <= 0.0f)
        {
            g->time_left    = 0.0f;
            g->state        = STATE_FAIL;
            g->result_start = SDL_GetTicks();
            g->anim_scale   = 0.1f;
            g->anim_alpha   = 0.0f;
        }
    }

    i = 0;
    while (i < 200)
    {
        g->stars_y[i] += (int)(g->stars_speed[i]);
        if (g->stars_y[i] > WIN_H)
            g->stars_y[i] = 0;
        i++;
    }

    if (g->state != STATE_PLAYING)
    {
        float t;
        t              = (SDL_GetTicks() - g->result_start) / 1000.0f;
        g->anim_angle  = t * 90.0f;
        g->anim_scale  = 0.3f + t * 0.7f;
        if (g->anim_scale > 1.0f)
            g->anim_scale = 1.0f;
        g->anim_alpha = t * 2.0f;
        if (g->anim_alpha > 1.0f)
            g->anim_alpha = 1.0f;
    }
}

/* ═══════════════════════════════════════════════════════
   RENDER STARFIELD
═══════════════════════════════════════════════════════ */
void render_starfield(Game *g)
{
    int      i;
    int      brightness;
    SDL_Rect sr;

    i = 0;
    while (i < 200)
    {
        brightness = 120 + g->stars_r[i] * 40;
        SDL_SetRenderDrawColor(g->renderer,
            brightness, brightness, brightness + 30, 200);
        sr.x = g->stars_x[i];
        sr.y = g->stars_y[i];
        sr.w = g->stars_r[i];
        sr.h = g->stars_r[i];
        SDL_RenderFillRect(g->renderer, &sr);
        i++;
    }
}

/* ═══════════════════════════════════════════════════════
   RENDER PUZZLE
═══════════════════════════════════════════════════════ */
void render_puzzle(Game *g)
{
    SDL_Rect dst;
    SDL_Rect border;
    int      t;

    t = 6;
    while (t >= 0)
    {
        SDL_SetRenderDrawColor(g->renderer, 80, 140, 255, 15 + t * 8);
        border.x = PUZZLE_X - t;
        border.y = PUZZLE_Y - t;
        border.w = PUZZLE_W + t * 2;
        border.h = PUZZLE_H + t * 2;
        SDL_RenderDrawRect(g->renderer, &border);
        t--;
    }

    dst.x = PUZZLE_X;
    dst.y = PUZZLE_Y;
    dst.w = PUZZLE_W;
    dst.h = PUZZLE_H;
    SDL_RenderCopy(g->renderer, g->puzzle_tex, NULL, &dst);
}

/* ═══════════════════════════════════════════════════════
   RENDER HOLE
═══════════════════════════════════════════════════════ */
void render_hole(Game *g)
{
    Uint32   ticks;
    int      alpha;
    int      i;
    int      x;
    int      y;

    i = 0;
    while (i < 3)
    {
        if (g->pieces[i].placed && g->pieces[i].correct)
            return;
        i++;
    }

    ticks = SDL_GetTicks();
    alpha = 160 + (int)(50 * sin(ticks / 400.0));

    SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, alpha);
    SDL_RenderFillRect(g->renderer, &g->hole_dst);

    SDL_SetRenderDrawColor(g->renderer, 255, 200, 50, 200);

    x = g->hole_dst.x;
    while (x < g->hole_dst.x + g->hole_dst.w)
    {
        SDL_RenderDrawPoint(g->renderer, x, g->hole_dst.y);
        SDL_RenderDrawPoint(g->renderer, x, g->hole_dst.y + g->hole_dst.h - 1);
        x += 8;
    }
    y = g->hole_dst.y;
    while (y < g->hole_dst.y + g->hole_dst.h)
    {
        SDL_RenderDrawPoint(g->renderer, g->hole_dst.x, y);
        SDL_RenderDrawPoint(g->renderer, g->hole_dst.x + g->hole_dst.w - 1, y);
        y += 8;
    }
}

/* ═══════════════════════════════════════════════════════
   RENDER PIECES
═══════════════════════════════════════════════════════ */
void render_pieces(Game *g)
{
    Piece   *p;
    SDL_Rect shadow;
    SDL_Rect glow;
    int      i;
    int      t;

    i = 0;
    while (i < 3)
    {
        p = &g->pieces[i];
        if (!p->placed)
        {
            shadow.x = p->dst.x + 6;
            shadow.y = p->dst.y + 6;
            shadow.w = PIECE_W;
            shadow.h = PIECE_H;
            SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 80);
            SDL_RenderFillRect(g->renderer, &shadow);

            SDL_RenderCopy(g->renderer, p->tex, NULL, &p->dst);

            if (p->dragging)
            {
                t = 0;
                while (t < 5)
                {
                    SDL_SetRenderDrawColor(g->renderer, 100, 180, 255, 30 - t * 5);
                    glow.x = p->dst.x - t;
                    glow.y = p->dst.y - t;
                    glow.w = PIECE_W + t * 2;
                    glow.h = PIECE_H + t * 2;
                    SDL_RenderDrawRect(g->renderer, &glow);
                    t++;
                }
            }
            else
            {
                SDL_SetRenderDrawColor(g->renderer, 200, 200, 200, 160);
                SDL_RenderDrawRect(g->renderer, &p->dst);
            }
        }
        i++;
    }
}

/* ═══════════════════════════════════════════════════════
   RENDER TIMER
═══════════════════════════════════════════════════════ */
void render_timer(Game *g)
{
    float    ratio;
    float    start_rad;
    float    tip_rad;
    float    pulse;
    float    rad;
    float    orb_rad;
    int      arcR;
    int      arcG;
    int      segments;
    int      ir;
    int      tx;
    int      ty;
    int      ox;
    int      oy;
    int      a;
    int      r;
    int      x;
    int      y;
    int      bright;
    Uint32   tick;
    SDL_Rect dot;
    SDL_Rect orb;

    tick      = SDL_GetTicks();
    ratio     = g->time_left / (float)TIMER_TOTAL;
    start_rad = -(float)M_PI / 2.0f;
    arcR      = (int)(255 * (1.0f - ratio));
    arcG      = (int)(200 * ratio);
    segments  = (int)(360 * ratio);

    a = 0;
    while (a < 360)
    {
        rad    = a * (float)M_PI / 180.0f;
        bright = (a % 20 == 0) ? 255 : 80;
        x      = TIMER_CX + (int)((TIMER_R + 10) * cos(rad));
        y      = TIMER_CY + (int)((TIMER_R + 10) * sin(rad));
        SDL_SetRenderDrawColor(g->renderer, bright, bright, bright + 40, 120);
        SDL_RenderDrawPoint(g->renderer, x, y);
        a += 4;
    }

    a = 0;
    while (a < 360)
    {
        rad = a * (float)M_PI / 180.0f;
        r   = 0;
        while (r < TIMER_R)
        {
            x = TIMER_CX + (int)(r * cos(rad));
            y = TIMER_CY + (int)(r * sin(rad));
            SDL_SetRenderDrawColor(g->renderer, 30, 30, 60, 200);
            SDL_RenderDrawPoint(g->renderer, x, y);
            r++;
        }
        a++;
    }

    a = 0;
    while (a < segments)
    {
        rad = start_rad + a * 2.0f * (float)M_PI / 360.0f;
        r   = TIMER_R - 12;
        while (r <= TIMER_R)
        {
            x = TIMER_CX + (int)(r * cos(rad));
            y = TIMER_CY + (int)(r * sin(rad));
            SDL_SetRenderDrawColor(g->renderer, arcR, arcG, 255 - arcR, 240);
            SDL_RenderDrawPoint(g->renderer, x, y);
            r++;
        }
        a++;
    }

    tip_rad = start_rad + segments * 2.0f * (float)M_PI / 360.0f;
    tx      = TIMER_CX + (int)(TIMER_R * cos(tip_rad));
    ty      = TIMER_CY + (int)(TIMER_R * sin(tip_rad));
    SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 255);
    dot.x = tx - 4;
    dot.y = ty - 4;
    dot.w = 8;
    dot.h = 8;
    SDL_RenderFillRect(g->renderer, &dot);

    pulse = 0.85f + 0.15f * (float)sin(tick / 200.0f);
    ir    = (int)(TIMER_R * 0.55f * pulse);
    a     = 0;
    while (a < 360)
    {
        rad = a * (float)M_PI / 180.0f;
        r   = ir - 2;
        while (r <= ir)
        {
            x = TIMER_CX + (int)(r * cos(rad));
            y = TIMER_CY + (int)(r * sin(rad));
            SDL_SetRenderDrawColor(g->renderer, arcR, arcG + 50, 200, 180);
            SDL_RenderDrawPoint(g->renderer, x, y);
            r++;
        }
        a++;
    }

    orb_rad = start_rad + (tick / 800.0f);
    ox      = TIMER_CX + (int)((TIMER_R - 6) * cos(orb_rad));
    oy      = TIMER_CY + (int)((TIMER_R - 6) * sin(orb_rad));
    SDL_SetRenderDrawColor(g->renderer, 255, 220, 100, 220);
    orb.x = ox - 3;
    orb.y = oy - 3;
    orb.w = 6;
    orb.h = 6;
    SDL_RenderFillRect(g->renderer, &orb);
}

/* ═══════════════════════════════════════════════════════
   RENDER HUD
═══════════════════════════════════════════════════════ */
void render_hud(Game *g)
{
    int      dot_r;
    int      gap;
    int      total_w;
    int      sx;
    int      sy;
    int      i;
    int      cx;
    SDL_Rect dr;
    SDL_Rect bar;
    SDL_Rect tlabel;

    dot_r   = 8;
    gap     = 24;
    total_w = NUM_PUZZLES * (dot_r * 2 + gap) - gap;
    sx      = (WIN_W - total_w) / 2;
    sy      = WIN_H - 28;

    i = 0;
    while (i < NUM_PUZZLES)
    {
        cx   = sx + i * (dot_r * 2 + gap) + dot_r;
        dr.x = cx - dot_r;
        dr.y = sy - dot_r;
        dr.w = dot_r * 2;
        dr.h = dot_r * 2;
        if (i == g->puzzle_idx)
        {
            SDL_SetRenderDrawColor(g->renderer, 100, 180, 255, 255);
            SDL_RenderFillRect(g->renderer, &dr);
        }
        else
        {
            SDL_SetRenderDrawColor(g->renderer, 80, 80, 120, 180);
            SDL_RenderDrawRect(g->renderer, &dr);
        }
        i++;
    }

    bar.x = PUZZLE_X;
    bar.y = PUZZLE_Y + PUZZLE_H + 12;
    bar.w = PUZZLE_W;
    bar.h = 22;
    SDL_SetRenderDrawColor(g->renderer, 60, 60, 100, 120);
    SDL_RenderFillRect(g->renderer, &bar);
    SDL_SetRenderDrawColor(g->renderer, 100, 140, 255, 180);
    SDL_RenderDrawRect(g->renderer, &bar);

    tlabel.x = TIMER_CX - 30;
    tlabel.y = TIMER_CY + TIMER_R + 16;
    tlabel.w = 60;
    tlabel.h = 4;
    SDL_SetRenderDrawColor(g->renderer, 100, 160, 255, 200);
    SDL_RenderFillRect(g->renderer, &tlabel);
}

/* ═══════════════════════════════════════════════════════
   RENDER RESULT
═══════════════════════════════════════════════════════ */
void render_result(Game *g)
{
    int      alpha;
    float    scale;
    float    shimmer;
    float    shake;
    float    ang;
    float    dist;
    int      bw;
    int      bh;
    int      bx;
    int      by;
    int      cx;
    int      cy;
    int      sz;
    int      sx2;
    int      sy2;
    int      sr;
    int      t;
    int      i;
    Uint32   tick;
    SDL_Rect full;
    SDL_Rect glow;
    SDL_Rect banner;
    SDL_Rect star;
    SDL_Rect cont;
    SDL_Rect shake_r;

    if (g->state == STATE_PLAYING)
        return;

    tick    = SDL_GetTicks();
    alpha   = (int)(g->anim_alpha * 220);
    scale   = g->anim_scale;
    shimmer = (float)sin(tick / 150.0f) * 6.0f * scale;

    bw = (int)(560 * scale);
    bh = (int)(180 * scale);
    bx = (WIN_W - bw) / 2;
    by = (WIN_H - bh) / 2;

    full.x = 0;
    full.y = 0;
    full.w = WIN_W;
    full.h = WIN_H;
    SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, alpha / 2);
    SDL_RenderFillRect(g->renderer, &full);

    if (g->state == STATE_SUCCESS)
    {
        t = 8;
        while (t >= 0)
        {
            SDL_SetRenderDrawColor(g->renderer, 255, 200, 50, (alpha * (8 - t)) / 16);
            glow.x = bx - t + (int)shimmer;
            glow.y = by - t;
            glow.w = bw + t * 2;
            glow.h = bh + t * 2;
            SDL_RenderFillRect(g->renderer, &glow);
            t--;
        }

        banner.x = bx + (int)shimmer;
        banner.y = by;
        banner.w = bw;
        banner.h = bh;
        SDL_SetRenderDrawColor(g->renderer, 30, 20, 0, alpha);
        SDL_RenderFillRect(g->renderer, &banner);
        SDL_SetRenderDrawColor(g->renderer, 255, 215, 0, alpha);
        SDL_RenderDrawRect(g->renderer, &banner);

        cx = bx + bw / 2 + (int)shimmer;
        cy = by + bh / 2;
        sz = (int)(30 * scale);
        SDL_SetRenderDrawColor(g->renderer, 255, 215, 0, alpha);
        i = 0;
        while (i < sz / 2)
        {
            SDL_RenderDrawPoint(g->renderer, cx - sz / 2 + i, cy + i);
            i++;
        }
        i = 0;
        while (i < sz)
        {
            SDL_RenderDrawPoint(g->renderer, cx + i, cy + sz / 2 - i);
            i++;
        }

        i = 0;
        while (i < 12)
        {
            ang  = i * (float)M_PI / 6.0f + tick / 500.0f;
            dist = 120.0f * scale;
            sx2  = bx + bw / 2 + (int)(dist * cos(ang));
            sy2  = by + bh / 2 + (int)(dist * 0.5f * sin(ang));
            sr   = (int)(6 * scale);
            SDL_SetRenderDrawColor(g->renderer, 255, 255, 100, (alpha * 3) / 4);
            star.x = sx2 - sr;
            star.y = sy2 - sr;
            star.w = sr * 2;
            star.h = sr * 2;
            SDL_RenderFillRect(g->renderer, &star);
            i++;
        }
    }
    else
    {
        t = 8;
        while (t >= 0)
        {
            SDL_SetRenderDrawColor(g->renderer, 200, 30, 30, (alpha * (8 - t)) / 16);
            glow.x = bx - t;
            glow.y = by - t;
            glow.w = bw + t * 2;
            glow.h = bh + t * 2;
            SDL_RenderFillRect(g->renderer, &glow);
            t--;
        }

        banner.x = bx;
        banner.y = by;
        banner.w = bw;
        banner.h = bh;
        SDL_SetRenderDrawColor(g->renderer, 20, 0, 0, alpha);
        SDL_RenderFillRect(g->renderer, &banner);
        SDL_SetRenderDrawColor(g->renderer, 200, 50, 50, alpha);
        SDL_RenderDrawRect(g->renderer, &banner);

        cx = bx + bw / 2;
        cy = by + bh / 2;
        sz = (int)(25 * scale);
        SDL_SetRenderDrawColor(g->renderer, 220, 60, 60, alpha);
        i = -sz;
        while (i <= sz)
        {
            SDL_RenderDrawPoint(g->renderer, cx + i, cy + i);
            SDL_RenderDrawPoint(g->renderer, cx + i, cy - i);
            SDL_RenderDrawPoint(g->renderer, cx + i + 1, cy + i);
            SDL_RenderDrawPoint(g->renderer, cx + i + 1, cy - i);
            i++;
        }

        shake    = (float)sin(tick / 60.0f) * 4.0f * (1.0f - g->anim_scale);
        shake_r.x = bx + (int)shake;
        shake_r.y = by;
        shake_r.w = bw;
        shake_r.h = bh;
        SDL_SetRenderDrawColor(g->renderer, 255, 80, 80, alpha / 3);
        SDL_RenderFillRect(g->renderer, &shake_r);
    }

    if ((tick / 600) % 2 == 0)
    {
        cont.x = WIN_W / 2 - 60;
        cont.y = by + bh + 20;
        cont.w = 120;
        cont.h = 6;
        if (g->state == STATE_SUCCESS)
            SDL_SetRenderDrawColor(g->renderer, 255, 215, 0, (alpha * 3) / 4);
        else
            SDL_SetRenderDrawColor(g->renderer, 200, 60, 60, (alpha * 3) / 4);
        SDL_RenderFillRect(g->renderer, &cont);
    }
}

/* ═══════════════════════════════════════════════════════
   RENDER (principale)
═══════════════════════════════════════════════════════ */
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
