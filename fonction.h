
#ifndef PLATFORMER_H
#define PLATFORMER_H

#include <SDL/SDL_ttf.h>
#include <SDL/SDL.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL_image.h>
#include <math.h> 

#define TILE_SIZE 32         
#define GRAVITY 0.73f
#define JUMP_FORCE -20.0f     
#define PLAYER_SPEED 15     
#define MINIMAP_SCALE 0.20f   
#define SCREEN_WIDTH 1920  
#define SCREEN_HEIGHT 1080
#define MAP_WIDTH_TILES 20
#define MAP_HEIGHT_TILES 20
#define MAX_HEALTH 3

typedef struct {
    float x, y;
    float vx, vy;
    int lifetime;
    int max_lifetime;
    SDL_Color color;
    int size;
} Particle;

typedef struct {
    Particle *particles;
    int capacity;
    int count;
} ParticleSystem;


typedef struct {
    SDL_Surface *sprite;
    SDL_Surface *minimap_icon;
    float x, y;
    int width, height;
    float velocity_y;
    int is_grounded;
    int health;
    Uint32 shake_timer;
    Uint32 shrink_timer;
    Uint32 hit_timer;
    int is_hit;
    ParticleSystem particle_system;
    const Uint8 *key_state;
} Player;

typedef struct {
    SDL_Rect collision_box;
    int is_active;
} Car;

typedef struct {
    SDL_Surface *bg;
    SDL_Surface *minimap;
    int **collision;
    int width, height;
    int offset_x;
    int offset_y;
    Car *car;
    int num_car;
    Uint32 shake_duration;
} Map;

typedef struct {
    SDL_Surface *icon;
    SDL_Surface *mini_icon;
    int x, y;
    float velocity_x;
    int direction;
    int moving_range;
   
} Enemy;
typedef enum { WEATHER_RAIN, WEATHER_SNOW } WeatherType;

typedef struct {
    ParticleSystem system;
    WeatherType type;
    Uint32 last_spawn;
} WeatherSystem;


// Prototypes
void InitParticleSystem(ParticleSystem *ps, int capacity);
void FreeParticleSystem(ParticleSystem *ps);
void AddParticle(ParticleSystem *ps, float x, float y, SDL_Color color, float vx, float vy, int lifetime, int size);
void UpdateParticles(ParticleSystem *ps, float delta_time);
void DrawParticles(ParticleSystem *ps, SDL_Surface *screen, Map *map);
int CheckCollisionRect(const SDL_Rect *a, const SDL_Rect *b);

void InitSDL();
void LoadMap(Map *map, const char *bg_path, const char *minimap_path);
void FreeMap(Map *map);
void DrawGame(SDL_Surface *screen, Map *map, Player *player);
int CheckCollision(Map *map, float x, float y);
void InitPlayer(Player *player, const char *sprite_path, const char *icon_path);
void FreePlayer(Player *player);
void UpdatePhysics(Player *player, Map *map, const Uint8 *keys, float delta_time);
void HandleInput(Player *player, Map *map, const Uint8 *keys);
void UpdateCamera(Map *map, Player *player, float delta_time);
time_t init_time();
int time_expired(int debut,int limit);
void show_time(int debut, SDL_Surface *screen, Player *player);
void InitEnemy(Enemy *enemy, Map *map);
void DrawEnemy(SDL_Surface *screen, Map *map, Enemy *enemy);
int CheckPlayerEnemyCollision(Player *player, Enemy *enemy);
void FreeEnemy(Enemy *enemy);
void UpdateEnemy(Enemy *enemy, Map *map);
int CheckCarCollision(Player *player, Car *car);
void HandleShakeEffect(Map *map);
void UpdatePlayerHitEffect(Player *player);

void InitWeather(WeatherSystem* weather, int capacity, WeatherType type);
void DrawWeather(WeatherSystem* weather, SDL_Surface* screen, Map* map);
void FreeWeather(WeatherSystem* weather);
void UpdateWeather(WeatherSystem* weather, int map_w, int map_h, int camera_x, int camera_y, float delta_time);
#endif

