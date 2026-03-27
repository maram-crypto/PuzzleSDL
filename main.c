#include "header.h"

int main(int argc, char *argv[])
{
    Game      g;
    char      path[64];
    SDL_Event e;
    int       running;
    Uint32    frame_ms;
    Uint32    t0;
    Uint32    elapsed;

    (void)argc;
    (void)argv;

    running  = 1;
    frame_ms = 1000 / FPS;

    if (!initialisation(&g))
        return 1;

    g.puzzle_idx = 0;
    snprintf(path, sizeof(path), "assets/puzzle%d.jpg", g.puzzle_idx + 1);

    if (!load_puzzle(&g, path))
    {
        fprintf(stderr,
            "Could not load assets/puzzle1.jpg\n"
            "Placez puzzle1.jpg ... puzzle5.jpg dans le dossier 'assets/'.\n");
        quitter(&g);
        return 1;
    }

    generate_pieces(&g);
    g.start_ticks = SDL_GetTicks();
    g.time_left   = TIMER_TOTAL;
    g.state       = STATE_PLAYING;

    while (running)
    {
        t0 = SDL_GetTicks();

        handle_events(&g, &e, &running);
        update(&g);
        render(&g);

        elapsed = SDL_GetTicks() - t0;
        if (elapsed < frame_ms)
            SDL_Delay(frame_ms - elapsed);
    }

    quitter(&g);
    return 0;
}
