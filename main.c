#include "fonction.h"

int main() {
    InitSDL();
    SDL_Surface *screen = SDL_SetVideoMode(1270, 800, 32, SDL_HWSURFACE);
    
    Map game_map;
    Player player;
    Enemy enemy;
    
    int current_level = 0;
    const char *bg_paths[] = {"assets/level1_bg.bmp", "assets/level2_bg.bmp"};
    const char *minimap_paths[] = {"assets/level1_minimap.bmp", "assets/level2_minimap.bmp"};
    
    LoadMap(&game_map, bg_paths[current_level], minimap_paths[current_level]);
    InitEnemy(&enemy, &game_map);
    InitPlayer(&player, "assets/joueur.bmp", "assets/mini_joueur.bmp");
    
    Uint32 start_time = SDL_GetTicks();
    Uint32 previous_time = start_time;
    
    int running = 1;
    while(running) {
        Uint32 current_time = SDL_GetTicks();
        float delta_time = (current_time - previous_time) / 1000.0f;
        previous_time = current_time;

        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) running = 0;
        }
        
        HandleInput(&player, &game_map);
        UpdatePhysics(&player, &game_map);
        UpdateEnemy(&enemy, &game_map);
        // Vérification collision joueur-ennemi
        if(CheckPlayerEnemyCollision(&player, &enemy)) {
            printf("Game Over!\n");
            running = 0; // Ou réinitialiser le niveau
        }

        
        // Condition de fin de niveau (pixels)
        if (player.x + player.width >= 0.95 * game_map.bg->w) {
            printf("Changement de niveau déclenché !\n");
            current_level++;
            if (current_level >= 2) current_level = 0;
            
            FreeMap(&game_map);
            FreePlayer(&player);
            
            LoadMap(&game_map, bg_paths[current_level], minimap_paths[current_level]);
            InitPlayer(&player, "assets/joueur.bmp", "assets/mini_joueur.bmp");
            if(current_level == 1) {
                // Position Y plus haute (ex: 300px depuis le haut)
                player.y = 600; 
                // Réinitialiser la vélocité pour éviter les bugs
                player.velocity_y = 0;
                player.is_grounded = 40;
            } else {
                // Position par défaut pour le niveau 1
                player.y = (game_map.height - 8) * TILE_SIZE;
            }
            game_map.offset_x = 0; // Réinitialiser la caméra
        }
         // Vérifier collisions avec toutes les roues
    for(int i = 0; i < game_map.num_car; i++) {
        if(CheckCarCollision(&player, &game_map.car[i])) {
            printf("Collision avec voiture !\n");
            running = 0; // Gérer la fin du jeu
        }
    }
        
        UpdateCamera(&game_map, &player, delta_time);
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
        DrawGame(screen, &game_map, &player);
        DrawEnemy(screen, &game_map, &enemy);
        show_time(start_time, screen);
        SDL_Flip(screen);
        SDL_Delay(10);
    }
    
    FreeMap(&game_map);
    FreePlayer(&player);
    FreeEnemy(&enemy);
    SDL_Quit();
    return 0;
}