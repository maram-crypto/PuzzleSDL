#include "header.h"



int main(int argc, char *argv[])

{

    Game        g;

    SDL_Window  *window = NULL;

    SDL_Renderer *renderer = NULL;

    char        path[64];

    SDL_Event   e;

    int         running;

    Uint32      frame_ms;

    Uint32      t0;

    Uint32      elapsed;



    (void)argc;

    (void)argv;



    running  = 1;

    frame_ms = 1000 / FPS;



    /* 1. SDL INIT */

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) return 1;

    if (!(IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) & (IMG_INIT_JPG | IMG_INIT_PNG))) return 1;

    if (TTF_Init() < 0) return 1;



    /* 2. CREATE WINDOW & RENDERER */

    window = SDL_CreateWindow(

        "INTERSTELLAR - Final Edition",

        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,

        WIN_W, WIN_H, SDL_WINDOW_SHOWN

    );

    if (!window) return 1;



    renderer = SDL_CreateRenderer(window, -1, 

        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer) return 1;



    

    if (!initialisation(&g, window, renderer))

    {

        SDL_DestroyRenderer(renderer);

        SDL_DestroyWindow(window);

        return 1;

    }



    /* 4. CHARGEMENT DU PREMIER NIVEAU */

    g.puzzle_idx = 0;

    snprintf(path, sizeof(path), "assets/puzzle%d.jpg", g.puzzle_idx + 1);



    if (!load_puzzle(&g, path))

    {

        fprintf(stderr, "Erreur: Fichier %s introuvable.\n", path);

        quitter(&g);

        return 1;

    }



    generate_pieces(&g);

    

    /* 5. GAME LOOP */

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



    /* 6. CLEAN UP */

    quitter(&g); // Nettoie les textures

    

    SDL_DestroyRenderer(renderer);

    SDL_DestroyWindow(window);

    TTF_Quit();

    IMG_Quit();

    SDL_Quit();



    return 0;

}
