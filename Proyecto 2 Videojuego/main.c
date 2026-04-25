		/* USER CODE BEGIN Header */
			/**
			 ******************************************************************************
			 * @file           : main.c
			 * @brief          : Main program body
			 ******************************************************************************
			 * @attention
			 *
			 * Copyright (c) 2024 STMicroelectronics.
			 * All rights reserved.
			 *
			 * This software is licensed under terms that can be found in the LICENSE file
			 * in the root directory of this software component.
			 * If no LICENSE file comes with this software, it is provided AS-IS.
			 *
			 ******************************************************************************
			 */
		/* USER CODE END Header */
		/* Includes ------------------------------------------------------------------*/
		#include "main.h"
		#include "fatfs.h"

		/* Private includes ----------------------------------------------------------*/
		/* USER CODE BEGIN Includes */
		#include "ili9341.h"
		#include "Bitmaps.h"
		#include "fatfs_sd.h"
		#include "string.h"
		#include "stdio.h"
		#include <stdlib.h>
		#include <stdbool.h>
		/* USER CODE END Includes */

		/* Private typedef -----------------------------------------------------------*/
		/* USER CODE BEGIN PTD */

		/* USER CODE END PTD */

		/* Private define ------------------------------------------------------------*/
		/* USER CODE BEGIN PD */
			//Definiciones SPI y SD
			FATFS fs;
			FATFS *pfs;
			FIL fill;
			FRESULT fres;
			DWORD fre_clust;
			uint32_t totalSpace, freeSpace;
			char buffer[100];

			// Definiciones para colisiones
			#define Pico_altura 36
			#define Pico_ancho 30

			#define Puerta_altura 38
			#define Puerta_ancho 40

			#define Fondo_altura 240
			#define Fondo_ancho 320

			#define Caja_nivel_altura 36
			#define Caja_nivel_ancho 36

			#define Numero_altura 12
			#define Numero_ancho 12

			#define caja_pico_WIDTH 25
			#define caja_pico_HEIGHT 25

			#define caja_grande_pico_WIDTH 50
			#define caja_grande_pico_HEIGHT 50

			#define boton_pico_WIDTH 34
			#define boton_pico_HEIGHT 14

			#define bloques_pico_WIDTH 15
			#define bloques_pico_HEIGHT 15

			#define Saltarines_pico_WIDTH 20
			#define Saltarines_pico_HEIGHT 20

			#define Puerta_Pico_WIDTH 40
			#define Puerta_Pico_HEIGHT 38

			#define Moneda_Pico_WIDTH 20
			#define Moneda_Pico_HEIGHT 28

			#define Llave_pico_WIDTH 20
			#define Llave_pico_HEIGHT 28

			#define Elevadores_pico_WIDTH 90
			#define Elevadores_pico_HEIGHT 50

			#define Elevador_siempre_pico_WIDTH 60
			#define Elevador_siempre_pico_HEIGHT 8

			#define Bloque_empujar_pico_WIDTH 30
			#define Bloque_empujar_pico_HEIGHT 220

			#define PICO_TRANSPARENT_KEY 0xF81F


			#define FLOOR_Y      (240 - 10 - Pico_altura)
			#define LEFT_WALL    10
			#define RIGHT_WALL   (308 - Pico_ancho)
			#define TOP_WALL     0


			// Estados del jugador
			#define FRAME_IDLE   0
			#define FRAME_JUMP   5

			#define JUMP_HEIGHT  42
			#define JUMP_STEP    2


			//            Definiciones nivel
			#define LEVEL_COUNT 8
			#define LEVEL_COLS  4
			#define LEVEL_ROWS  2

			#define BOX_START_X 61
			#define BOX_START_Y 80
			#define BOX_GAP_X   54
			#define BOX_GAP_Y   54

			// Definiciones para lectura de archivos SD
			#define MAP_BUFFER_SIZE         24576
			#define MAX_SCENE_OBJECTS       120
			#define MAX_ASSET_PIXELS        (144 * 188)   // ajusta al asset más grande que cargarás
			#define ASSET_TRANSPARENT_KEY   0xF81F

			#define BG_MENU_INDEX           0
			#define BG_LEVEL_INDEX          1

		// IDs simples. Si quieres, luego los cambias por el header game_asset_ids.h
		#define ASSET_ID_JUMPERS            1
		#define ASSET_ID_PUSH_BLOCK         2
		#define ASSET_ID_GROUND_BLOCKS      3
		#define ASSET_ID_BUTTON             4
		#define ASSET_ID_BIG_BOX            5
		#define ASSET_ID_BOX                6
		#define ASSET_ID_ELEVATOR_STATIC    7
		#define ASSET_ID_ELEVATORS          8
		#define ASSET_ID_KEY                9
		#define ASSET_ID_COIN_STRIP         10
		#define ASSET_ID_DOOR               11
		/* USER CODE END PD */

		/* Private macro -------------------------------------------------------------*/
		/* USER CODE BEGIN PM */

		/* USER CODE END PM */

		/* Private variables ---------------------------------------------------------*/
		SPI_HandleTypeDef hspi1;

		UART_HandleTypeDef huart5;
		UART_HandleTypeDef huart2;
		UART_HandleTypeDef huart3;

		/* USER CODE BEGIN PV */

			uint8_t rx3 = 0;
			volatile uint8_t moveCmd = 'S';
			volatile uint8_t jumpEvent = 0;

			int keyObjectIndex = -1;
			int doorObjectIndex = -1;
			uint8_t keyCollected = 0;

			int playerX = 20;
			int playerY = 0;
			int prevX   = 20;
			int prevY   = 0;

			uint8_t currentLevel = 0;
			uint8_t faceLeft = 0;
			uint8_t animStep = 0;
			uint32_t lastAnimTick = 0;

			uint8_t jumpState = 0;      // 0 = suelo, 1 = subiendo, 2 = bajando
			int jumpProgress = 0;       // cuánto ha subido del total

			const uint8_t walkFrames[3] = {2, 3, 4};


			uint8_t rx5 = 0;
			volatile uint8_t moveCmd2 = 'S';
			volatile uint8_t jumpEvent2 = 0;

			int player2X = 80;
			int player2Y = 0;
			int prev2X   = 80;
			int prev2Y   = 0;

			uint8_t faceLeft2 = 0;
			uint8_t animStep2 = 0;
			uint32_t lastAnimTick2 = 0;

			uint8_t jumpState2 = 0;
			int jumpProgress2 = 0;

			typedef struct {
			    uint8_t assetId;
			    int16_t x;
			    int16_t y;
			    uint16_t width;
			    uint16_t height;
			    uint8_t active;
			    uint8_t frame;

			    uint8_t solid;
			    uint8_t movable;
			    uint8_t collectible;
			    uint8_t trigger;
			} SceneObject;
			typedef struct {
			    int x1;
			    int y1;
			    int x2;
			    int y2;
			} LevelSpawn;


			LevelSpawn levelSpawns[] = {
			    {125, 194, 165, 194},   // Nivel 0
			    {200, 194, 235, 194},   // Nivel 1
			    {200,  40, 230,  40},   // Nivel 2
			    {30,  194, 80,  194},   // Nivel 3
			    {120,  194, 150, 194},   // Nivel 4
			    {30,  194, 70, 194},   // Nivel 5
			};

			//Logica del cambio de monitoreo del nivel
			typedef enum {
				STATE_GAME = 0,
				STATE_LEVEL_SELECT,
				STATE_PAUSE,
				STATE_LEVEL_CONFIRMED

			} AppState;

			AppState appState = STATE_GAME;

			volatile uint8_t menuEvent = 0;

			uint8_t selectedLevel = 0;
			uint8_t confirmedLevel = 0xFF;

			volatile uint8_t pauseEvent = 0;

			// snapshot de posiciones (picos = jugadores en tu caso)
			int savedPlayer1X = 0;
			int savedPlayer1Y = 0;
			int savedPlayer2X = 0;
			int savedPlayer2Y = 0;

			AppState prevStateBeforePause = STATE_GAME;
			uint8_t pauseOption = 0;

			//Variables para lectura de mapas de niveles para la SD
			char mapBuffer[MAP_BUFFER_SIZE];
			UINT mapBytesRead = 0;
			uint8_t sdReady = 0;
			uint8_t currentBgIndex = BG_MENU_INDEX;

			typedef struct {
				uint8_t id;
				const char *file;
				uint16_t width;
				uint16_t height;
			} AssetInfo;


			static const AssetInfo g_assetTable[] = {
				{ ASSET_ID_JUMPERS,         "jumpers.rgb565",          86, 12 },
				{ ASSET_ID_PUSH_BLOCK,      "push_block.rgb565",       52, 36 },
				{ ASSET_ID_GROUND_BLOCKS,   "ground_blocks.rgb565",    114, 14 },
				{ ASSET_ID_BUTTON,          "button.rgb565",           48, 16 },
				{ ASSET_ID_BIG_BOX,         "big_box.rgb565",          114, 188 },
				{ ASSET_ID_BOX,             "box.rgb565",              36, 36 },
				{ ASSET_ID_ELEVATOR_STATIC, "elevator_static.rgb565",  44, 8 },
				{ ASSET_ID_ELEVATORS,       "elevators.rgb565",        118, 44 },
				{ ASSET_ID_KEY,             "key.rgb565",              20, 32 },
				{ ASSET_ID_COIN_STRIP,      "coin_strip.rgb565",       80, 32 },
				{ ASSET_ID_DOOR,            "door.rgb565",             80, 38 }
			};



			SceneObject sceneObjects[MAX_SCENE_OBJECTS];
			uint8_t sceneObjectCount = 0;
			uint16_t assetBuffer[MAX_ASSET_PIXELS];

			uint8_t unlockedLevels[8] = {1, 0, 0, 0, 0, 0, 0, 0};
			volatile uint8_t squareEvent = 0;
			uint8_t squarePressCount = 0;
			uint8_t allLevelsUnlocked = 0;

			int buttonObjectIndex = -1;
						uint8_t buttonPressed = 0;
						int elevatorLeftIndex = -1;
						int elevatorRightIndex = -1;
						uint8_t map4AlternateJumpMode = 0;   // 1 solo en MAPA4
						uint8_t jumpTurn = 1;
		/* USER CODE END PV */

		/* Private function prototypes -----------------------------------------------*/
		void SystemClock_Config(void);
		static void MX_GPIO_Init(void);
		static void MX_SPI1_Init(void);
		static void MX_USART3_UART_Init(void);
		static void MX_UART5_Init(void);
		static void MX_USART2_UART_Init(void);
		/* USER CODE BEGIN PFP */
			void unlock_all_levels(void);
			void drawPlayer(void);
			void drawPlayer2(void);
			void updatePlayer1(void);
			void updatePlayer2(void);
			uint8_t players_collide(int x1, int y1, int x2, int y2);
			void redrawPlayersIfNeeded(void);
			FRESULT mount_sd_once(void);
			FRESULT load_text_file(const char *filename);
			FRESULT load_map_by_level(uint8_t level);
			const AssetInfo* get_asset_info(uint8_t assetId);
			FRESULT load_asset_pixels(uint8_t assetId);
			void clear_scene_objects(void);
			void add_scene_object(uint8_t assetId, int16_t x, int16_t y);
			void parse_tmj_for_static_assets(void);
			void draw_static_objects(void);
			void draw_background_region(uint8_t bgIndex, int x, int y, int w, int h);
			void restore_scene_region(int x, int y, int w, int h);
			uint8_t rects_intersect(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
			void draw_asset_from_sd(uint8_t assetId, int x, int y);
			uint8_t is_solid_at(int x, int y, int w, int h);
			uint8_t is_solid_below(int x, int y);
			uint8_t is_player_support(int upperX, int upperY, int lowerX, int lowerY);
			uint8_t players_overlap_except_support(int x1, int y1, int x2, int y2);
			void draw_scene_object(SceneObject *obj);
			void send_jump_music(void);
			void drawLevelBackgroundOnly(void);
			void drawLevelSelectScreen(void);
			void return_to_menu_after_map1(void);

			void update_key_and_door_logic(void);
						uint8_t player_pushes_from_left(int px, int py, char cmd, SceneObject *obj);
						uint8_t player_pushes_from_right(int px, int py, char cmd, SceneObject *obj);
						uint8_t push_block_can_move(uint8_t idx, int dx);
						void update_push_blocks_map1(void);
						void unlock_only_level_2(void);
						void return_to_menu_after_map1(void);
						uint8_t is_top_support_object(int px, int py, SceneObject *obj);
						uint8_t snap_player_to_top_surface(int *px, int *py);
						void update_button_key_logic_level2(void);
						void go_to_next_level(void);
						uint8_t is_player_on_elevator(SceneObject *obj, int px, int py);
						uint8_t is_rect_on_elevator_surface(int x, int y, int w, int h, SceneObject *obj);
						void update_level3_elevators(void);
		/* USER CODE END PFP */

		/* Private user code ---------------------------------------------------------*/
		/* USER CODE BEGIN 0 */

						void update_key_and_door_logic(void)
									{
							if (currentLevel == 3 && doorObjectIndex >= 0) {
							    sceneObjects[doorObjectIndex].frame = 1;
							    sceneObjects[doorObjectIndex].solid = 0;
							}
									    // Recoger llave
									    if (!keyCollected && keyObjectIndex >= 0 && sceneObjects[keyObjectIndex].active)
									    {
									        if (rects_intersect(playerX, playerY, Pico_ancho, Pico_altura,
									                            sceneObjects[keyObjectIndex].x,
									                            sceneObjects[keyObjectIndex].y,
									                            sceneObjects[keyObjectIndex].width,
									                            sceneObjects[keyObjectIndex].height)
									            ||
									            rects_intersect(player2X, player2Y, Pico_ancho, Pico_altura,
									                            sceneObjects[keyObjectIndex].x,
									                            sceneObjects[keyObjectIndex].y,
									                            sceneObjects[keyObjectIndex].width,
									                            sceneObjects[keyObjectIndex].height))
									        {
									            keyCollected = 1;
									            sceneObjects[keyObjectIndex].active = 0;

									            if (doorObjectIndex >= 0) {
									                sceneObjects[doorObjectIndex].frame = 1;
									                sceneObjects[doorObjectIndex].solid = 0;
									            }

									            LCD_Clear(0x0000);
									            drawLevelBackgroundOnly();
									            draw_static_objects();
									            drawPlayer();
									            drawPlayer2();
									        }
									    }

									    // Entrar a la puerta abierta
									    if (doorObjectIndex >= 0 && sceneObjects[doorObjectIndex].frame == 1)
									    {
									        uint8_t p1AtDoor = rects_intersect(playerX, playerY, Pico_ancho, Pico_altura,
									                                           sceneObjects[doorObjectIndex].x,
									                                           sceneObjects[doorObjectIndex].y,
									                                           sceneObjects[doorObjectIndex].width,
									                                           sceneObjects[doorObjectIndex].height);

									        uint8_t p2AtDoor = rects_intersect(player2X, player2Y, Pico_ancho, Pico_altura,
									                                           sceneObjects[doorObjectIndex].x,
									                                           sceneObjects[doorObjectIndex].y,
									                                           sceneObjects[doorObjectIndex].width,
									                                           sceneObjects[doorObjectIndex].height);

									        if ((p1AtDoor && moveCmd == 'F') || (p2AtDoor && moveCmd2 == 'F'))
									        {
									            if (confirmedLevel == 0) {
									                return_to_menu_after_map1();
									            } else {
									                go_to_next_level();
									            }
									        }
									    }
									}
						void go_to_next_level(void)
									{
									    uint8_t nextLevel = confirmedLevel + 1;

									    if (nextLevel < LEVEL_COUNT) {
									        unlockedLevels[nextLevel] = 1;
									    }

									    if (load_map_by_level(nextLevel) == FR_OK)
									    {
									        confirmedLevel = nextLevel;
									        parse_tmj_for_static_assets();

									        LevelSpawn s = levelSpawns[confirmedLevel];

									        playerX  = s.x1;
									        playerY  = s.y1 - Pico_altura;
									        player2X = s.x2;
									        player2Y = s.y2 - Pico_altura;

									        prevX  = playerX;
									        prevY  = playerY;
									        prev2X = player2X;
									        prev2Y = player2Y;

									        map4AlternateJumpMode = (confirmedLevel == 3) ? 1 : 0;
									        jumpTurn = 1;

									        if (confirmedLevel == 3)   // MAPA4
									        {
									            keyCollected = 1;

									            if (keyObjectIndex >= 0) {
									                sceneObjects[keyObjectIndex].active = 0;
									            }

									            if (doorObjectIndex >= 0) {
									                sceneObjects[doorObjectIndex].frame = 1;
									                sceneObjects[doorObjectIndex].solid = 0;
									            }

									            player2X = -100;
									            player2Y = -100;
									            prev2X = player2X;
									            prev2Y = player2Y;
									        }

									        jumpState = 0;
									        jumpState2 = 0;
									        jumpProgress = 0;
									        jumpProgress2 = 0;

									        currentBgIndex = BG_LEVEL_INDEX;
									        appState = STATE_LEVEL_CONFIRMED;

									        LCD_Clear(0x0000);
									        drawLevelBackgroundOnly();
									        draw_static_objects();
									        drawPlayer();
									        if (!map4AlternateJumpMode) {
									            drawPlayer2();
									        }
									    }
									    else
									    {
									    }
									}

			void unlock_all_levels(void)
			{
			    for (uint8_t i = 0; i < LEVEL_COUNT; i++) {
			        unlockedLevels[i] = 1;
			    }
			    allLevelsUnlocked = 1;
			}

			void send_jump_music(void)
					{
					    uint8_t msg[2] = {'J', '\n'};
					    HAL_UART_Transmit(&huart5, msg, 2, 10);
					}

			void drawPlayer(void)
			{
				uint8_t frame = FRAME_IDLE;

				if (jumpState != 0) {
					frame = FRAME_JUMP;
				} else if (moveCmd == 'L' || moveCmd == 'R') {
					if (HAL_GetTick() - lastAnimTick >= 100) {
						animStep = (animStep + 1) % 3;
						lastAnimTick = HAL_GetTick();
					}
					frame = walkFrames[animStep];
				} else {
					animStep = 0;
					frame = FRAME_IDLE;
				}

				LCD_Sprite(playerX, playerY, Pico_ancho, Pico_altura,
						   pico_verde, 11, frame, faceLeft, 0);
			}

			void drawPlayer2(void)
			{
				uint8_t frame = FRAME_IDLE;

				if (jumpState2 != 0) {
					frame = FRAME_JUMP;
				} else if (moveCmd2 == 'L' || moveCmd2 == 'R') {
					if (HAL_GetTick() - lastAnimTick2 >= 100) {
						animStep2 = (animStep2 + 1) % 3;
						lastAnimTick2 = HAL_GetTick();
					}
					frame = walkFrames[animStep2];
				} else {
					animStep2 = 0;
					frame = FRAME_IDLE;
				}

				LCD_Sprite(player2X, player2Y, Pico_ancho, Pico_altura,
						   pico_verde, 11, frame, faceLeft2, 0);
			}

			uint8_t players_collide(int x1, int y1, int x2, int y2)
			{
				return rects_intersect(x1, y1, Pico_ancho, Pico_altura,
									   x2, y2, Pico_ancho, Pico_altura);
			}

			void updatePlayer1(void)
			{
				prevX = playerX;
				prevY = playerY;

				if (moveCmd == 'L') {
					playerX -= 2;
					faceLeft = 1;
				}
				else if (moveCmd == 'R') {
					playerX += 2;
					faceLeft = 0;
				}

				if (playerX < LEFT_WALL) playerX = LEFT_WALL;
				if (playerX > RIGHT_WALL) playerX = RIGHT_WALL;

				if (jumpEvent && jumpState == 0) {
					jumpEvent = 0;
					jumpState = 1;
					jumpProgress = 0;
					send_jump_music();
				}

				if (jumpState == 1) {
					playerY -= JUMP_STEP;
					jumpProgress += JUMP_STEP;

					if (jumpProgress >= JUMP_HEIGHT ||
						is_solid_at(playerX, playerY, Pico_ancho, Pico_altura)) {

						playerY += JUMP_STEP;
						jumpState = 2;
					}
				}
				else {
					if (!is_solid_below(playerX, playerY)) {
						playerY += JUMP_STEP;
						jumpState = 2;
					}
					else {
						jumpState = 0;
						jumpProgress = 0;
					}

					if (jumpState == 2 &&
						is_player_support(playerX, playerY, player2X, player2Y)) {

						playerY = player2Y - Pico_altura;
						jumpState = 0;
						jumpProgress = 0;
					}
				}

				// 🔥 INTERACCIÓN CON BLOQUES
				for (int i = 0; i < sceneObjectCount; i++)
				{
					SceneObject *obj = &sceneObjects[i];

					if (obj->assetId == ASSET_ID_PUSH_BLOCK)
					{
						if (rects_intersect(playerX, playerY, Pico_ancho, Pico_altura,
											obj->x, obj->y, obj->width, obj->height))
						{
							if (obj->frame > 0)
							{
								obj->frame--; // 🔥 CAMBIA SPRITE
							}
						}
					}
				}

				if (playerY < TOP_WALL) playerY = TOP_WALL;
				if (playerY > FLOOR_Y) playerY = FLOOR_Y;

				if (players_overlap_except_support(playerX, playerY, player2X, player2Y)) {
					playerX = prevX;
					playerY = prevY;
				}
			}

			void updatePlayer2(void)
			{
				prev2X = player2X;
				prev2Y = player2Y;

				if (moveCmd2 == 'L') {
					player2X -= 2;
					faceLeft2 = 1;
				}
				else if (moveCmd2 == 'R') {
					player2X += 2;
					faceLeft2 = 0;
				}

				if (player2X < LEFT_WALL) player2X = LEFT_WALL;
				if (player2X > RIGHT_WALL) player2X = RIGHT_WALL;

				if (jumpEvent2 && jumpState2 == 0) {
					jumpEvent2 = 0;
					jumpState2 = 1;
					jumpProgress2 = 0;
					send_jump_music();
				}

				if (jumpState2 == 1) {
					player2Y -= JUMP_STEP;
					jumpProgress2 += JUMP_STEP;

					if (jumpProgress2 >= JUMP_HEIGHT ||
						is_solid_at(player2X, player2Y, Pico_ancho, Pico_altura)) {

						player2Y += JUMP_STEP;
						jumpState2 = 2;
					}
				}
				else {
					if (!is_solid_below(player2X, player2Y)) {
						player2Y += JUMP_STEP;
						jumpState2 = 2;
					}
					else {
						jumpState2 = 0;
						jumpProgress2 = 0;
					}

					if (jumpState2 == 2 &&
						is_player_support(player2X, player2Y, playerX, playerY)) {

						player2Y = playerY - Pico_altura;
						jumpState2 = 0;
						jumpProgress2 = 0;
					}
				}

				// 🔥 MISMA INTERACCIÓN QUE PLAYER 1
				for (int i = 0; i < sceneObjectCount; i++)
				{
					SceneObject *obj = &sceneObjects[i];

					if (obj->assetId == ASSET_ID_PUSH_BLOCK)
					{
						if (rects_intersect(player2X, player2Y, Pico_ancho, Pico_altura,
											obj->x, obj->y, obj->width, obj->height))
						{
							if (obj->frame > 0)
							{
								obj->frame--;
							}
						}
					}
				}

				if (player2Y < TOP_WALL) player2Y = TOP_WALL;
				if (player2Y > FLOOR_Y) player2Y = FLOOR_Y;

				if (players_overlap_except_support(player2X, player2Y, playerX, playerY)) {
					player2X = prev2X;
					player2Y = prev2Y;
				}
			}

			void redrawPlayersIfNeeded(void)
			{
				uint8_t p1Moved = (playerX != prevX) || (playerY != prevY);
				uint8_t p2Moved = (player2X != prev2X) || (player2Y != prev2Y);

				if (p1Moved) {
					restore_scene_region(prevX, prevY, Pico_ancho, Pico_altura);
				}

				if (p2Moved) {
					restore_scene_region(prev2X, prev2Y, Pico_ancho, Pico_altura);
				}

				if (p1Moved || p2Moved) {
					drawPlayer();
					drawPlayer2();
				}
			}

			int min_i(int a, int b)
			{
				return (a < b) ? a : b;
			}

			int max_i(int a, int b)
			{
				return (a > b) ? a : b;
			}

			uint8_t is_player_support(int upperX, int upperY, int lowerX, int lowerY)
			{
				int feet = upperY + Pico_altura;
				int head = lowerY;

				int overlap = min_i(upperX + Pico_ancho, lowerX + Pico_ancho) - max_i(upperX, lowerX);

				if (overlap < 8) return 0;

				// SOLO si viene cayendo desde arriba
				if (feet >= head && feet <= head + 4)
				{
					return 1;
				}

				return 0;
			}

			uint8_t players_overlap_except_support(int x1, int y1, int x2, int y2)
			{
				if (is_player_support(x1, y1, x2, y2)) return 0;
				if (is_player_support(x2, y2, x1, y1)) return 0;
				return players_collide(x1, y1, x2, y2);
			}

			void drawOneLevelBox(uint8_t idx, uint8_t selected)
			{
				int row = idx / LEVEL_COLS;
				int col = idx % LEVEL_COLS;

				int bx = BOX_START_X + col * BOX_GAP_X;
				int by = BOX_START_Y + row * BOX_GAP_Y;

				// caja normal o caja seleccionada
				LCD_Sprite(bx, by, Caja_nivel_ancho, Caja_nivel_altura,
						   cajas, 2, selected ? 1 : 0, 0, 0);

				// texto dentro de la caja
				if (unlockedLevels[idx]) {
					char txt[2];
					txt[0] = '1' + idx;   // nivel 1, 2, 3...
					txt[1] = '\0';
					LCD_Print(txt, bx + 14, by + 12, 1, 0x0000, 0xFFFF);
				} else {
					LCD_Print("X", bx + 14, by + 12, 1, 0x0000, 0xFFFF);
				}
			}

			void drawLevelSelectScreen(void)
			{
				// segunda apariencia del fondo
				LCD_Sprite(0, 0, Fondo_ancho, Fondo_altura, Fondo, 2, 1, 0, 0);

				for (uint8_t i = 0; i < LEVEL_COUNT; i++) {
					drawOneLevelBox(i, i == selectedLevel);
				}
			}

			void drawLevelBackgroundOnly(void)
			{
				LCD_Sprite(0, 0, Fondo_ancho, Fondo_altura, Fondo, 2, 1, 0, 0);
			}


			void transmit_uart(char *string) {
				uint8_t len = strlen(string);
				HAL_UART_Transmit(&huart2, (uint8_t*) string, len, 200);
			}

			const AssetInfo* get_asset_info(uint8_t assetId)
			{
				for (uint32_t i = 0; i < sizeof(g_assetTable)/sizeof(g_assetTable[0]); i++) {
					if (g_assetTable[i].id == assetId) {
						return &g_assetTable[i];
					}
				}
				return NULL;
			}

			FRESULT mount_sd_once(void)
			{
				if (sdReady) return FR_OK;

				FRESULT fr = f_mount(&fs, "", 1);
				sprintf(buffer, "f_mount=%d\r\n", fr);
				transmit_uart(buffer);

				sdReady = (fr == FR_OK);
				return fr;
			}

			FRESULT load_text_file(const char *filename)
			{
			    UINT br = 0;

			    if (!sdReady) return FR_NOT_READY;

			    memset(mapBuffer, 0, sizeof(mapBuffer));

			    fres = f_open(&fill, filename, FA_READ);
			    sprintf(buffer, "f_open(%s)=%d\r\n", filename, fres);
			    transmit_uart(buffer);
			    if (fres != FR_OK) return fres;

			    fres = f_read(&fill, mapBuffer, sizeof(mapBuffer) - 1, &br);
			    sprintf(buffer, "f_read=%d bytes=%u\r\n", fres, br);
			    transmit_uart(buffer);

			    f_close(&fill);

			    if (fres == FR_OK) {
			        mapBuffer[br] = '\0';
			        mapBytesRead = br;

			        // 🔥 DEBUG CLAVE
			        transmit_uart("---- MAPA ----\r\n");
			        transmit_uart(mapBuffer);
			        transmit_uart("\r\n--------------\r\n");
			    }

			    return fres;
			}

			FRESULT load_map_by_level(uint8_t level)
			{
			    if (level == 0) return load_text_file("MAPA1.TMJ");
			    if (level == 1) return load_text_file("MAPA2.TMJ");
			    if (level == 2) return load_text_file("MAPA3.TMJ");
			    if (level == 3) return load_text_file("MAPA4.TMJ");
			    if (level == 4) return load_text_file("MAPA5.TMJ");
			    if (level == 5) return load_text_file("MAPA6.TMJ");

			    return FR_INVALID_NAME;
			}

			FRESULT load_asset_pixels(uint8_t assetId)
			{
				const AssetInfo *a = get_asset_info(assetId);
				UINT br = 0;

				if (!a) return FR_INVALID_NAME;
				if ((a->width * a->height) > MAX_ASSET_PIXELS) return FR_INVALID_OBJECT;

				fres = f_open(&fill, a->file, FA_READ);
				if (fres != FR_OK) return fres;

				fres = f_read(&fill, assetBuffer, a->width * a->height * 2, &br);
				f_close(&fill);

				if (fres != FR_OK) return fres;
				if (br != (UINT)(a->width * a->height * 2)) return FR_INT_ERR;

				return FR_OK;
			}

			void draw_asset_from_sd(uint8_t assetId, int x, int y)
			{
				const AssetInfo *a = get_asset_info(assetId);
				if (!a) return;

				if (load_asset_pixels(assetId) == FR_OK) {
					LCD_BitmapTransparent(x, y, a->width, a->height, assetBuffer, ASSET_TRANSPARENT_KEY);
				}
			}

			void clear_scene_objects(void)
			{
				sceneObjectCount = 0;
				memset(sceneObjects, 0, sizeof(sceneObjects));
			}

			void add_scene_object(uint8_t assetId, int16_t x, int16_t y)
			{
			    if (sceneObjectCount >= MAX_SCENE_OBJECTS) return;

			    SceneObject *obj = &sceneObjects[sceneObjectCount];

			    obj->assetId = assetId;
			    obj->x = x;
			    obj->y = y;
			    obj->active = 1;
			    obj->frame = 0;

			    // 🔥 RESET FLAGS
			    obj->solid = 0;
			    obj->movable = 0;
			    obj->collectible = 0;
			    obj->trigger = 0;

			    switch (assetId)
			    {
			        case ASSET_ID_PUSH_BLOCK:
			            obj->width = Bloque_empujar_pico_WIDTH;
			            obj->height = Bloque_empujar_pico_HEIGHT;
			            obj->frame = 2;
			            obj->solid = 1;
			            obj->movable = 1;
			            break;

			        case ASSET_ID_BOX:
			            obj->width = caja_pico_WIDTH;
			            obj->height = caja_pico_HEIGHT;
			            obj->solid = 1;
			            obj->movable = 1;
			            break;

			        case ASSET_ID_BIG_BOX:
			            obj->width = caja_grande_pico_WIDTH;
			            obj->height = caja_grande_pico_HEIGHT;
			            obj->solid = 1;
			            obj->movable = 1;
			            break;

			        case ASSET_ID_DOOR:
			            obj->width = Puerta_Pico_WIDTH;
			            obj->height = Puerta_Pico_HEIGHT;
			            obj->solid = 1;
			            obj->trigger = 1;
			            break;

			        case ASSET_ID_KEY:
			            obj->width = Llave_pico_WIDTH;
			            obj->height = Llave_pico_HEIGHT;
			            obj->collectible = 1;
			            break;

			        case ASSET_ID_GROUND_BLOCKS:
			            obj->width = bloques_pico_WIDTH;
			            obj->height = bloques_pico_HEIGHT;
			            obj->solid = 1;
			            break;

			        case ASSET_ID_BUTTON:
			            obj->width = boton_pico_WIDTH;
			            obj->height = boton_pico_HEIGHT;
			            obj->trigger = 1;
			            break;

			        case ASSET_ID_ELEVATORS:
			            obj->width = Elevadores_pico_WIDTH;
			            obj->height = Elevadores_pico_HEIGHT;
			            obj->solid = 1;
			            break;

			        case ASSET_ID_ELEVATOR_STATIC:
			            obj->width = Elevador_siempre_pico_WIDTH;
			            obj->height = Elevador_siempre_pico_HEIGHT;
			            obj->solid = 1;
			            break;

			        case ASSET_ID_JUMPERS:
			            obj->width = Saltarines_pico_WIDTH;
			            obj->height = Saltarines_pico_HEIGHT;
			            obj->trigger = 1;
			            break;

			        case ASSET_ID_COIN_STRIP:
			            obj->width = Moneda_Pico_WIDTH;
			            obj->height = Moneda_Pico_HEIGHT;
			            obj->collectible = 1;
			            break;

			        default:
			            obj->width = 20;
			            obj->height = 20;
			            break;
			    }

			    sceneObjectCount++;
			}

			/*
			 * Esta versión asume que en Tiled a cada objeto visible le agregaste
			 * una propiedad numérica: "assetId"
			 * y que el objeto también trae "x" y "y".
			 *
			 * Si tu TMJ tiene otro formato, ajustas solo esta función.
			 */

			void return_to_menu_after_map1(void)
			{
			    selectedLevel = 1;          // desbloqueas siguiente nivel
			    unlockedLevels[1] = 1;

			    confirmedLevel = 0xFF;
			    currentBgIndex = BG_MENU_INDEX;

			    moveCmd = 'S';
			    moveCmd2 = 'S';

			    jumpState = 0;
			    jumpState2 = 0;

			    appState = STATE_LEVEL_SELECT;

			    LCD_Clear(0x0000);
			    drawLevelSelectScreen();
			}

			void parse_tmj_for_static_assets(void)
			{
			    clear_scene_objects();
			    keyObjectIndex = -1;
			    doorObjectIndex = -1;
			    buttonObjectIndex = -1;
			    elevatorLeftIndex = -1;
			    elevatorRightIndex = -1;
			    keyCollected = 0;
			    buttonPressed = 0;
			    char *p = mapBuffer;

			    while ((p = strstr(p, "{")) != NULL)
			    {
			        int x = 0, y = 0, gid = 0;

			        char *end = strstr(p, "}");
			        if (!end) break;

			        // X
			        char *px = strstr(p, "\"x\"");
			        if (px && px < end)
			        {
			            char *c = strchr(px, ':');
			            if (c) x = atoi(c + 1);
			        }

			        // Y
			        char *py = strstr(p, "\"y\"");
			        if (py && py < end)
			        {
			            char *c = strchr(py, ':');
			            if (c) y = atoi(c + 1);
			        }

			        // GID
			        char *pgid = strstr(p, "\"gid\"");
			        if (pgid && pgid < end)
			        {
			            char *c = strchr(pgid, ':');
			            if (c) gid = atoi(c + 1);
			        }

			        if (gid == 0)
			        {
			            p = end + 1;
			            continue;
			        }

			        uint8_t assetId = 0;

			        // ======================================================
			        // 🔥 NIVEL 0 (MAPA1)
			        // ======================================================
			        if (currentLevel == 0)
			        {
			            if (gid == 5) assetId = ASSET_ID_KEY;
			            else if (gid == 6) assetId = ASSET_ID_DOOR;

			            else if (gid >= 8 && gid <= 10)
			                assetId = ASSET_ID_PUSH_BLOCK;

			            else if (gid == 11 || gid == 12)
			                assetId = ASSET_ID_BOX;

			            else if (gid >= 22 && gid <= 24)
			                assetId = ASSET_ID_BIG_BOX;

			            else if (gid >= 13 && gid <= 21)
			                assetId = ASSET_ID_GROUND_BLOCKS;
			        }

			        // ======================================================
			        // 🔥 NIVEL 1 (MAPA2)
			        // ======================================================
			        else if (currentLevel == 1)
			        {
			            if (gid == 5) assetId = ASSET_ID_KEY;
			            else if (gid == 6) assetId = ASSET_ID_DOOR;

			            else if (gid == 11 || gid == 12)
			                assetId = ASSET_ID_BOX;

			            else if (gid >= 22 && gid <= 24)
			                assetId = ASSET_ID_BIG_BOX;

			            else if (gid >= 8 && gid <= 10)
			                assetId = ASSET_ID_PUSH_BLOCK;

			            else if (gid >= 13 && gid <= 21)
			                assetId = ASSET_ID_GROUND_BLOCKS;
			            else if (gid == 25 || gid == 26 || gid == 27)
			                assetId = ASSET_ID_BUTTON;
			        }

			        // ======================================================
			        // 🔥 NIVEL 2 (MAPA3)
			        // ======================================================
			        else if (currentLevel == 2)
			        {
			            if (gid == 5) assetId = ASSET_ID_KEY;
			            else if (gid == 6) assetId = ASSET_ID_DOOR;

			            else if (gid >= 28 && gid <= 30)
			                assetId = ASSET_ID_ELEVATORS;

			            else if (gid >= 13 && gid <= 21)
			                assetId = ASSET_ID_GROUND_BLOCKS;
			        }

			        // ======================================================
			        // 🔥 NIVEL 3 (MAPA4)
			        // ======================================================
			        else if (currentLevel == 3)
			        {
			            if (gid == 6) assetId = ASSET_ID_DOOR;

			            else if (gid >= 13 && gid <= 21)
			                assetId = ASSET_ID_GROUND_BLOCKS;
			        }

			        // ======================================================
			        // 🔥 NIVEL 4 (MAPA5)
			        // ======================================================
			        else if (currentLevel == 4)
			        {
			            if (gid == 6) assetId = ASSET_ID_DOOR;

			            else if (gid == 1)
			                assetId = ASSET_ID_COIN_STRIP;

			            else if (gid == 30 || gid == 31)
			                assetId = ASSET_ID_JUMPERS;

			            else if (gid >= 13 && gid <= 21)
			                assetId = ASSET_ID_GROUND_BLOCKS;
			        }

			        // ======================================================
			        // 🔥 NIVEL 5 (MAPA6)
			        // ======================================================
			        else if (currentLevel == 5)
			        {
			            if (gid == 5) assetId = ASSET_ID_KEY;
			            else if (gid == 6) assetId = ASSET_ID_DOOR;

			            else if (gid == 26)
			                assetId = ASSET_ID_BUTTON;

			            else if (gid == 35)
			                assetId = ASSET_ID_ELEVATOR_STATIC;

			            else if (gid >= 13 && gid <= 22)
			                assetId = ASSET_ID_GROUND_BLOCKS;
			        }

			        // ======================================================
			        // CREAR OBJETO
			        // ======================================================
			        if (assetId != 0)
			        {
			            add_scene_object(assetId, x, y);
			            int newIndex = sceneObjectCount - 1;

			            if (assetId == ASSET_ID_KEY) {
			                keyObjectIndex = newIndex;
			            }
			            else if (assetId == ASSET_ID_DOOR) {
			                doorObjectIndex = newIndex;
			            }
			            else if (assetId == ASSET_ID_BUTTON) {
			                buttonObjectIndex = newIndex;
			            }
			            else if (assetId == ASSET_ID_ELEVATORS) {
			                if (elevatorLeftIndex < 0) elevatorLeftIndex = newIndex;
			                else if (elevatorRightIndex < 0) elevatorRightIndex = newIndex;
			            }
			            SceneObject *obj = &sceneObjects[sceneObjectCount - 1];
			            obj->y -= obj->height;
			        }

			        p = end + 1;
			    }
			    if (currentLevel == 1 && keyObjectIndex >= 0) {
			        sceneObjects[keyObjectIndex].active = 0;
			    }
			    sprintf(buffer, "Objetos cargados: %d\r\n", sceneObjectCount);
			    transmit_uart(buffer);
			}

			void draw_static_objects(void)
			{
				for (uint8_t i = 0; i < sceneObjectCount; i++)
				{
					if (sceneObjects[i].active)
					{
						draw_scene_object(&sceneObjects[i]);
					}
				}
			}

			uint8_t rects_intersect(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
			{
				if (x1 + w1 <= x2) return 0;
				if (x2 + w2 <= x1) return 0;
				if (y1 + h1 <= y2) return 0;
				if (y2 + h2 <= y1) return 0;
				return 1;
			}

			void draw_background_region(uint8_t bgIndex, int x, int y, int w, int h)
			{
				if (x < 0) { w += x; x = 0; }
				if (y < 0) { h += y; y = 0; }
				if (x + w > Fondo_ancho)  w = Fondo_ancho - x;
				if (y + h > Fondo_altura) h = Fondo_altura - y;
				if (w <= 0 || h <= 0) return;

				HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin, GPIO_PIN_RESET); // CS low
				SetWindows(x, y, x + w - 1, y + h - 1);
				HAL_GPIO_WritePin(GPIOA, LCD_RS_Pin, GPIO_PIN_SET);   // RS high

				const int fondoStride = Fondo_ancho * 2;

				for (int row = 0; row < h; row++) {
					int srcY = y + row;
					int base = srcY * fondoStride + bgIndex * Fondo_ancho + x;

					for (int col = 0; col < w; col++) {
						uint16_t pixel = Fondo[base + col];
						LCD_DATA(pixel >> 8);
						LCD_DATA(pixel & 0xFF);
					}
				}

				HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin, GPIO_PIN_SET);   // CS high
			}

			void restore_scene_region(int x, int y, int w, int h)
			{
				draw_background_region(currentBgIndex, x, y, w, h);

				for (uint8_t i = 0; i < sceneObjectCount; i++)
				{
					if (!sceneObjects[i].active) continue;

					if (rects_intersect(
							x, y, w, h,
							sceneObjects[i].x, sceneObjects[i].y,
							sceneObjects[i].width, sceneObjects[i].height))
					{
						draw_scene_object(&sceneObjects[i]); // ✅ SOLO bitmap
					}
				}
			}

			uint8_t is_solid_at(int x, int y, int w, int h)
			{
			    if (y + h >= FLOOR_Y + Pico_altura) return 1;

			    if (x < LEFT_WALL || x + w > RIGHT_WALL + Pico_ancho) return 1;

			    for (uint8_t i = 0; i < sceneObjectCount; i++)
			    {
			        if (!sceneObjects[i].active) continue;

			        if (sceneObjects[i].solid)
			        {
			            if (rects_intersect(x, y, w, h,
			                                sceneObjects[i].x,
			                                sceneObjects[i].y,
			                                sceneObjects[i].width,
			                                sceneObjects[i].height))
			            {
			                return 1;
			            }
			        }
			    }

			    return 0;
			}

			uint8_t is_solid_below(int x, int y)
			{
				// piso real
				if (is_solid_at(x, y + 1, Pico_ancho, Pico_altura)) {
					return 1;
				}

				// 👇 SOLO soporte temporal del jugador (NO piso permanente)

				// jugador 1 sobre jugador 2
				if (x == playerX && y == playerY) {
					if (jumpState == 2 &&
						is_player_support(x, y + 1, player2X, player2Y)) {
						return 1;
					}
				}

				// jugador 2 sobre jugador 1
				if (x == player2X && y == player2Y) {
					if (jumpState2 == 2 &&
						is_player_support(x, y + 1, playerX, playerY)) {
						return 1;
					}
				}

				return 0;
			}

			void save_peak_positions(void)
			{
				savedPlayer1X = playerX;
				savedPlayer1Y = playerY;

				savedPlayer2X = player2X;
				savedPlayer2Y = player2Y;
			}

			void drawPauseOption(uint8_t idx, uint8_t selected)
			{
				int w = 220;
				int h = 40;

				int x = (Fondo_ancho - w) / 2;   // centrado horizontal
				int y = 80 + idx * 50;           // espaciado vertical

				uint16_t color = selected ? 0x07E0 : 0xFFFF;
				uint16_t textColor = selected ? 0xFFFF : 0x0000;

				FillRect(x, y, w, h, color);

				char *text;

				if (idx == 0) text = "CONTINUAR";
				else if (idx == 1) text = "REINICIAR";
				else text = "MENU";

				// centrado de texto aproximado
				int textX = x + (w / 2) - (strlen(text) * 6 / 2);
				int textY = y + (h / 2) - 4;

				LCD_Print(text, textX, textY, 1, textColor, color);
			}

			void draw_pause_menu(void)
			{
				// fondo pantalla completa
				FillRect(0, 0, Fondo_ancho, Fondo_altura, 0xFED4);

				// título centrado
				LCD_Print("PAUSA", (Fondo_ancho / 2) - 30, 40, 2, 0x0000, 0xFED4);

				drawPauseOption(0, pauseOption == 0);
				drawPauseOption(1, pauseOption == 1);
				drawPauseOption(2, pauseOption == 2);
			}


			void handle_sd_card_operations(const char *filename)
			{
				UINT br = 0;

				if (!sdReady) {
					mount_sd_once();
				}

				memset(mapBuffer, 0, sizeof(mapBuffer));

				fres = f_open(&fill, filename, FA_READ);
				if (fres != FR_OK) return;

				fres = f_read(&fill, mapBuffer, sizeof(mapBuffer) - 1, &br);
				f_close(&fill);

				if (fres == FR_OK) {
					mapBuffer[br] = '\0';

					transmit_uart("Contenido del archivo:\r\n");
					transmit_uart(mapBuffer);
					transmit_uart("\r\n");
				}
			}

			void draw_scene_object(SceneObject *obj)
			{
			    switch (obj->assetId)
			    {
			        case ASSET_ID_PUSH_BLOCK:
			            LCD_Sprite(obj->x, obj->y, 30, 220, Bloque_empujar_pico, 3, obj->frame, 0, 0);
			            break;

			        case ASSET_ID_BOX:
			            LCD_Sprite(obj->x, obj->y, 25, 25, caja_pico, 2, 0, 0, 0);
			            break;

			        case ASSET_ID_BIG_BOX:
			            LCD_Sprite(obj->x, obj->y, 50, 50, caja_grande_pico, 3, 0, 0, 0);
			            break;

			        case ASSET_ID_DOOR:
			            LCD_Sprite(obj->x, obj->y, 40, 38, puerta, 2, obj->frame, 0, 0);
			            break;

			        case ASSET_ID_KEY:
			            LCD_Bitmap(obj->x, obj->y, 20, 28, Llave_pico);
			            break;

			        case ASSET_ID_GROUND_BLOCKS:
			            LCD_Sprite(obj->x, obj->y, 15, 15, bloques_pico, 10, 0, 0, 0);
			            break;

			        case ASSET_ID_BUTTON:
			            LCD_Sprite(obj->x, obj->y, 34, 14, boton_pico, 2, obj->frame, 0, 0);
			            break;

			        case ASSET_ID_ELEVATORS:
			            LCD_Sprite(obj->x, obj->y, 90, 50, Elevadores_pico, 3, 0, 0, 0);
			            break;
			        case ASSET_ID_COIN_STRIP:
			            LCD_Sprite(obj->x, obj->y, 20, 28, Moneda_Pico, 4, 0, 0, 0);
			            break;

			        case ASSET_ID_JUMPERS:
			            LCD_Sprite(obj->x, obj->y, 20, 20, Saltarines_pico, 4, 0, 0, 0);
			            break;

			        case ASSET_ID_ELEVATOR_STATIC:
			            LCD_Sprite(obj->x, obj->y, 60, 8, Elevador_siempre_pico, 1, 0, 0, 0);
			            break;
			    }
			}

			void update_button_key_logic_level2(void)
			{
			    if (currentLevel != 1) return;
			    if (buttonPressed) return;
			    if (buttonObjectIndex < 0) return;

			    SceneObject *btn = &sceneObjects[buttonObjectIndex];

			    uint8_t p1OnButton = rects_intersect(playerX, playerY, Pico_ancho, Pico_altura,
			                                         btn->x, btn->y, btn->width, btn->height);

			    uint8_t p2OnButton = rects_intersect(player2X, player2Y, Pico_ancho, Pico_altura,
			                                         btn->x, btn->y, btn->width, btn->height);

			    if (p1OnButton || p2OnButton)
			    {
			        buttonPressed = 1;
			        btn->frame = 1;

			        if (keyObjectIndex >= 0) {
			            sceneObjects[keyObjectIndex].active = 1;
			        }

			        LCD_Clear(0x0000);
			        drawLevelBackgroundOnly();
			        draw_static_objects();
			        drawPlayer();
			        drawPlayer2();
			    }
			}

			void update_push_blocks(void)
			{
			    for (uint8_t i = 0; i < sceneObjectCount; i++)
			    {
			        SceneObject *obj = &sceneObjects[i];

			        if (!obj->active) continue;
			        if (!obj->movable) continue;

			        int oldX = obj->x;

			        uint8_t pushed = 0;

			        // =========================
			        // PLAYER 1 EMPUJA
			        // =========================
			        if (rects_intersect(playerX, playerY, Pico_ancho, Pico_altura,
			                            obj->x, obj->y, obj->width, obj->height))
			        {
			            if (moveCmd == 'R')
			            {
			                obj->x += 1;
			                playerX = obj->x - Pico_ancho; // 🔥 pegarlo al bloque
			                pushed = 1;
			            }
			            else if (moveCmd == 'L')
			            {
			                obj->x -= 1;
			                playerX = obj->x + obj->width;
			                pushed = 1;
			            }
			        }

			        // =========================
			        // PLAYER 2 EMPUJA
			        // =========================
			        if (rects_intersect(player2X, player2Y, Pico_ancho, Pico_altura,
			                            obj->x, obj->y, obj->width, obj->height))
			        {
			            if (moveCmd2 == 'R')
			            {
			                obj->x += 1;
			                player2X = obj->x - Pico_ancho;
			                pushed = 1;
			            }
			            else if (moveCmd2 == 'L')
			            {
			                obj->x -= 1;
			                player2X = obj->x + obj->width;
			                pushed = 1;
			            }
			        }

			        // =========================
			        // LIMPIAR Y REDIBUJAR
			        // =========================
			        if (obj->x != oldX)
			        {
			            int minX = (oldX < obj->x) ? oldX : obj->x;
			            int width = obj->width + abs(obj->x - oldX);

			            restore_scene_region(minX, obj->y, width, obj->height);

			            draw_scene_object(obj);

			            drawPlayer();
			            drawPlayer2();
			        }
			    }
			}

			void update_coins(void)
			{
			    for (uint8_t i = 0; i < sceneObjectCount; i++)
			    {
			        SceneObject *obj = &sceneObjects[i];

			        if (!obj->active) continue;
			        if (!obj->collectible) continue;

			        if (rects_intersect(playerX, playerY, Pico_ancho, Pico_altura,
			                            obj->x, obj->y, obj->width, obj->height)
			         || rects_intersect(player2X, player2Y, Pico_ancho, Pico_altura,
			                            obj->x, obj->y, obj->width, obj->height))
			        {
			            obj->active = 0;

			            restore_scene_region(obj->x, obj->y, obj->width, obj->height);
			        }
			    }
			}
			void update_jumpers(void)
			{
			    for (uint8_t i = 0; i < sceneObjectCount; i++)
			    {
			        SceneObject *obj = &sceneObjects[i];

			        if (!obj->active) continue;
			        if (obj->assetId != ASSET_ID_JUMPERS) continue;

			        if (rects_intersect(playerX, playerY, Pico_ancho, Pico_altura,
			                            obj->x, obj->y, obj->width, obj->height))
			        {
			            playerY -= 70;
			        }

			        if (rects_intersect(player2X, player2Y, Pico_ancho, Pico_altura,
			                            obj->x, obj->y, obj->width, obj->height))
			        {
			            player2Y -= 20;
			        }
			    }
			}

			void update_level_logic(void)
			{
			    update_push_blocks();
			    if (currentLevel == 1) {
			            update_button_key_logic_level2();
			        }
			    update_key_and_door_logic();
			    update_coins();
			    update_jumpers();
			}
		/* USER CODE END 0 */

		/**
		  * @brief  The application entry point.
		  * @retval int
		  */
		int main(void)
		{

		  /* USER CODE BEGIN 1 */

		  /* USER CODE END 1 */

		  /* MCU Configuration--------------------------------------------------------*/

		  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
		  HAL_Init();

		  /* USER CODE BEGIN Init */

		  /* USER CODE END Init */

		  /* Configure the system clock */
		  SystemClock_Config();

		  /* USER CODE BEGIN SysInit */

		  /* USER CODE END SysInit */

		  /* Initialize all configured peripherals */
		  MX_GPIO_Init();
		  MX_SPI1_Init();
		  MX_USART3_UART_Init();
		  MX_FATFS_Init();
		  MX_UART5_Init();
		  MX_USART2_UART_Init();
		  /* USER CODE BEGIN 2 */


			  LCD_Init();
			  LCD_Clear(0x0000);

			  HAL_Delay(500);


			  HAL_Delay(500);
			  mount_sd_once();

			  currentBgIndex = BG_MENU_INDEX;

			  // O si prefieres tu fondo:
			  LCD_Sprite(0, 0, Fondo_ancho, Fondo_altura, Fondo, 2, 0, 0, 0);
			  //LCD_Bitmap(0, 0, Fondo_ancho, Fondo_altura, Fondo);

			  playerX = 20;
			  playerY = FLOOR_Y;
			  prevX = playerX;
			  prevY = playerY;

			  player2X = 80;
			  player2Y = FLOOR_Y;
			  prev2X = player2X;
			  prev2Y = player2Y;

			  drawPlayer();
			  drawPlayer2();

			  HAL_UART_Receive_IT(&huart3, &rx3, 1);
			  HAL_UART_Receive_IT(&huart5, &rx5, 1);

		  /* USER CODE END 2 */

		  /* Infinite loop */
		  /* USER CODE BEGIN WHILE */

			  while (1)
			  {
			      // =========================
			      // 🔥 PAUSA SOLO EN NIVEL
			      // =========================
			      if (pauseEvent && appState == STATE_LEVEL_CONFIRMED)
			      {
			          pauseEvent = 0;

			          pauseOption = 0;
			          appState = STATE_PAUSE;
			          draw_pause_menu();
			          continue;
			      }

			      // =========================
			      // ESTADO: JUEGO NORMAL (pantalla inicio)
			      // =========================
			      if (appState == STATE_GAME)
			      {
			          if (menuEvent) {
			              menuEvent = 0;

			              appState = STATE_LEVEL_SELECT;
			              selectedLevel = 0;

			              moveCmd = 'S';
			              moveCmd2 = 'S';

			              LCD_Clear(0x0000);
			              drawLevelSelectScreen();
			              HAL_Delay(20);
			              continue;
			          }

			          updatePlayer1();
			          updatePlayer2();
			          redrawPlayersIfNeeded();
			      }

			      // =========================
			      // ESTADO: SELECTOR DE NIVEL
			      // =========================
			      else if (appState == STATE_LEVEL_SELECT)
			      {
			          // 🔥 TRIÁNGULO = volver a inicio
			          if (pauseEvent)
			          {
			              pauseEvent = 0;

			              appState = STATE_GAME;

			              moveCmd = 'S';
			              moveCmd2 = 'S';

			              LCD_Clear(0x0000);

			              currentBgIndex = BG_MENU_INDEX;
			              LCD_Sprite(0, 0, Fondo_ancho, Fondo_altura, Fondo, 2, 0, 0, 0);

			              playerX = 20;
			              playerY = FLOOR_Y;
			              prevX = playerX;
			              prevY = playerY;

			              player2X = 80;
			              player2Y = FLOOR_Y;
			              prev2X = player2X;
			              prev2Y = player2Y;

			              map4AlternateJumpMode = (confirmedLevel == 3) ? 1 : 0;
			              jumpTurn = 1;

			              if (confirmedLevel == 3)   // MAPA4
			              {
			                  keyCollected = 1;

			                  if (keyObjectIndex >= 0) {
			                      sceneObjects[keyObjectIndex].active = 0;
			                  }

			                  if (doorObjectIndex >= 0) {
			                      sceneObjects[doorObjectIndex].frame = 1;
			                      sceneObjects[doorObjectIndex].solid = 0;
			                  }

			                  player2X = -100;
			                  player2Y = -100;
			                  prev2X = player2X;
			                  prev2Y = player2Y;
			              }

			              drawPlayer();
			              if (!map4AlternateJumpMode) {
			                  drawPlayer2();
			              }

			              continue;
			          }

			          uint8_t oldSelected = selectedLevel;
			          moveCmd2 = 'S';

			          if (moveCmd == 'L') {
			              if ((selectedLevel % LEVEL_COLS) > 0) selectedLevel--;
			              moveCmd = 'S';
			          }
			          else if (moveCmd == 'R') {
			              if ((selectedLevel % LEVEL_COLS) < (LEVEL_COLS - 1)) selectedLevel++;
			              moveCmd = 'S';
			          }
			          else if (moveCmd == 'F') {
			              if (selectedLevel >= LEVEL_COLS) selectedLevel -= LEVEL_COLS;
			              moveCmd = 'S';
			          }
			          else if (moveCmd == 'B') {
			              if (selectedLevel + LEVEL_COLS < LEVEL_COUNT) selectedLevel += LEVEL_COLS;
			              moveCmd = 'S';
			          }

			          if (oldSelected != selectedLevel) {
			              drawOneLevelBox(oldSelected, 0);
			              drawOneLevelBox(selectedLevel, 1);
			          }

			          // 🔓 cheat unlock
			          if (squareEvent) {
			              squareEvent = 0;

			              if (!allLevelsUnlocked) {
			                  squarePressCount++;

			                  sprintf(buffer, "Square count=%d\r\n", squarePressCount);
			                  transmit_uart(buffer);

			                  if (squarePressCount >= 5) {
			                      unlock_all_levels();
			                      drawLevelSelectScreen();
			                  }
			              }
			          }

			          // 🔥 CARGAR NIVEL
			          if (menuEvent) {
			              menuEvent = 0;

			              if (unlockedLevels[selectedLevel]) {

			                  confirmedLevel = selectedLevel;
			                  currentLevel   = confirmedLevel;

			                  fres = load_map_by_level(confirmedLevel);

			                  if (fres == FR_OK)
			                  {
			                      parse_tmj_for_static_assets();

			                      LevelSpawn s = levelSpawns[confirmedLevel];

			                      playerX  = s.x1;
			                      playerY  = s.y1 - Pico_altura;

			                      player2X = s.x2;
			                      player2Y = s.y2 - Pico_altura;

			                      prevX  = playerX;
			                      prevY  = playerY;
			                      prev2X = player2X;
			                      prev2Y = player2Y;

			                      appState = STATE_LEVEL_CONFIRMED;
			                      currentBgIndex = BG_LEVEL_INDEX;

			                      LCD_Clear(0x0000);
			                      drawLevelBackgroundOnly();
			                      draw_static_objects();

			                      drawPlayer();
			                      drawPlayer2();
			                  }
			                  else
			                  {
			                      transmit_uart("Error cargando mapa\r\n");
			                  }
			              }
			          }
			      }

			      // =========================
			      // ESTADO: NIVEL (GAMEPLAY REAL)
			      // =========================
			      else if (appState == STATE_LEVEL_CONFIRMED)
			      {
			    	  updatePlayer1();

			    	  if (!map4AlternateJumpMode) {
			    	      updatePlayer2();
			    	  }

			    	  // 🔥 TODA LA LOGICA CENTRAL
			    	  update_level_logic();

			    	  if (map4AlternateJumpMode) {
			    	      if ((playerX != prevX) || (playerY != prevY)) {
			    	          restore_scene_region(prevX, prevY, Pico_ancho, Pico_altura);
			    	          drawPlayer();
			    	      }

			    	      if (jumpTurn == 1) {
			    	          LCD_Print("TURNO P1", 10, 10, 1, 0x0000, 0xFFFF);
			    	      } else {
			    	          LCD_Print("TURNO P2", 10, 10, 1, 0x0000, 0xFFFF);
			    	      }
			    	  } else {
			    	      redrawPlayersIfNeeded();
			    	  }
			      }

			      // =========================
			      // ESTADO: PAUSA
			      // =========================
			      else if (appState == STATE_PAUSE)
			      {
			          uint8_t oldOption = pauseOption;

			          if (moveCmd == 'F') {
			              if (pauseOption > 0) pauseOption--;
			              moveCmd = 'S';
			          }
			          else if (moveCmd == 'B') {
			              if (pauseOption < 2) pauseOption++;
			              moveCmd = 'S';
			          }

			          if (oldOption != pauseOption) {
			              drawPauseOption(oldOption, 0);
			              drawPauseOption(pauseOption, 1);
			          }

			          if (menuEvent)
			          {
			              menuEvent = 0;

			              if (pauseOption == 0)
			              {
			                  appState = STATE_LEVEL_CONFIRMED;

			                  LCD_Clear(0x0000);
			                  drawLevelBackgroundOnly();
			                  draw_static_objects();
			                  drawPlayer();
			                  if (!map4AlternateJumpMode) {
			                      drawPlayer2();
			                  }
			              }
			              else if (pauseOption == 1)
			              {
			                  appState = STATE_LEVEL_CONFIRMED;

			                  LCD_Clear(0x0000);
			                  drawLevelBackgroundOnly();
			                  draw_static_objects();

			                  LevelSpawn s = levelSpawns[confirmedLevel];

			                  playerX  = s.x1;
			                  playerY  = s.y1 - Pico_altura;

			                  player2X = s.x2;
			                  player2Y = s.y2 - Pico_altura;

			                  prevX  = playerX;
			                  prevY  = playerY;
			                  prev2X = player2X;
			                  prev2Y = player2Y;

			                  drawPlayer();
			                  if (!map4AlternateJumpMode) {
			                      drawPlayer2();
			                  }
			              }
			              else if (pauseOption == 2)
			              {
			                  appState = STATE_LEVEL_SELECT;

			                  LCD_Clear(0x0000);
			                  drawLevelSelectScreen();
			              }

			              pauseOption = 0;
			          }
			      }

			      HAL_Delay(20);
			  }
		  /* USER CODE END 3 */
		}

		/**
		  * @brief System Clock Configuration
		  * @retval None
		  */
		void SystemClock_Config(void)
		{
		  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
		  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

		  /** Configure the main internal regulator output voltage
		  */
		  __HAL_RCC_PWR_CLK_ENABLE();
		  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

		  /** Initializes the RCC Oscillators according to the specified parameters
		  * in the RCC_OscInitTypeDef structure.
		  */
		  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
		  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
		  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
		  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
		  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
		  RCC_OscInitStruct.PLL.PLLM = 8;
		  RCC_OscInitStruct.PLL.PLLN = 80;
		  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
		  RCC_OscInitStruct.PLL.PLLQ = 2;
		  RCC_OscInitStruct.PLL.PLLR = 2;
		  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
		  {
			Error_Handler();
		  }

		  /** Initializes the CPU, AHB and APB buses clocks
		  */
		  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
									  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
		  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
		  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
		  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
		  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

		  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
		  {
			Error_Handler();
		  }
		}

		/**
		  * @brief SPI1 Initialization Function
		  * @param None
		  * @retval None
		  */
		static void MX_SPI1_Init(void)
		{

		  /* USER CODE BEGIN SPI1_Init 0 */

		  /* USER CODE END SPI1_Init 0 */

		  /* USER CODE BEGIN SPI1_Init 1 */

		  /* USER CODE END SPI1_Init 1 */
		  /* SPI1 parameter configuration*/
		  hspi1.Instance = SPI1;
		  hspi1.Init.Mode = SPI_MODE_MASTER;
		  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
		  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
		  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
		  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
		  hspi1.Init.NSS = SPI_NSS_SOFT;
		  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
		  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
		  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
		  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
		  hspi1.Init.CRCPolynomial = 10;
		  if (HAL_SPI_Init(&hspi1) != HAL_OK)
		  {
			Error_Handler();
		  }
		  /* USER CODE BEGIN SPI1_Init 2 */

		  /* USER CODE END SPI1_Init 2 */

		}

		/**
		  * @brief UART5 Initialization Function
		  * @param None
		  * @retval None
		  */
		static void MX_UART5_Init(void)
		{

		  /* USER CODE BEGIN UART5_Init 0 */

		  /* USER CODE END UART5_Init 0 */

		  /* USER CODE BEGIN UART5_Init 1 */

		  /* USER CODE END UART5_Init 1 */
		  huart5.Instance = UART5;
		  huart5.Init.BaudRate = 115200;
		  huart5.Init.WordLength = UART_WORDLENGTH_8B;
		  huart5.Init.StopBits = UART_STOPBITS_1;
		  huart5.Init.Parity = UART_PARITY_NONE;
		  huart5.Init.Mode = UART_MODE_TX_RX;
		  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
		  if (HAL_UART_Init(&huart5) != HAL_OK)
		  {
			Error_Handler();
		  }
		  /* USER CODE BEGIN UART5_Init 2 */

		  /* USER CODE END UART5_Init 2 */

		}

		/**
		  * @brief USART2 Initialization Function
		  * @param None
		  * @retval None
		  */
		static void MX_USART2_UART_Init(void)
		{

		  /* USER CODE BEGIN USART2_Init 0 */

		  /* USER CODE END USART2_Init 0 */

		  /* USER CODE BEGIN USART2_Init 1 */

		  /* USER CODE END USART2_Init 1 */
		  huart2.Instance = USART2;
		  huart2.Init.BaudRate = 115200;
		  huart2.Init.WordLength = UART_WORDLENGTH_8B;
		  huart2.Init.StopBits = UART_STOPBITS_1;
		  huart2.Init.Parity = UART_PARITY_NONE;
		  huart2.Init.Mode = UART_MODE_TX_RX;
		  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
		  if (HAL_UART_Init(&huart2) != HAL_OK)
		  {
			Error_Handler();
		  }
		  /* USER CODE BEGIN USART2_Init 2 */

		  /* USER CODE END USART2_Init 2 */

		}

		/**
		  * @brief USART3 Initialization Function
		  * @param None
		  * @retval None
		  */
		static void MX_USART3_UART_Init(void)
		{

		  /* USER CODE BEGIN USART3_Init 0 */

		  /* USER CODE END USART3_Init 0 */

		  /* USER CODE BEGIN USART3_Init 1 */

		  /* USER CODE END USART3_Init 1 */
		  huart3.Instance = USART3;
		  huart3.Init.BaudRate = 115200;
		  huart3.Init.WordLength = UART_WORDLENGTH_8B;
		  huart3.Init.StopBits = UART_STOPBITS_1;
		  huart3.Init.Parity = UART_PARITY_NONE;
		  huart3.Init.Mode = UART_MODE_TX_RX;
		  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
		  if (HAL_UART_Init(&huart3) != HAL_OK)
		  {
			Error_Handler();
		  }
		  /* USER CODE BEGIN USART3_Init 2 */

		  /* USER CODE END USART3_Init 2 */

		}

		/**
		  * @brief GPIO Initialization Function
		  * @param None
		  * @retval None
		  */
		static void MX_GPIO_Init(void)
		{
		  GPIO_InitTypeDef GPIO_InitStruct = {0};
		  /* USER CODE BEGIN MX_GPIO_Init_1 */
		  /* USER CODE END MX_GPIO_Init_1 */

		  /* GPIO Ports Clock Enable */
		  __HAL_RCC_GPIOH_CLK_ENABLE();
		  __HAL_RCC_GPIOC_CLK_ENABLE();
		  __HAL_RCC_GPIOA_CLK_ENABLE();
		  __HAL_RCC_GPIOB_CLK_ENABLE();
		  __HAL_RCC_GPIOD_CLK_ENABLE();

		  /*Configure GPIO pin Output Level */
		  HAL_GPIO_WritePin(GPIOC, LCD_RST_Pin|LCD_D1_Pin, GPIO_PIN_RESET);

		  /*Configure GPIO pin Output Level */
		  HAL_GPIO_WritePin(GPIOA, LCD_RD_Pin|LCD_WR_Pin|LCD_RS_Pin|LCD_D7_Pin
								  |LCD_D0_Pin|LCD_D2_Pin, GPIO_PIN_RESET);

		  /*Configure GPIO pin Output Level */
		  HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin|LCD_D6_Pin|LCD_D3_Pin|LCD_D5_Pin
								  |LCD_D4_Pin, GPIO_PIN_RESET);

		  /*Configure GPIO pin Output Level */
		  HAL_GPIO_WritePin(SD_SS_GPIO_Port, SD_SS_Pin, GPIO_PIN_SET);

		  /*Configure GPIO pins : LCD_RST_Pin LCD_D1_Pin */
		  GPIO_InitStruct.Pin = LCD_RST_Pin|LCD_D1_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		  /*Configure GPIO pins : LCD_RD_Pin LCD_WR_Pin LCD_RS_Pin LCD_D7_Pin
								   LCD_D0_Pin LCD_D2_Pin */
		  GPIO_InitStruct.Pin = LCD_RD_Pin|LCD_WR_Pin|LCD_RS_Pin|LCD_D7_Pin
								  |LCD_D0_Pin|LCD_D2_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		  /*Configure GPIO pins : LCD_CS_Pin LCD_D6_Pin LCD_D3_Pin LCD_D5_Pin
								   LCD_D4_Pin */
		  GPIO_InitStruct.Pin = LCD_CS_Pin|LCD_D6_Pin|LCD_D3_Pin|LCD_D5_Pin
								  |LCD_D4_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		  /*Configure GPIO pin : SD_SS_Pin */
		  GPIO_InitStruct.Pin = SD_SS_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
		  HAL_GPIO_Init(SD_SS_GPIO_Port, &GPIO_InitStruct);

		  /* USER CODE BEGIN MX_GPIO_Init_2 */
		  /* USER CODE END MX_GPIO_Init_2 */
		}

		/* USER CODE BEGIN 4 */
		void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
		{
			if (huart->Instance == USART3)
			{
				if (rx3 != '\r' && rx3 != '\n')
				{
					if (rx3 == 'J') {
						if (map4AlternateJumpMode) {
											      if (jumpTurn == 1) {
											            jumpEvent = 1;
											            jumpTurn = 2;
											        }
											    } else {
											        jumpEvent = 1;
											    }
					}
					else if (rx3 == 'C') {
						menuEvent = 1;
					}
					else if (rx3 == 'Q') {
					    squareEvent = 1;
					}
					else if (rx3 == 'F' || rx3 == 'B' || rx3 == 'L' ||
							 rx3 == 'R' || rx3 == 'S')
					{
						moveCmd = rx3;
					}
					else if (rx3 == 'T') {
						pauseEvent = 1;
					}
				}

				HAL_UART_Receive_IT(&huart3, &rx3, 1);
			}

			else if (huart->Instance == UART5)
			{
				if (rx5 != '\r' && rx5 != '\n')
				{
					if (rx5 == 'J') {
						if (map4AlternateJumpMode) {
											        if (jumpTurn == 2) {
											            jumpEvent = 1;   // el mismo personaje sube
											            jumpTurn = 1;
											        }
											    } else {
											        jumpEvent2 = 1;
											    }
					}
					else if (rx5 == 'F' || rx5 == 'B' || rx5 == 'L' ||
							 rx5 == 'R' || rx5 == 'S')
					{
						moveCmd2 = rx5;
					}
				}

				HAL_UART_Receive_IT(&huart5, &rx5, 1);
			}
		}
		/* USER CODE END 4 */

		/**
		  * @brief  This function is executed in case of error occurrence.
		  * @retval None
		  */
		void Error_Handler(void)
		{
		  /* USER CODE BEGIN Error_Handler_Debug */
				/* User can add his own implementation to report the HAL error return state */
				__disable_irq();
				while (1) {
				}
		  /* USER CODE END Error_Handler_Debug */
		}
		#ifdef USE_FULL_ASSERT
		/**
		  * @brief  Reports the name of the source file and the source line number
		  *         where the assert_param error has occurred.
		  * @param  file: pointer to the source file name
		  * @param  line: assert_param error line source number
		  * @retval None
		  */
		void assert_failed(uint8_t *file, uint32_t line)
		{
		  /* USER CODE BEGIN 6 */
			  /* User can add his own implementation to report the file name and line number,
				 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
		  /* USER CODE END 6 */
		}
		#endif /* USE_FULL_ASSERT */
