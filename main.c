#include "fonction.h"

int main() {
    InitSDL();
    SDL_Surface *screen = SDL_SetVideoMode(1920, 1080, 32, SDL_HWSURFACE);
    if (!screen) {
        fprintf(stderr, "Erreur création fenêtre: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    Map game_map = {0};
    Player player = {0};
    Enemy enemy = {0};
    ParticleSystem ps={0};
    WeatherSystem weather;
    
    InitParticleSystem(&ps, 50); // Initialisation ajoutée

    int current_level = 0;
    const char *bg_paths[] = {"assets/level1_bg.bmp", "assets/level2_bg.bmp"};
    const char *minimap_paths[] = {"assets/level1_minimap.bmp", "assets/level2_minimap.bmp"};
    
    LoadMap(&game_map, bg_paths[current_level], minimap_paths[current_level]);
    InitEnemy(&enemy, &game_map);
    InitPlayer(&player, "assets/joueur.bmp", "assets/mini_joueur.bmp");
    InitWeather(&weather, 1000, WEATHER_RAIN); // 1000 particules

   
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
         const Uint8 *keys = SDL_GetKeyState(NULL);
        
        UpdatePhysics(&player, &game_map, keys, delta_time);
        HandleInput(&player, &game_map, keys);
        UpdateParticles(&player.particle_system, delta_time);
        UpdateEnemy(&enemy, &game_map);
        UpdatePlayerHitEffect(&player);
        UpdateWeather(&weather, game_map.bg->w, game_map.bg->h, game_map.offset_x, game_map.offset_y, delta_time);

        
        // Collision avec ennemi
        if(CheckPlayerEnemyCollision(&player, &enemy)) {
            if(!player.is_hit) {
                player.health--;
                player.is_hit = 1;
                player.hit_timer = SDL_GetTicks();
                game_map.shake_duration = 200;
                
                // Effet de réduction temporaire
                player.width = player.sprite->w ;
                player.shrink_timer = SDL_GetTicks() + 1000;
            }
        }

        // Collision avec voitures
        for(int i = 0; i < game_map.num_car; i++) {
            if(CheckCarCollision(&player, &game_map.car[i])) {
                if(!player.is_hit) {
                    player.health--;
                    player.is_hit = 1;
                    player.hit_timer = SDL_GetTicks();
                    game_map.shake_duration = 200;
                }
            }
        }

        // Changement de niveau
        if (player.x + player.width >= 0.95f * game_map.bg->w) {
            current_level = (current_level + 1) % 2; // Alterner entre 0 et 1
            
            FreeMap(&game_map);
            FreePlayer(&player);
            
            LoadMap(&game_map, bg_paths[current_level], minimap_paths[current_level]);
            InitPlayer(&player, "assets/joueur.bmp", "assets/mini_joueur.bmp");
            
            // Position initiale différente selon le niveau
            player.y = (current_level == 1) ? 600 : (game_map.height - 8) * TILE_SIZE;
            player.velocity_y = 0;
            player.is_grounded = (current_level == 1) ? 40 : 1;
            game_map.offset_x = 0;
        }
        // Gestion des touches pour la météo
if(keys[SDLK_k]) { 
    FreeWeather(&weather); 
    InitWeather(&weather, 1000, WEATHER_RAIN); 
}
if(keys[SDLK_s]) { 
    FreeWeather(&weather); 
    InitWeather(&weather, 1000, WEATHER_SNOW); 
}

        // Game over
        if (player.health <= 0) {
            printf("Game Over!\n");
            running = 0;
        }

        UpdateCamera(&game_map, &player, delta_time);
        
        // Rendu
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
        DrawGame(screen, &game_map, &player);
        DrawEnemy(screen, &game_map, &enemy);
        show_time(start_time, screen, &player);
        DrawWeather(&weather, screen, &game_map);
        SDL_Flip(screen);
        
        SDL_Delay(10);
    }

    
    // Nettoyage
    FreeMap(&game_map);
    FreePlayer(&player);
    FreeEnemy(&enemy);
    FreeParticleSystem(&ps);
    FreeWeather(&weather);
    TTF_Quit();
    SDL_Quit();
    return 0;
}