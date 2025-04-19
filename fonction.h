#ifndef PLATFORMER_H
#define PLATFORMER_H
#include <SDL/SDL_ttf.h>
#include <SDL/SDL.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL_image.h>
#include <math.h> 



// Configuration des constantes
#define TILE_SIZE 32          // Taille des tuiles en pixels
#define GRAVITY 0.73     // Force de la gravité
#define JUMP_FORCE -20.0     // Force initiale du saut
#define PLAYER_SPEED 9      // Vitesse de déplacement horizontal
#define MINIMAP_SCALE 0.20   // Échelle de la minimap
#define SCREEN_WIDTH 1270    // Résolution de l'écran
#define SCREEN_HEIGHT 800
#define MAP_WIDTH_TILES 20
#define MAP_HEIGHT_TILES 20

// Structure du joueur
typedef struct {
    SDL_Surface *sprite;      // Sprite principal
    SDL_Surface *minimap_icon;// Icône pour la minimap
    float x, y;               // Position en float pour mouvement fluide
    int width, height;        // Dimensions du sprite
    float velocity_y;         // Vitesse verticale  
    int is_grounded;          // Indicateur de contact au sol
} Player;
typedef struct {
    SDL_Rect collision_box; // Zone de collision (coordonnées monde)
    int is_active;          // Si la voiture est active
}Car;

// Structure de la carte
typedef struct {
    SDL_Surface *bg;          // Arrière-plan principal
    SDL_Surface *minimap;     // Carte réduite
    int **collision;          // Matrice de collision
    int width, height;        // Dimensions en tuiles
    int offset_x;             // Décalage horizontal pour le scrolling
    int offset_y;             // Décalage verticale pour le scrolling
    Car *car;
    int num_car;
} Map;
// Structure de l'ennemi
typedef struct {
    SDL_Surface *icon;       // Icône principale
    SDL_Surface *mini_icon;  // Icône pour la minimap
    int x, y;                // Position sur la carte
    float velocity_x;        // Ajouter la vélocité horizontale
    int direction;           // Direction du mouvement (1 = droite, -1 = gauche)
    int moving_range; // <-- Ajouter pour limiter la zone de mouvement
} Enemy;

// Prototypes de fonctions
void InitSDL();
void LoadMap(Map *map, const char *bg_path, const char *minimap_path);
void FreeMap(Map *map);
void DrawGame(SDL_Surface *screen, Map *map, Player *player);
int CheckCollision(Map *map, float x, float y);
void InitPlayer(Player *player, const char *sprite_path, const char *icon_path);
void FreePlayer(Player *player);
void UpdatePhysics(Player *player, Map *map);
void HandleInput(Player *player, Map *map);
void UpdateCamera(Map *map, Player *player, float delta_time);
time_t init_time();
int time_expired(int debut,int limit);//retourne 1 si le temps a éxpiré sinon 0
void show_time(int debut,SDL_Surface *screen);
void InitEnemy(Enemy *enemy, Map *map);
void DrawEnemy(SDL_Surface *screen, Map *map, Enemy *enemy);
int CheckPlayerEnemyCollision(Player *player, Enemy *enemy);
void FreeEnemy(Enemy *enemy);
void UpdateEnemy(Enemy *enemy, Map *map);
int CheckCarCollision(Player *player, Car *car);
#endif