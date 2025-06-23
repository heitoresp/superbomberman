#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

// Definições de constantes
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800
#define GRID_SIZE 15
#define TILE_SIZE 40
#define MAX_ENEMIES 10
#define MAX_BOMBS 5
#define MAX_LEVELS 5
#define MAX_EXPLOSIONS 100

// Tipos de texturas
typedef enum {
    TEX_EMPTY,
    TEX_INDESTRUCTIBLE,
    TEX_DESTRUCTIBLE,
    TEX_EXIT,
    TEX_BOMB_POWERUP,
    TEX_RANGE_POWERUP,
    TEX_PLAYER,
    TEX_ENEMY,
    TEX_BOMB,
    TEX_EXPLOSION,
    NUM_TEXTURES
} TextureType;

// Tipos de células
typedef enum {
    EMPTY,
    INDESTRUCTIBLE,
    DESTRUCTIBLE,
    EXIT,
    BOMB_POWERUP,
    RANGE_POWERUP
} TileType;

// Estrutura do jogador
typedef struct {
    float realX, realY; // Posição real para interpolação
    int x, y;           // Posição no grid
    int max_bombs;
    int bomb_range;
    bool alive;
    int direction;      // 0: direita, 1: esquerda, 2: cima, 3: baixo
} Player;

// Estrutura do inimigo
typedef struct {
    float realX, realY; // Posição real para interpolação
    int x, y;           // Posição no grid
    bool alive;
    int move_timer;
} Enemy;

// Estrutura da bomba
typedef struct {
    int x, y;
    int timer;
    int range;
    bool exploded;
} Bomb;

// Estrutura da explosão
typedef struct {
    int x, y;
    int timer;
} Explosion;

// Estrutura do jogo
typedef struct {
    Player player;
    Enemy enemies[MAX_ENEMIES];
    int enemy_count;
    Bomb bombs[MAX_BOMBS];
    int bomb_count;
    Explosion explosions[MAX_EXPLOSIONS];
    int explosion_count;
    TileType grid[GRID_SIZE][GRID_SIZE];
    TileType hiddenGrid[GRID_SIZE][GRID_SIZE];
    int level;
    int score;
    bool game_over;
    bool level_complete;
    Texture2D textures[NUM_TEXTURES]; // Texturas do jogo
} GameState;

// Estrutura do menu
typedef enum {
    MAIN_MENU,
    PLAYING,
    GAME_OVER,
    LEVEL_COMPLETE,
    LOAD_MAP
} GameScreen;

// Protótipos de funções
void InitGame(GameState *game, int level);
void GenerateLevel(GameState *game);
void DrawGame(GameState *game);
void UpdateGame(GameState *game);
void PlantBomb(GameState *game);
void ExplodeBomb(GameState *game, Bomb *bomb);
void MoveEnemies(GameState *game);
void LoadCustomMap(GameState *game, const char *filename);
void SaveGame(GameState *game);
bool LoadGame(GameState *game);
void ResetGame(GameState *game);

int main(void) {
    // Inicialização da janela
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Mini Bomberman");
    SetTargetFPS(60);

    // Inicialização de variáveis
    GameState game;
    memset(&game, 0, sizeof(GameState)); // Garantir inicialização

    // Carregar texturas
    game.textures[TEX_EMPTY] = LoadTexture("assets/empty.png");
    game.textures[TEX_INDESTRUCTIBLE] = LoadTexture("assets/indestructible.png");
    game.textures[TEX_DESTRUCTIBLE] = LoadTexture("assets/destructible.png");
    game.textures[TEX_EXIT] = LoadTexture("assets/exit.png");
    game.textures[TEX_BOMB_POWERUP] = LoadTexture("assets/bomb_powerup.png");
    game.textures[TEX_RANGE_POWERUP] = LoadTexture("assets/range_powerup.png");
    game.textures[TEX_PLAYER] = LoadTexture("assets/player.png");
    game.textures[TEX_ENEMY] = LoadTexture("assets/enemy.png");
    game.textures[TEX_BOMB] = LoadTexture("assets/bomb.png");
    game.textures[TEX_EXPLOSION] = LoadTexture("assets/explosion.png");

    GameScreen currentScreen = MAIN_MENU;
    bool saveFileExists = false;
    char mapFilename[256] = {0};

    // Verifica se existe arquivo de save
    saveFileExists = LoadGame(&game);

    // Loop principal do jogo
    while (!WindowShouldClose()) {
        // Atualização do jogo
        switch (currentScreen) {
            case MAIN_MENU:
                if (IsKeyPressed(KEY_ONE)) {
                    ResetGame(&game);
                    InitGame(&game, 1);
                    currentScreen = PLAYING;
                }
                else if (IsKeyPressed(KEY_TWO) && saveFileExists) {
                    if (LoadGame(&game)) {
                        currentScreen = PLAYING;
                    }
                }
                else if (IsKeyPressed(KEY_THREE)) {
                    currentScreen = LOAD_MAP;
                }
                else if (IsKeyPressed(KEY_FOUR)) {
                    CloseWindow();
                    return 0;
                }
                break;

            case LOAD_MAP:
                if (IsKeyPressed(KEY_ENTER) && mapFilename[0] != '\0') {
                    LoadCustomMap(&game, mapFilename);
                    currentScreen = PLAYING;
                }
                else {
                    // Captura de entrada de texto para nome do arquivo
                    int key = GetCharPressed();
                    while (key > 0) {
                        if (key >= 32 && key <= 125 && strlen(mapFilename) < 255) {
                            strncat(mapFilename, (char*)&key, 1);
                        }
                        key = GetCharPressed();
                    }
                    if (IsKeyPressed(KEY_BACKSPACE) && strlen(mapFilename) > 0) {
                        mapFilename[strlen(mapFilename)-1] = '\0';
                    }
                }
                break;

            case PLAYING:
                if (!game.game_over && !game.level_complete) {
                    UpdateGame(&game);
                }
                else if (game.game_over) {
                    if (IsKeyPressed(KEY_ENTER)) {
                        ResetGame(&game);
                        InitGame(&game, 1);
                        currentScreen = PLAYING;
                    }
                    else if (IsKeyPressed(KEY_ESCAPE)) {
                        currentScreen = MAIN_MENU;
                    }
                }
                else if (game.level_complete) {
                    if (IsKeyPressed(KEY_ENTER)) {
                        SaveGame(&game);
                        game.level++;
                        InitGame(&game, game.level);
                        currentScreen = PLAYING;
                    }
                }
                break;

            case GAME_OVER:
                if (IsKeyPressed(KEY_ENTER)) {
                    ResetGame(&game);
                    InitGame(&game, 1);
                    currentScreen = PLAYING;
                }
                else if (IsKeyPressed(KEY_ESCAPE)) {
                    currentScreen = MAIN_MENU;
                }
                break;

            case LEVEL_COMPLETE:
                if (IsKeyPressed(KEY_ENTER)) {
                    SaveGame(&game);
                    game.level++;
                    InitGame(&game, game.level);
                    currentScreen = PLAYING;
                }
                break;
        }

        // Desenho
        BeginDrawing();
            ClearBackground(RAYWHITE);

            switch (currentScreen) {
                case MAIN_MENU:
                    DrawText("MINI BOMBERMAN", SCREEN_WIDTH/2 - 150, 100, 40, BLACK);
                    DrawText("1. Jogar", SCREEN_WIDTH/2 - 50, 200, 20, BLACK);

                    if (saveFileExists) {
                        DrawText("2. Continuar", SCREEN_WIDTH/2 - 50, 230, 20, BLACK);
                    } else {
                        DrawText("2. Continuar (indisponivel)", SCREEN_WIDTH/2 - 50, 230, 20, GRAY);
                    }

                    DrawText("3. Carregar Mapa", SCREEN_WIDTH/2 - 50, 260, 20, BLACK);
                    DrawText("4. Sair", SCREEN_WIDTH/2 - 50, 290, 20, BLACK);
                    break;

                case LOAD_MAP:
                    DrawText("Digite o nome do arquivo do mapa:", SCREEN_WIDTH/2 - 150, 200, 20, BLACK);
                    DrawText(mapFilename, SCREEN_WIDTH/2 - 100, 230, 20, BLACK);
                    DrawText("Pressione ENTER para carregar", SCREEN_WIDTH/2 - 150, 300, 20, DARKGRAY);
                    break;

                case PLAYING:
                    DrawGame(&game);

                    // Desenhar informações da UI
                    DrawText(TextFormat("Fase: %d", game.level), 10, 10, 20, BLACK);
                    DrawText(TextFormat("Score: %d", game.score), 10, 40, 20, BLACK);
                    DrawText(TextFormat("Bombas: %d/%d", game.player.max_bombs - game.bomb_count, game.player.max_bombs), 10, 70, 20, BLACK);
                    DrawText(TextFormat("Alcance: %d", game.player.bomb_range), 10, 100, 20, BLACK);

                    if (game.game_over) {
                        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.5f));
                        DrawText("GAME OVER", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 50, 40, RED);
                        DrawText("Pressione ENTER para jogar novamente", SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 + 10, 20, WHITE);
                        DrawText("Pressione ESC para voltar ao menu", SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT/2 + 40, 20, WHITE);
                    }
                    else if (game.level_complete) {
                        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.5f));
                        DrawText("FASE COMPLETA!", SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 - 50, 40, GREEN);
                        DrawText("Pressione ENTER para avancar", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 10, 20, WHITE);
                    }
                    break;
            }
        EndDrawing();
    }

    // Desinicialização
    for (int i = 0; i < NUM_TEXTURES; i++) {
        UnloadTexture(game.textures[i]);
    }
    CloseWindow();
    return 0;
}

void InitGame(GameState *game, int level) {
    // Manter power-ups se não for o nível 1
    int max_bombs = (level == 1) ? 1 : game->player.max_bombs;
    int bomb_range = (level == 1) ? 2 : game->player.bomb_range;

    game->level = level;
    game->score = (level == 1) ? 0 : game->score; // Resetar score apenas no nível 1
    game->game_over = false;
    game->level_complete = false;
    game->bomb_count = 0;
    game->explosion_count = 0;

    // Inicializar jogador
    game->player.realX = 1.0f;
    game->player.realY = 1.0f;
    game->player.x = 1;
    game->player.y = 1;
    game->player.max_bombs = max_bombs;
    game->player.bomb_range = bomb_range;
    game->player.alive = true;
    game->player.direction = 0; // Direita

    // Gerar nível
    GenerateLevel(game);
}

void GenerateLevel(GameState *game) {
    // Inicializar grid com vazio
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            game->grid[y][x] = EMPTY;
            game->hiddenGrid[y][x] = EMPTY;
        }
    }

    // Adicionar paredes indestrutíveis nas bordas e em posições internas
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            if (y == 0 || y == GRID_SIZE-1 || x == 0 || x == GRID_SIZE-1) {
                game->grid[y][x] = INDESTRUCTIBLE;
            }
            else if (y % 2 == 0 && x % 2 == 0) {
                game->grid[y][x] = INDESTRUCTIBLE;
            }
        }
    }

    // Adicionar paredes destrutíveis aleatórias
    int destructibleWalls = 40 + game->level * 5;
    for (int i = 0; i < destructibleWalls; i++) {
        int x = rand() % (GRID_SIZE-2) + 1;
        int y = rand() % (GRID_SIZE-2) + 1;

        // Não colocar em cima do jogador ou em posições fixas
        if ((x == 1 && y == 1) || (x == 1 && y == 2) || (x == 2 && y == 1)) {
            continue;
        }

        if (game->grid[y][x] == EMPTY) {
            game->grid[y][x] = DESTRUCTIBLE;
        }
    }

    // Esconder saída e power-ups sob paredes destrutíveis
    int exitX, exitY;
    do {
        exitX = rand() % (GRID_SIZE-2) + 1;
        exitY = rand() % (GRID_SIZE-2) + 1;
    } while (game->grid[exitY][exitX] != DESTRUCTIBLE);
    game->hiddenGrid[exitY][exitX] = EXIT;

    int bombPowerX, bombPowerY;
    do {
        bombPowerX = rand() % (GRID_SIZE-2) + 1;
        bombPowerY = rand() % (GRID_SIZE-2) + 1;
    } while (game->grid[bombPowerY][bombPowerX] != DESTRUCTIBLE);
    game->hiddenGrid[bombPowerY][bombPowerX] = BOMB_POWERUP;

    int rangePowerX, rangePowerY;
    do {
        rangePowerX = rand() % (GRID_SIZE-2) + 1;
        rangePowerY = rand() % (GRID_SIZE-2) + 1;
    } while (game->grid[rangePowerY][rangePowerX] != DESTRUCTIBLE);
    game->hiddenGrid[rangePowerY][rangePowerX] = RANGE_POWERUP;

    // Inicializar inimigos com distância mínima do jogador
    game->enemy_count = 2 + game->level;
    for (int i = 0; i < game->enemy_count; i++) {
        int x, y;
        int attempts = 0;
        int dx, dy;

        do {
            x = rand() % (GRID_SIZE-2) + 1;
            y = rand() % (GRID_SIZE-2) + 1;
            attempts++;

            dx = abs(x - game->player.x);
            dy = abs(y - game->player.y);

            if (attempts > 100) break;
        } while (game->grid[y][x] != EMPTY ||
                 (x == 1 && y == 1) ||
                 (dx < 3 && dy < 3));

        game->enemies[i].realX = (float)x;
        game->enemies[i].realY = (float)y;
        game->enemies[i].x = x;
        game->enemies[i].y = y;
        game->enemies[i].alive = true;
        game->enemies[i].move_timer = 0;
    }
}

void DrawGame(GameState *game) {
    // Desenhar grid
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            Vector2 position = {
                x * TILE_SIZE + (SCREEN_WIDTH - GRID_SIZE * TILE_SIZE) / 2,
                y * TILE_SIZE + 50
            };

            switch (game->grid[y][x]) {
                case EMPTY:
                    DrawTextureV(game->textures[TEX_EMPTY], position, WHITE);
                    break;
                case INDESTRUCTIBLE:
                    DrawTextureV(game->textures[TEX_INDESTRUCTIBLE], position, WHITE);
                    break;
                case DESTRUCTIBLE:
                    DrawTextureV(game->textures[TEX_DESTRUCTIBLE], position, WHITE);
                    break;
                case EXIT:
                    DrawTextureV(game->textures[TEX_EXIT], position, WHITE);
                    break;
                case BOMB_POWERUP:
                    DrawTextureV(game->textures[TEX_BOMB_POWERUP], position, WHITE);
                    break;
                case RANGE_POWERUP:
                    DrawTextureV(game->textures[TEX_RANGE_POWERUP], position, WHITE);
                    break;
            }
        }
    }

    // Desenhar explosões
    for (int i = 0; i < game->explosion_count; i++) {
        Vector2 position = {
            game->explosions[i].x * TILE_SIZE + (SCREEN_WIDTH - GRID_SIZE * TILE_SIZE) / 2,
            game->explosions[i].y * TILE_SIZE + 50
        };
        DrawTextureV(game->textures[TEX_EXPLOSION], position, WHITE);
    }

    // Desenhar jogador
    if (game->player.alive) {
        Vector2 position = {
            game->player.realX * TILE_SIZE + (SCREEN_WIDTH - GRID_SIZE * TILE_SIZE) / 2,
            game->player.realY * TILE_SIZE + 50
        };
        DrawTextureV(game->textures[TEX_PLAYER], position, WHITE);
    }

    // Desenhar inimigos
    for (int i = 0; i < game->enemy_count; i++) {
        if (game->enemies[i].alive) {
            Vector2 position = {
                game->enemies[i].realX * TILE_SIZE + (SCREEN_WIDTH - GRID_SIZE * TILE_SIZE) / 2,
                game->enemies[i].realY * TILE_SIZE + 50
            };
            DrawTextureV(game->textures[TEX_ENEMY], position, WHITE);
        }
    }

    // Desenhar bombas
    for (int i = 0; i < game->bomb_count; i++) {
        if (!game->bombs[i].exploded) {
            Vector2 position = {
                game->bombs[i].x * TILE_SIZE + (SCREEN_WIDTH - GRID_SIZE * TILE_SIZE) / 2,
                game->bombs[i].y * TILE_SIZE + 50
            };
            DrawTextureV(game->textures[TEX_BOMB], position, WHITE);
        }
    }
}

void UpdateGame(GameState *game) {
    // Movimentação do jogador
    if (game->player.alive) {
        int targetX = game->player.x;
        int targetY = game->player.y;

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
            targetX++;
            game->player.direction = 0;
        }
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
            targetX--;
            game->player.direction = 1;
        }
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            targetY--;
            game->player.direction = 2;
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            targetY++;
            game->player.direction = 3;
        }

        // Verificar se a movimentação é válida
        if (targetX != game->player.x || targetY != game->player.y) {
            if (targetX >= 0 && targetX < GRID_SIZE && targetY >= 0 && targetY < GRID_SIZE) {
                TileType tile = game->grid[targetY][targetX];

                // Verificar se há bomba no caminho
                bool hasBomb = false;
                for (int i = 0; i < game->bomb_count; i++) {
                    if (!game->bombs[i].exploded &&
                        game->bombs[i].x == targetX &&
                        game->bombs[i].y == targetY) {
                        hasBomb = true;
                        break;
                    }
                }

                if (!hasBomb && (tile == EMPTY || tile == EXIT || tile == BOMB_POWERUP || tile == RANGE_POWERUP)) {
                    game->player.x = targetX;
                    game->player.y = targetY;
                }
            }
        }

        // Interpolação suave da posição
        float speed = 5.0f * GetFrameTime();
        game->player.realX += (game->player.x - game->player.realX) * speed;
        game->player.realY += (game->player.y - game->player.realY) * speed;

        // Coletar power-ups ao passar sobre eles
        TileType currentTile = game->grid[game->player.y][game->player.x];
        if (currentTile == BOMB_POWERUP) {
            game->player.max_bombs++;
            game->grid[game->player.y][game->player.x] = EMPTY;
        }
        else if (currentTile == RANGE_POWERUP) {
            game->player.bomb_range++;
            game->grid[game->player.y][game->player.x] = EMPTY;
        }
        else if (currentTile == EXIT) {
            // Verificar se todos os inimigos estão mortos
            bool allEnemiesDead = true;
            for (int i = 0; i < game->enemy_count; i++) {
                if (game->enemies[i].alive) {
                    allEnemiesDead = false;
                    break;
                }
            }

            if (allEnemiesDead) {
                game->level_complete = true;
            }
        }

        // Plantar bomba
        if (IsKeyPressed(KEY_SPACE)) {
            PlantBomb(game);
        }
    }

    // Atualizar bombas
    for (int i = 0; i < game->bomb_count; i++) {
        if (!game->bombs[i].exploded) {
            game->bombs[i].timer--;

            if (game->bombs[i].timer <= 0) {
                ExplodeBomb(game, &game->bombs[i]);
            }
        }
    }

    // Remover bombas explodidas
    for (int i = 0; i < game->bomb_count; i++) {
        if (game->bombs[i].exploded) {
            for (int j = i; j < game->bomb_count - 1; j++) {
                game->bombs[j] = game->bombs[j+1];
            }
            game->bomb_count--;
            i--;
        }
    }

    // Atualizar explosões
    for (int i = 0; i < game->explosion_count; i++) {
        game->explosions[i].timer--;
        if (game->explosions[i].timer <= 0) {
            for (int j = i; j < game->explosion_count - 1; j++) {
                game->explosions[j] = game->explosions[j+1];
            }
            game->explosion_count--;
            i--;
        }
    }

    // Movimentar inimigos
    MoveEnemies(game);

    // Verificar colisão entre jogador e inimigos
    for (int i = 0; i < game->enemy_count; i++) {
        if (game->enemies[i].alive &&
            game->player.x == game->enemies[i].x &&
            game->player.y == game->enemies[i].y) {
            game->player.alive = false;
            game->game_over = true;
        }
    }
}

void PlantBomb(GameState *game) {
    if (game->bomb_count < game->player.max_bombs) {
        // Verificar se já não há bomba nesta posição
        bool bombAlreadyHere = false;
        for (int i = 0; i < game->bomb_count; i++) {
            if (game->bombs[i].x == game->player.x && game->bombs[i].y == game->player.y) {
                bombAlreadyHere = true;
                break;
            }
        }

        if (!bombAlreadyHere) {
            Bomb newBomb = {
                .x = game->player.x,
                .y = game->player.y,
                .timer = 180, // 3 segundos (60 FPS * 3)
                .range = game->player.bomb_range,
                .exploded = false
            };

            game->bombs[game->bomb_count] = newBomb;
            game->bomb_count++;
        }
    }
}

void ExplodeBomb(GameState *game, Bomb *bomb) {
    bomb->exploded = true;

    // Adicionar explosão central
    if (game->explosion_count < MAX_EXPLOSIONS) {
        game->explosions[game->explosion_count].x = bomb->x;
        game->explosions[game->explosion_count].y = bomb->y;
        game->explosions[game->explosion_count].timer = 60; // 1 segundo
        game->explosion_count++;
    }

    // Explosão central - verifica se jogador ainda está na posição
    if (bomb->x == game->player.x && bomb->y == game->player.y) {
        game->player.alive = false;
        game->game_over = true;
    }

    // Direções: cima, baixo, esquerda, direita
    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};

    for (int d = 0; d < 4; d++) {
        for (int r = 1; r <= bomb->range; r++) {
            int x = bomb->x + dx[d] * r;
            int y = bomb->y + dy[d] * r;

            // Verificar limites
            if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) break;

            TileType tile = game->grid[y][x];

            // Parar em paredes indestrutíveis
            if (tile == INDESTRUCTIBLE) break;

            // Adicionar explosão
            if (game->explosion_count < MAX_EXPLOSIONS) {
                game->explosions[game->explosion_count].x = x;
                game->explosions[game->explosion_count].y = y;
                game->explosions[game->explosion_count].timer = 60; // 1 segundo
                game->explosion_count++;
            }

            // Destruir paredes destrutíveis e revelar itens
            if (tile == DESTRUCTIBLE) {
                // Revelar item escondido se existir
                if (game->hiddenGrid[y][x] != EMPTY) {
                    game->grid[y][x] = game->hiddenGrid[y][x];
                    game->hiddenGrid[y][x] = EMPTY;
                } else {
                    game->grid[y][x] = EMPTY;
                }
                break; // A explosão para após destruir a parede
            }

            // Matar inimigos
            for (int i = 0; i < game->enemy_count; i++) {
                if (game->enemies[i].alive &&
                    game->enemies[i].x == x &&
                    game->enemies[i].y == y) {
                    game->enemies[i].alive = false;
                    game->score += 100;
                }
            }

            // Matar jogador
            if (game->player.x == x && game->player.y == y) {
                game->player.alive = false;
                game->game_over = true;
            }

            // Detonar outras bombas
            for (int i = 0; i < game->bomb_count; i++) {
                if (!game->bombs[i].exploded &&
                    game->bombs[i].x == x &&
                    game->bombs[i].y == y) {
                    game->bombs[i].timer = 0;
                }
            }
        }
    }
}

void MoveEnemies(GameState *game) {
    for (int i = 0; i < game->enemy_count; i++) {
        if (game->enemies[i].alive) {
            game->enemies[i].move_timer++;

            if (game->enemies[i].move_timer >= 30) { // Mover a cada 0.5 segundos
                game->enemies[i].move_timer = 0;

                // IA simples: mover aleatoriamente
                int direction = rand() % 4;
                int newX = game->enemies[i].x;
                int newY = game->enemies[i].y;

                switch (direction) {
                    case 0: newX++; break; // Direita
                    case 1: newX--; break; // Esquerda
                    case 2: newY++; break; // Baixo
                    case 3: newY--; break; // Cima
                }

                // Verificar se o movimento é válido
                if (newX >= 0 && newX < GRID_SIZE && newY >= 0 && newY < GRID_SIZE) {
                    TileType tile = game->grid[newY][newX];

                    // Verificar se há bomba no caminho
                    bool hasBomb = false;
                    for (int j = 0; j < game->bomb_count; j++) {
                        if (!game->bombs[j].exploded &&
                            game->bombs[j].x == newX &&
                            game->bombs[j].y == newY) {
                            hasBomb = true;
                            break;
                        }
                    }

                    if (!hasBomb && (tile == EMPTY || tile == EXIT || tile == BOMB_POWERUP || tile == RANGE_POWERUP)) {
                        game->enemies[i].x = newX;
                        game->enemies[i].y = newY;
                    }
                }
            }

            // Interpolação suave da posição
            float speed = 5.0f * GetFrameTime();
            game->enemies[i].realX += (game->enemies[i].x - game->enemies[i].realX) * speed;
            game->enemies[i].realY += (game->enemies[i].y - game->enemies[i].realY) * speed;
        }
    }
}

void LoadCustomMap(GameState *game, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return;

    // Resetar jogo
    ResetGame(game);
    game->level = 1;

    // Inicializar hiddenGrid
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            game->hiddenGrid[y][x] = EMPTY;
        }
    }

    // Ler mapa do arquivo
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            char c = fgetc(file);
            switch (c) {
                case ' ': game->grid[y][x] = EMPTY; break;
                case 'W': game->grid[y][x] = INDESTRUCTIBLE; break;
                case 'B': game->grid[y][x] = DESTRUCTIBLE; break;
                default: game->grid[y][x] = EMPTY;
            }
        }
        fgetc(file); // Pular nova linha
    }

    // Posicionar jogador
    game->player.realX = 1.0f;
    game->player.realY = 1.0f;
    game->player.x = 1;
    game->player.y = 1;

    // Adicionar alguns inimigos
    game->enemy_count = 3;
    for (int i = 0; i < game->enemy_count; i++) {
        game->enemies[i].realX = 5.0f + i * 2;
        game->enemies[i].realY = 5.0f;
        game->enemies[i].x = 5 + i * 2;
        game->enemies[i].y = 5;
        game->enemies[i].alive = true;
        game->enemies[i].move_timer = 0;
    }

    fclose(file);
}

void SaveGame(GameState *game) {
    FILE *file = fopen("save.bin", "wb");
    if (!file) return;

    fwrite(game, sizeof(GameState), 1, file);
    fclose(file);
}

bool LoadGame(GameState *game) {
    FILE *file = fopen("save.bin", "rb");
    if (!file) return false;

    fread(game, sizeof(GameState), 1, file);
    fclose(file);
    return true;
}

void ResetGame(GameState *game) {
    memset(game, 0, sizeof(GameState));
}
