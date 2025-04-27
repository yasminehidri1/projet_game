#include "fonction.h"


void InitSDL() {
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Erreur SDL_Init : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    if(TTF_Init() != 0) {
        fprintf(stderr, "Erreur TTF_Init : %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
}


void LoadMap(Map *map, const char *bg_path, const char *minimap_path) {
    memset(map, 0, sizeof(Map));
    map->bg = SDL_LoadBMP(bg_path);
    map->minimap = SDL_LoadBMP(minimap_path);
    map->shake_duration = 0;
    if(!map->bg || !map->minimap) {
        fprintf(stderr, "Erreur chargement des assets de la carte\n");
        exit(EXIT_FAILURE);
    }
    
    map->width = map->bg->w / TILE_SIZE;  // 6880/32 = 215
    map->height = map->bg->h / TILE_SIZE; // 800/32 = 25
    
    map->collision = (int**)malloc(map->height * sizeof(int *));
    if(!map->collision) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    for(int y = 0; y < map->height; y++) {
        map->collision[y] =(int*) malloc(map->width * sizeof(int));
        for(int x = 0; x < map->width; x++) {
            // Exemple : collisions seulement sur les bords
            map->collision[y][x] = (x == 0 || x == map->width-1 || y == map->height-1);
        }
    }
    map->num_car = 5;
    map->car = (Car*)malloc(map->num_car * sizeof(Car));
    
    // voiture 1 
    map->car[0].collision_box = (SDL_Rect){1268, 870, 80, 80}; // x, y, w, h
    map->car[0].is_active = 1;
    
    // voiture 2
    map->car[1].collision_box = (SDL_Rect){2774, 870, 80, 80};
    map->car[1].is_active = 1;
     // voiture 3
     map->car[2].collision_box = (SDL_Rect){4334, 870, 80, 80};
     map->car[2].is_active = 1;
     // voiture 4
     map->car[3].collision_box = (SDL_Rect){5660, 870, 80, 80};
     map->car[3].is_active = 1;
     // voiture 5
     map->car[4].collision_box = (SDL_Rect){6533, 870, 80, 80};
     map->car[4].is_active = 1;
    }

void FreeMap(Map *map) {
    if(map) {
        if(map->collision) {
            for(int y = 0; y < map->height; y++) {
                free(map->collision[y]);
            }
            free(map->collision);
        }
        
        if(map->bg) SDL_FreeSurface(map->bg);
        if(map->minimap) SDL_FreeSurface(map->minimap);
        if(map->car) free(map->car);
        
    }
}

int CheckCollision(Map *map, float x, float y) {
    if(!map || !map->collision) return 0;
    
    int tx = (int)x / TILE_SIZE;
    int ty = (int)y / TILE_SIZE;

    // Vérification des limites
    if(tx < 0 || tx >= map->width || ty < 0 || ty >= map->height)
        return 1; // Considérer hors limites comme collision

    return map->collision[ty][tx];
}

void InitPlayer(Player *player, const char *sprite_path, const char *icon_path) {
    memset(player, 0, sizeof(Player));
    
    player->sprite = IMG_Load(sprite_path);
    player->minimap_icon = IMG_Load(icon_path);
    
    if(!player->sprite || !player->minimap_icon) {
        fprintf(stderr, "Error loading player sprites: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    
    SDL_SetColorKey(player->sprite, SDL_SRCCOLORKEY, SDL_MapRGB(player->sprite->format, 0, 0, 0));
    SDL_SetColorKey(player->minimap_icon, SDL_SRCCOLORKEY, SDL_MapRGB(player->minimap_icon->format, 0, 0, 0));

    player->x = 50;
    player->y = (MAP_HEIGHT_TILES) * TILE_SIZE;
    player->width = player->sprite->w;
    player->height = player->sprite->h;
    player->health = 3;
    InitParticleSystem(&player->particle_system, 1000);
}

void FreePlayer(Player *player) {
    if(player) {
        if(player->sprite) SDL_FreeSurface(player->sprite);
        if(player->minimap_icon) SDL_FreeSurface(player->minimap_icon);
        FreeParticleSystem(&player->particle_system);
        memset(player, 0, sizeof(Player));
    }
}

void UpdatePhysics(Player *player, Map *map, const Uint8 *keys, float delta_time) {
    player->key_state = keys; // Stocke la référence
    player->velocity_y += GRAVITY;//(acceleration du joueur)
    player->y += player->velocity_y;

    // Détection de Collision avec le Sol
    int tile_x = (player->x + player->width/2) / TILE_SIZE;
    int tile_y = (player->y + player->height) / TILE_SIZE;
    if(CheckCollision(map, tile_x * TILE_SIZE, tile_y * TILE_SIZE)) {
        player->y = tile_y * TILE_SIZE - player->height; // Aligne le joueur au sol
        player->is_grounded = 1; 
        player->velocity_y = 0; 
    } else {
        player->is_grounded = 0; 
    }
    // Poussière de course (quand le joueur se déplace au sol)
    if (player->is_grounded && (keys[SDLK_LEFT] || keys[SDLK_RIGHT])) {
        SDL_Color dust_color = {180, 160, 140};
        AddParticle(&player->particle_system, 
                   player->x + player->width/2,
                   player->y + player->height - 5,
                   dust_color,
                   (keys[SDLK_LEFT] ? 1.0f : -1.0f) * -0.5f,
                   0.0f,
                   30,
                   2 + rand() % 3);
    }
 

     // Étincelles de saut (au décollage)
     static int was_grounded = 1;
     if (player->is_grounded && !was_grounded) { // Atterrissage
       SDL_Color impact_color = {220, 220, 255};
        for (int i = 0; i < 15; i++) {
        AddParticle(&player->particle_system,
            player->x + player->width/2,
            player->y + player->height,
            impact_color,
            (rand() % 10 - 5) * 0.3f,
            -(rand() % 5) * 0.5f,
            40,
            3);}
    
 }
 was_grounded = player->is_grounded;
 
}

void HandleInput(Player *player, Map *map, const Uint8 *keys) {
    //const Uint8 *keys = SDL_GetKeyState(NULL);
    
   
    if((keys[SDLK_LEFT])|| (keys[SDLK_s])) {
        float new_x = player->x - PLAYER_SPEED;
       
        if(!CheckCollision(map, new_x, player->y) && 
           !CheckCollision(map, new_x, player->y + player->height - 1)) {
            player->x = fmax(0, new_x);// Bloque à x=0 (bord gauche de l'écran)
        }
    }
    
    
   if(keys[SDLK_RIGHT]|| (keys[SDLK_d])) {
    float new_x = player->x + PLAYER_SPEED;
    float max_x = map->bg->w - player->width; // Utilisez la largeur réelle de l'image

    if(new_x <= max_x) { 
        player->x = new_x;
      }
     }

 if(keys[SDLK_UP] && player->is_grounded) {
    player->velocity_y = JUMP_FORCE;
    player->is_grounded = 0;
 }
}

void UpdateCamera(Map *map, Player *player, float delta_time) {
    // Calcule la position target_x pour que le joueur soit au centre de l'écran (ou presque).
    float target_x = player->x + player->width/2 - SCREEN_WIDTH/2.0f;
    
    float max_offset = map->bg->w - SCREEN_WIDTH;// Détermine le décalage maximal possible pour que la caméra ne dépasse pas le bord droit de la carte.
    target_x = fmaxf(0.0, fminf(target_x, max_offset));//Clamp la valeur entre 0 (bord gauche) et max_offset (bord droit)
    
    float lerp_speed = 10.0; 
    map->offset_x += (target_x - map->offset_x) * lerp_speed * delta_time;

    map->offset_x = fmaxf(0.0, fminf(map->offset_x, max_offset));
}

void DrawGame(SDL_Surface *screen, Map *map, Player *player) {
    HandleShakeEffect(map);
    SDL_Rect bg_src = {map->offset_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_BlitSurface(map->bg, &bg_src, screen, NULL);
    SDL_Rect player_dest = {
        (int)(player->x - map->offset_x),
        (int)(player->y - map->offset_y), 
        //(int)player->y,
        player->width,
        player->height
    };
    if(SDL_GetTicks() % 100 < 50 && player->shake_timer > SDL_GetTicks()) {
        SDL_SetColorKey(player->sprite, SDL_SRCCOLORKEY, SDL_MapRGB(player->sprite->format, 255, 0, 0));
    }
    SDL_BlitSurface(player->sprite, NULL, screen, &player_dest);
    SDL_SetColorKey(player->sprite, SDL_SRCCOLORKEY, SDL_MapRGB(player->sprite->format, 0, 0, 0));
    DrawParticles(&player->particle_system, screen, map);

    SDL_Rect minimap_dest = {SCREEN_WIDTH - map->minimap->w - 60, 10, map->minimap->w, map->minimap->h};
    SDL_BlitSurface(map->minimap, NULL, screen, &minimap_dest);
    float scale_x = (float)map->minimap->w / map->bg->w;
    float scale_y = (float)map->minimap->h / map->bg->h;
    
    SDL_Rect player_minimap_pos = {
        1380 + (int)(player->x * scale_x) - player->minimap_icon->w/2,
        20 + (int)(player->y * scale_y) - player->minimap_icon->h/2,
        player->minimap_icon->w,
        player->minimap_icon->h
    };
    SDL_BlitSurface(player->minimap_icon, NULL, screen, &player_minimap_pos);
    // Overlay rouge quand touché ou vie faible
    if(player->is_hit || player->health <= 0) {
        SDL_Surface *blood = SDL_CreateRGBSurface(SDL_HWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0,0,0,0);
        SDL_FillRect(blood, NULL, SDL_MapRGB(blood->format, 255, 0, 0));
        SDL_SetAlpha(blood, SDL_SRCALPHA, player->is_hit ? 10 : 30); // Plus opaque quand touché
        SDL_BlitSurface(blood, NULL, screen, NULL);
        SDL_FreeSurface(blood);
    }
    }

time_t init_time(){ 
  return time( NULL );
}

int time_expired(int debut,int limite){
    unsigned int end = SDL_GetTicks();
    unsigned int secondes = (unsigned int) difftime( end, debut );//fonction qui donne la diff entre deux temps
    if(secondes/1000>=limite)return 1;return 0;
}

void show_time(int debut, SDL_Surface *screen, Player *player) {
    char ch[24];
    TTF_Font *font = TTF_OpenFont("assets/time.ttf", 50);
    if (!font) {
        fprintf(stderr, "Erreur chargement police: %s\n", TTF_GetError());
        return;
    }

    SDL_Color color = {255, 255, 255};
    
    // Timer
    Uint32 maintenant = SDL_GetTicks();
    Uint32 temps_ecoule = (maintenant - debut) / 1000;
    sprintf(ch, "Time: %02u:%02u", temps_ecoule / 60, temps_ecoule % 60);
    
    SDL_Surface *text = TTF_RenderText_Solid(font, ch, color);
    if (text) {
        SDL_Rect text_pos = {10, 10, text->w, text->h};  // Utilisez les vraies dimensions
        SDL_BlitSurface(text, NULL, screen, &text_pos);
        SDL_FreeSurface(text);
    }

    // Vies
    for (int i = 0; i < player->health; i++) {
        SDL_Surface *heart = SDL_LoadBMP("assets/heart.bmp");
        
        if (heart) {
            SDL_SetColorKey(heart, SDL_SRCCOLORKEY, SDL_MapRGB(heart->format, 0, 0, 0));

            SDL_Rect heart_pos = {220 + i * 40, 10, heart->w, heart->h};  // Taille réelle
            SDL_BlitSurface(heart, NULL, screen, &heart_pos);
            SDL_FreeSurface(heart);  // Libération immédiate après utilisation
        } else {
            fprintf(stderr, "Coeur introuvable! Verifiez assets/heart.bmp\n");
        }
    }
    
    TTF_CloseFont(font);
}
void InitEnemy(Enemy *enemy, Map *map) {
    int SIZE = 30;
    int MINI_SIZE = 10; 
    
    enemy->icon = SDL_CreateRGBSurface(SDL_HWSURFACE, SIZE, SIZE, 32, 0, 0, 0, 0);
    enemy->mini_icon = SDL_CreateRGBSurface(SDL_HWSURFACE, MINI_SIZE, MINI_SIZE, 32, 0, 0, 0, 0);
    
    if(!enemy->icon || !enemy->mini_icon) {
        fprintf(stderr, "Erreur création surfaces ennemi\n");
        exit(EXIT_FAILURE);
    }
    
    SDL_FillRect(enemy->icon, NULL, SDL_MapRGB(enemy->icon->format, 255, 0, 0));
    SDL_FillRect(enemy->mini_icon, NULL, SDL_MapRGB(enemy->mini_icon->format, 255, 0, 0));
    
    enemy->x = map->bg->w / 2;
    enemy->y = map->bg->h - 170;
    enemy->velocity_x = 5.0; 
    enemy->direction = 1;
    enemy->moving_range = 200; 
}
void DrawEnemy(SDL_Surface *screen, Map *map, Enemy *enemy) {
    
    SDL_Rect main_pos = {
        (int)(enemy->x - map->offset_x - 2),
        (int)(enemy->y - map->offset_y - 2),
        5, 5
    };
    SDL_BlitSurface(enemy->icon, NULL, screen, &main_pos);
    
    
    float scale_x = (float)map->minimap->w / map->bg->w;
    float scale_y = (float)map->minimap->h / map->bg->h;
    
    SDL_Rect mini_pos = {
        SCREEN_WIDTH - map->minimap->w - 55 + (int)(enemy->x * scale_x),
        10 + (int)(enemy->y * scale_y),
        3, 3
    };
    SDL_BlitSurface(enemy->mini_icon, NULL, screen, &mini_pos);
}
int CheckPlayerEnemyCollision(Player *player, Enemy *enemy) {
    
    SDL_Rect player_rect = {
        (int)player->x,
        (int)player->y,
        player->width,
        player->height
    };
    
    SDL_Rect enemy_rect = {
        enemy->x - 2,
        enemy->y - 2,
        5,
        5
    };

    return !(player_rect.x + player_rect.w < enemy_rect.x ||
             player_rect.x > enemy_rect.x + enemy_rect.w ||
             player_rect.y + player_rect.h < enemy_rect.y ||
             player_rect.y > enemy_rect.y + enemy_rect.h);

}
void FreeEnemy(Enemy *enemy) {
    SDL_FreeSurface(enemy->icon);
    SDL_FreeSurface(enemy->mini_icon);
}
void UpdateEnemy(Enemy *enemy, Map *map) {
    
    int old_x = enemy->x;
    
    enemy->x += enemy->velocity_x * enemy->direction;
    
    if(abs(enemy->x - (map->bg->w/2)) > enemy->moving_range/2) {
        enemy->direction *= -1;
    }
   
    int tile_left = (enemy->x - enemy->icon->w/2) / TILE_SIZE;
    int tile_right = (enemy->x + enemy->icon->w/2) / TILE_SIZE;
    int tile_y = (enemy->y + enemy->icon->h/2) / TILE_SIZE;
    
    if(map->collision[tile_y][tile_left] || map->collision[tile_y][tile_right]) {
        enemy->x = old_x;
        enemy->direction *= -1;
    }
}

int CheckCollisionRect(const SDL_Rect *a, const SDL_Rect *b) {
    return !(a->x + a->w <= b->x || 
            b->x + b->w <= a->x ||
            a->y + a->h <= b->y ||
            b->y + b->h <= a->y);
}

int CheckCarCollision(Player *player, Car *car) {
    if (!car->is_active) return 0;
    
    SDL_Rect p = {
        (int)player->x,
        (int)player->y,
        player->width,
        player->height
    };
    
    return CheckCollisionRect(&p, &car->collision_box);
}
void HandleShakeEffect(Map *map) {
    if(!map) return;
    
    if(map->shake_duration > 0) {
        map->offset_x += rand() % 10 - 5;
        map->shake_duration -= 02;
        
    }
}
void UpdatePlayerHitEffect(Player *player) {
    if (player->is_hit) {
        if (SDL_GetTicks() - player->hit_timer >= 500) { // 2 secondes
            player->is_hit = 0; // Désactiver l'effet après 2s
        }
    }
}
void InitParticleSystem(ParticleSystem *ps, int capacity) {
    ps->particles = (Particle*)malloc(capacity * sizeof(Particle));
    ps->capacity = capacity;
    ps->count = 0;
}

void FreeParticleSystem(ParticleSystem *ps) {
    free(ps->particles);
    ps->count = ps->capacity = 0;
}

void AddParticle(ParticleSystem *ps, float x, float y, SDL_Color color, float vx, float vy, int lifetime, int size) {
    if (ps->count >= ps->capacity) return;
    
    Particle p = {
        .x = x,
        .y = y,
        .vx = vx + ((rand() % 20 - 10) * 0.1f), // Petite variation aléatoire
        .vy = vy + ((rand() % 20 - 10) * 0.1f),
        .lifetime = lifetime,
        .max_lifetime = lifetime,
        .color = color,
        .size = size
    };
    
    ps->particles[ps->count++] = p;
}

void UpdateParticles(ParticleSystem *ps, float delta_time) {
    for (int i = 0; i < ps->count; ) {
        ps->particles[i].x += ps->particles[i].vx * delta_time * 60.0f;
        ps->particles[i].y += ps->particles[i].vy * delta_time * 60.0f;
        ps->particles[i].vy += 0.1f; // Gravité légère
        ps->particles[i].lifetime--;
        
        if (ps->particles[i].lifetime <= 0) {
            // Remplace par la dernière particule pour éviter les trous
            ps->particles[i] = ps->particles[--ps->count];
        } else {
            i++;
        }
    }
}

void DrawParticles(ParticleSystem *ps, SDL_Surface *screen, Map *map) {
    for (int i = 0; i < ps->count; i++) {
        Particle p = ps->particles[i];
        float alpha = (float)p.lifetime / p.max_lifetime; // Fade out
        
        SDL_Rect rect = {
            (int)(p.x - map->offset_x - p.size/2),
            (int)(p.y - map->offset_y - p.size/2),
            p.size,
            p.size
        };
        
        // Calcul couleur avec transparence
        Uint32 sdl_color = SDL_MapRGBA(screen->format,
            p.color.r,
            p.color.g,
            p.color.b,
            (Uint8)(alpha * 255));
        
        SDL_FillRect(screen, &rect, sdl_color);
    }
}

void InitWeather(WeatherSystem* weather, int capacity, WeatherType type) {
    InitParticleSystem(&weather->system, capacity);
    weather->type = type;
    weather->last_spawn = 0;
    
    
}

void UpdateWeather(WeatherSystem* weather, int map_w, int map_h, int camera_x, int camera_y, float delta_time) {
    const float WIND_FORCE = 0.3f;
    const int SPAWN_BUFFER = 200; // Tampon autour de la caméra pour générer les particules

    int spawn_start_x = camera_x - SPAWN_BUFFER;
    int spawn_end_x = camera_x + SCREEN_WIDTH + SPAWN_BUFFER;
    spawn_start_x = fmax(0, spawn_start_x);
    spawn_end_x = fmin(map_w, spawn_end_x);

    if(SDL_GetTicks() - weather->last_spawn > 50) {
        for(int i = 0; i < 5; i++) {
            float spawn_x, spawn_y;
            if (weather->type == WEATHER_RAIN) {
                spawn_x = spawn_start_x + rand() % (spawn_end_x - spawn_start_x);
                spawn_y = -10 - rand() % 50;
            } else {
                spawn_x = rand() % map_w;
                spawn_y = -10 - rand() % 50;
            }

            AddParticle(&weather->system, spawn_x, spawn_y,
                       (weather->type == WEATHER_RAIN) ? (SDL_Color){150, 200, 255, 255} : (SDL_Color){255, 255, 255, 255},
                       (weather->type == WEATHER_RAIN) ? WIND_FORCE * (rand() % 3 - 1) : sin(SDL_GetTicks()*0.001f) * 2.0f,
                       (weather->type == WEATHER_RAIN) ? 15.0f + rand() % 5 : 3.0f + (rand() % 100)/100.0f,
                       1000,
                       (weather->type == WEATHER_RAIN) ? 2 : 3);
        }
        weather->last_spawn = SDL_GetTicks();
    }

    for(int i = 0; i < weather->system.count; i++) {
        Particle* p = &weather->system.particles[i];
        
        if(weather->type == WEATHER_RAIN) {
            p->vy += 0.1f;
            p->vx += (rand() % 10 - 5) * 0.01f;
        } else {
            p->vx = sin(SDL_GetTicks()*0.001f + p->x*0.01f) * 2.0f;
            p->vy += 0.05f;
        }

        if(p->y > map_h || p->x < -50 || p->x > map_w + 50) {
            p->x = (weather->type == WEATHER_RAIN) ? (spawn_start_x + rand() % (spawn_end_x - spawn_start_x)) : rand() % map_w;
            p->y = -10 - rand() % 50;
            p->lifetime = 1000;
        }
    }

    UpdateParticles(&weather->system, delta_time);
}

void DrawWeather(WeatherSystem* weather, SDL_Surface* screen, Map* map) {
    for(int i = 0; i < weather->system.count; i++) {
        Particle p = weather->system.particles[i];
        SDL_Rect rect;

        if(weather->type == WEATHER_RAIN) {
            // Dessiner une ligne oblique
            int length = 15 + (int)(p.vy * 0.5f);
            for(int j = 0; j < length; j++) {
                int px = p.x - map->offset_x + j * p.vx * 0.2f;
                int py = p.y - map->offset_y + j;
                if(px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                    *((Uint32*)screen->pixels + py * screen->w + px) = 
                        SDL_MapRGB(screen->format, p.color.r, p.color.g, p.color.b);
                }
            }
        }
        else {
            // Dessiner un point avec effet de scintillement
            rect.x = p.x - map->offset_x + sin(SDL_GetTicks()*0.005f + p.x) * 2;
            rect.y = p.y - map->offset_y;
            rect.w = 3 + sin(SDL_GetTicks()*0.01f + p.x) * 1;
            rect.h = 3;
            SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255, 255, 255));
        }
    }
}

void FreeWeather(WeatherSystem* weather) {
    FreeParticleSystem(&weather->system);
}
