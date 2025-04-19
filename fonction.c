#include "fonction.h"

// Initialisation de SDL
void InitSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Erreur SDL_Init : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    if (TTF_Init() != 0) {
        fprintf(stderr, "Erreur TTF_Init : %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
}
// Chargement de la carte
void LoadMap(Map *map, const char *bg_path, const char *minimap_path) {
    map->bg = SDL_LoadBMP(bg_path);
    map->minimap = SDL_LoadBMP(minimap_path);
    // Vérification des chargements
    if(!map->bg || !map->minimap) {
        fprintf(stderr, "Erreur chargement des assets de la carte\n");
        exit(EXIT_FAILURE);
    }
    
    // Taille en tuiles
    map->width = map->bg->w / TILE_SIZE;  // 2040/32 = 63.75 → 63
    map->height = map->bg->h / TILE_SIZE; // 800/32 = 25
    
    // Allocation de la matrice de collision
    map->collision = malloc(map->height * sizeof(int *));
    for(int y = 0; y < map->height; y++) {
        map->collision[y] = malloc(map->width * sizeof(int));
        for(int x = 0; x < map->width; x++) {
            // Exemple : collisions seulement sur les bords
            map->collision[y][x] = (x == 0 || x == map->width-1 || y == map->height-1);
        }
    }
    map->num_car = 5;
    map->car = malloc(map->num_car * sizeof(Car));
    
    // voiture 1 (coordonnées en pixels dans l'image complète)
    map->car[0].collision_box = (SDL_Rect){1163, 608, 80, 80}; // x, y, w, h
    map->car[0].is_active = 1;
    
    // voiture 2
    map->car[1].collision_box = (SDL_Rect){2550, 608, 80, 80};
    map->car[1].is_active = 1;
     // voiture 3
     map->car[2].collision_box = (SDL_Rect){3955, 608, 80, 80};
     map->car[2].is_active = 1;
     // voiture 4
     map->car[3].collision_box = (SDL_Rect){5170, 608, 80, 80};
     map->car[3].is_active = 1;
     // voiture 5
     map->car[4].collision_box = (SDL_Rect){5979, 608, 80, 80};
     map->car[4].is_active = 1;
}
// Libération de la mémoire de la carte
void FreeMap(Map *map) {
    for(int y = 0; y < map->height; y++)
        free(map->collision[y]);
        free(map->collision);
    SDL_FreeSurface(map->bg);
    SDL_FreeSurface(map->minimap);
    free(map->car);
}
// Gestion des collisions
int CheckCollision(Map *map, float x, float y) {
    // Conversion en coordonnées tuiles
    int tx = (int)x / TILE_SIZE;
    int ty = (int)y / TILE_SIZE;
    
    // Ajustez pour le dernier tile incomplet
    if(tx >= map->width) tx = map->width - 1;
    if(ty >= map->height) ty = map->height - 1;
    
    return map->collision[ty][tx];
}
// Initialisation du joueur
void InitPlayer(Player *player, const char *sprite_path, const char *icon_path) {
    player->sprite = SDL_LoadBMP(sprite_path);
    player->minimap_icon = SDL_LoadBMP(icon_path);
    
    if(!player->sprite || !player->minimap_icon) {
        fprintf(stderr, "Erreur chargement des sprites du joueur\n");
        exit(EXIT_FAILURE);
    }
    SDL_SetColorKey(player->sprite, SDL_SRCCOLORKEY, SDL_MapRGB(player->sprite->format, 0, 0, 0));
    SDL_SetColorKey(player->minimap_icon, SDL_SRCCOLORKEY, SDL_MapRGB(player->minimap_icon->format, 0, 0, 0));

    
    // Position initiale
    player->x = 50;
    player->y = (MAP_HEIGHT_TILES - 8) * TILE_SIZE; // Position Y alignée sur l'avant-dernière ligne
    player->width = player->sprite->w;
    player->height = player->sprite->h;
    player->is_grounded = 1;
    player->velocity_y = 0;
}
// Libération du joueur
void FreePlayer(Player *player) {
    SDL_FreeSurface(player->sprite);
    SDL_FreeSurface(player->minimap_icon);
}
// Mise à jour de la physique
void UpdatePhysics(Player *player, Map *map) {
    // Appliquer la gravité en permanence
    player->velocity_y += GRAVITY;
    player->y += player->velocity_y;

    // Collision verticale BASSE seulement
    int tile_x = (player->x + player->width/2) / TILE_SIZE;
    int tile_y = (player->y + player->height) / TILE_SIZE;

    if(CheckCollision(map, tile_x * TILE_SIZE, tile_y * TILE_SIZE)) {
        player->y = tile_y * TILE_SIZE - player->height;
        player->is_grounded = 1;
        player->velocity_y = 0;
    } else {
        player->is_grounded = 0;
    }
}
// Gestion des entrées
void HandleInput(Player *player, Map *map) {
    const Uint8 *keys = SDL_GetKeyState(NULL);
    
    // Ajouter des collisions horizontales dans HandleInput()
    if(keys[SDLK_LEFT]) {
        float new_x = player->x - PLAYER_SPEED;
        // Vérifier collision sur le côté gauche
        if(!CheckCollision(map, new_x, player->y) && 
           !CheckCollision(map, new_x, player->y + player->height - 1)) {
            player->x = fmax(0, new_x);
        }
    }
    
    // Dans HandleInput() :
   if(keys[SDLK_RIGHT]) {
    float new_x = player->x + PLAYER_SPEED;
    float max_x = map->bg->w - player->width; // Utilisez la largeur réelle de l'image

    if(new_x <= max_x) { // Pas besoin de CheckCollision() ici
        player->x = new_x;
      }
     }
 // Saut
    
 if(keys[SDLK_UP] && player->is_grounded) {
    player->velocity_y = JUMP_FORCE;
    player->is_grounded = 0;
 }
}
// Mise à jour de la caméra
void UpdateCamera(Map *map, Player *player, float delta_time) {
    // Calcul de la cible centrée sur le joueur
    float target_x = player->x + player->width/2 - SCREEN_WIDTH/2.0f;
    
    // Limites du scrolling
    float max_offset = map->bg->w - SCREEN_WIDTH;
    target_x = fmaxf(0.0f, fminf(target_x, max_offset));
    
    // Lissage plus réactif
    float lerp_speed = 10.0f; // Augmenté pour plus de réactivité
    map->offset_x += (target_x - map->offset_x) * lerp_speed * delta_time;
    
    // Protection contre les erreurs d'arrondi
    map->offset_x = fmaxf(0.0f, fminf(map->offset_x, max_offset));
}
// Affichage du jeu
void DrawGame(SDL_Surface *screen, Map *map, Player *player) {
    // Arrière-plan avec scrolling
    SDL_Rect bg_src = {map->offset_x, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_BlitSurface(map->bg, &bg_src, screen, NULL);
    
    // Joueur
    SDL_Rect player_dest = {
        (int)(player->x - map->offset_x),
        (int)(player->y - map->offset_y), // Si offset_y existe
        //(int)player->y,
        player->width,
        player->height
    };
    SDL_BlitSurface(player->sprite, NULL, screen, &player_dest);
    
    // Minimap
    SDL_Rect minimap_dest = {SCREEN_WIDTH - map->minimap->w - 10, 10, map->minimap->w, map->minimap->h};
    SDL_BlitSurface(map->minimap, NULL, screen, &minimap_dest);
    // Calcul de la position du joueur dans la minimap
    float scale_x = (float)map->minimap->w / map->bg->w;
    float scale_y = (float)map->minimap->h / map->bg->h;
    
    SDL_Rect player_minimap_pos = {
        850+33 + (int)(player->x * scale_x) - player->minimap_icon->w/2,
        25 + (int)(player->y * scale_y) - player->minimap_icon->h/2,
        player->minimap_icon->w,
        player->minimap_icon->h
    };

    // Dessin de l'icône du joueur
    SDL_BlitSurface(player->minimap_icon, NULL, screen, &player_minimap_pos);

}
//retourne le temps actuel
time_t init_time(){ 
  return time( NULL );
}
//begin est le temps du début,limit le temps limite
int time_expired(int debut,int limite){
    unsigned int end = SDL_GetTicks();
    unsigned int secondes = (unsigned int) difftime( end, debut );//fonction qui donne la diff entre deux temps
    if(secondes/1000>=limite)return 1;return 0;
}
//affichage le temps
void show_time(int debut,SDL_Surface *screen){
    char ch[24];
    TTF_Font *font = TTF_OpenFont("time.ttf", 60);
    SDL_Color color = {255, 255, 255};
    SDL_Surface *text = NULL;

    Uint32 maintenant = SDL_GetTicks();
    Uint32 temps_ecoule = (maintenant - debut) / 1000; // En secondes
    sprintf(ch, "%02u:%02u", temps_ecoule / 60, temps_ecoule % 60);
    text = TTF_RenderText_Solid(font, ch, color);

    SDL_Rect text_pos = {10, 10, 0, 0}; // Position du texte
    SDL_BlitSurface(text, NULL, screen, &text_pos);

    SDL_FreeSurface(text);
    TTF_CloseFont(font);
}
void InitEnemy(Enemy *enemy, Map *map) {
    int SIZE = 30; // Taille du point (15x15 pixels)
    int MINI_SIZE = 10; // Taille dans la minimap
    
    enemy->icon = SDL_CreateRGBSurface(SDL_HWSURFACE, SIZE, SIZE, 32, 0, 0, 0, 0);
    SDL_FillRect(enemy->icon, NULL, SDL_MapRGB(enemy->icon->format, 255, 0, 0));
    
    enemy->mini_icon = SDL_CreateRGBSurface(SDL_HWSURFACE, MINI_SIZE, MINI_SIZE, 32, 0, 0, 0, 0);
    SDL_FillRect(enemy->mini_icon, NULL, SDL_MapRGB(enemy->mini_icon->format, 255, 0, 0));
    
    enemy->x = map->bg->w / 2;
    enemy->y = map->bg->h -70;
    enemy->velocity_x = 5.0f; // Vitesse plus visible
    enemy->direction = 1;
    enemy->moving_range = 200; // Déplacement sur 200 pixels
}
void DrawEnemy(SDL_Surface *screen, Map *map, Enemy *enemy) {
    // Dessin dans la vue principale
    SDL_Rect main_pos = {
        (int)(enemy->x - map->offset_x - 2),
        (int)(enemy->y - map->offset_y - 2),
        5, 5
    };
    SDL_BlitSurface(enemy->icon, NULL, screen, &main_pos);
    
    // Dessin dans la minimap
    float scale_x = (float)map->minimap->w / map->bg->w;
    float scale_y = (float)map->minimap->h / map->bg->h;
    
    SDL_Rect mini_pos = {
        SCREEN_WIDTH - map->minimap->w - 10 + (int)(enemy->x * scale_x),
        10 + (int)(enemy->y * scale_y),
        3, 3
    };
    SDL_BlitSurface(enemy->mini_icon, NULL, screen, &mini_pos);
}
int CheckPlayerEnemyCollision(Player *player, Enemy *enemy) {
    // Création des rectangles de collision
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

    // Collision manuelle pour SDL 1.2
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
    // Sauvegarde de l'ancienne position
    int old_x = enemy->x;
    
    // Déplacement
    enemy->x += enemy->velocity_x * enemy->direction;
    
    // Vérification de la zone de mouvement
    if(abs(enemy->x - (map->bg->w/2)) > enemy->moving_range/2) {
        enemy->direction *= -1;
    }
    
    // Collision avec le décor
    int tile_left = (enemy->x - enemy->icon->w/2) / TILE_SIZE;
    int tile_right = (enemy->x + enemy->icon->w/2) / TILE_SIZE;
    int tile_y = (enemy->y + enemy->icon->h/2) / TILE_SIZE;
    
    if(map->collision[tile_y][tile_left] || map->collision[tile_y][tile_right]) {
        enemy->x = old_x;
        enemy->direction *= -1;
    }
}
// Corrigez l'implémentation
int CheckCarCollision(Player *player, Car *car) {
    if(!car->is_active) return 0;
    
    SDL_Rect p = {
        (int)player->x,
        (int)player->y,
        player->width,
        player->height
    };
    
    return (p.x < car->collision_box.x + car->collision_box.w &&
            p.x + p.w > car->collision_box.x &&
            p.y < car->collision_box.y + car->collision_box.h &&
            p.y + p.h > car->collision_box.y);
}