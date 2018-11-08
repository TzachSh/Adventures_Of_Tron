//#####			         Adventures of Tron					   ###### //
//##### Created By: Tzachi Sheratzky, Or Cohen, Eliran Menashe ###### // 

#include <stdio.h>
#include <dos.h>
#include <conf.h>
#include <kernel.h>
#include <io.h>
#include <bios.h>
#include <time.h>
#include <string.h>
#include <math.h>

//DEFINES
#define ARROW_NUMBER 5
#define TARGET_NUMBER 4
#define TRUE 1
#define FALSE 0
#define COLS 80
#define ROWS 25
#define FLOORS_LENGTH 5
#define ELEVATORS_LENGTH 8
#define FLOORS_DISTANCE 6
#define ENEMY_LENGTH 6
#define ENEMIES_DISTANCE 10
#define FLOATING_BITS_LENGTH 7
#define RTL 1
#define LTR 0
#define LEFT 'L'
#define RIGHT 'R'
#define DOWN 'S'
#define STANDING 'S'
#define JUMP_RIGHT 'K'
#define JUMP_LEFT 'H'
#define ALL_FLOATS_COLLECTED 1
#define ONE_FLOAT_COLLECTED 0
#define EMPTY_FLOAT_COLLECTED -1
#define REPEAT_FLOAT_EFFECT 2
#define OFF 0
#define ON 1
#define DELAY_SOUND 50

extern SYSCALL  sleept(int);
extern struct intmap far *sys_imp;
int mapinit(int vec, int(*newisr)(), int mdevno);
INTPROC newISR70h(int mdevno);
void Sound(int hertz);
void NoSound();

// Define enemy types in the game
enum EnemyType
{
	Recognizer,
	GridBug,
	Tank
};

// Define struct for position
typedef struct
{
	int x;
	int y;
}Position;
// Define struct for each cell on screen
typedef struct
{
	char character;
	unsigned char attribute;
}Block;
// Define struct for tank's bullet
typedef struct
{
	Position position;
	Block blocks;
	int height;
	int width;
	int active;
}Bullet;
// Define struct for each enemy in the game
typedef struct
{
	Position position;
	Block blocks[4];
	int direction;
	int height;
	int width;
	enum EnemyType type;
	Bullet bullet;
	int current_floor;
	int speed;
	int in_game;
}Enemy;
// Define struct for each floor
typedef struct
{
	int number;
	int height;
	int y;
	Block blocks[COLS];
}Floor;
// Define struct for each floating bit
typedef struct
{
	Position position;
	Block blocks;
	int height;
	int width;
	int score;
	int speed;
	int active;
	int floor;
	int direction;
}FloatingBit;

// Define struct for the solar
typedef struct
{
	Position position;
	Block blocks[2];
	int height;
	int width;
	int speed;
	int active;
	int floor;
	int direction;
	int playerHold;
}Solar;
// Define struct for the ladder
typedef struct
{
	Position position;
	int width;
	Block blocks[ROWS];
	int passed_from_left;
	int passed_from_right;
	int isPassed;
}Ladder;
// Define struct for each elevator
typedef struct {
	Position position;
	Block blocks[16];
	int isActivated;
	int width;
	int pId;
	int distance;
}Elevator;
// Define struct for Tron (the player)
typedef struct {
	Position position;
	Block blocks[5];
	Elevator* elevator;
	Floor currentFloor;
	int onElevator;
	int BLOCKS_LENGTH;
	int height;
	int width;
	int life;
	int direction;
	int isFalling;
	int isJumping;
	int goDown;
	int initializing;
	int canLevelUp;
	int inLeveling;
}Player;

//Global variables

Block display_draft[25][80];
Player player;
Ladder ladder;
Floor floors[FLOORS_LENGTH];
Elevator elevators[ELEVATORS_LENGTH];
Enemy enemies[ENEMY_LENGTH];
FloatingBit floatingBits[FLOATING_BITS_LENGTH];
Solar solar;
int receiver_pid;
int create_enemy_pid;
unsigned char far *b800h; // screen pointer
Block display[2000];
Block blank;
char ch_arr[2048]; // chars buffer
int front = -1;
int rear = -1;
int elevatorsActive = FALSE; // is elevators active,  true = start moving elevators, false = end moving
int elevatorsMoving = FALSE; // is elevators moving right now
volatile int gameOver = FALSE; // check for game over
volatile int gameWin = FALSE; // check for win the game
volatile int speakers_status = OFF; // define sound flag
volatile delay_sound = DELAY_SOUND; //init sound time
volatile int floating_bits_status = EMPTY_FLOAT_COLLECTED; // floating bits collecting status
volatile int floating_bits_counter = 0; // how many floating bits collected
int groups_speeds[3] = { 30, 30, 30 }; // speeds group for each enemy type (for level 3)

int enemyReleaseNum = 0; // count how many enemies are in game
int isEnemiesSpeedDoubled = FALSE; // check if to double enemies speed
int enemyWait = FALSE; // wait between activating each group of enemy
int next_schedule;
int level = 1;
int point_in_cycle;
int gcycle_length;
int gno_of_pids;
unsigned long score = 0;
volatile unsigned long delay_schedule = 0; // 0x70 interrupt counter
volatile unsigned long end_level_schedule = 0; // 0x70 check if to douoble enemies by this timer
char old_0A1h_mask; // save old 0A1H
char old_70h_A_mask; // save old 70h 
int x71h1;
int x71h2 = 0;
int x71h3;

// Function which receiving status
//and then changes speaker status by this status
void ChangeSpeaker(int status)
{
	int portval;
	//   portval = inportb( 0x61 );

	portval = 0;
	asm{
		PUSH AX
		MOV AL,61h
		MOV byte ptr portval,AL
		POP AX
	}

		if (status == ON)
			portval |= 0x03;
		else
			portval &= ~0x03;
	// outportb( 0x61, portval );
	asm{
		PUSH AX
		MOV AX,portval
		OUT 61h,AL
		POP AX
	} // asm

} //ChangeSpeaker

  //Function which receiving hertz
  //and then creates sound by the hertz using the ports 0x43 and 0x42
void Sound(int hertz)
{
	unsigned divisor = 1193180L / hertz;

	ChangeSpeaker(ON);

	speakers_status = ON;
	delay_sound = DELAY_SOUND;

	//        outportb( 0x43, 0xB6 );
	asm{
		PUSH AX
		MOV AL,0B6h
		OUT 43h,AL
		POP AX
	} // asm

	  //       outportb( 0x42, divisor & 0xFF );
		asm{
		PUSH AX
		MOV AX,divisor
		AND AX,0FFh
		OUT 42h,AL
		POP AX
	} // asm

	  //        outportb( 0x42, divisor >> 8 );

		asm{
		PUSH AX
		MOV AX,divisor
		MOV AL,AH
		OUT 42h,AL
		POP AX
	} // asm

} //Sound

  //Function which disables sound
void NoSound(void)
{
	ChangeSpeaker(OFF);
	speakers_status = OFF;
} //NoSound

  //Function which receiving array and 3 speeds
  //and then randomize speed for each cell in the array
void randomInsertArr(int arr[], int speed1, int speed2, int speed3)
{
	int i, currIndex, firstIndex, secondIndex, thirdIndex, tempIndex;
	int othersIndexes[2] = { 0, 0 };

	firstIndex = getRandomRange(0, 2);

	for (i = 0, currIndex = 0; i < 3; i++)
	{
		if (firstIndex != i)
		{
			othersIndexes[currIndex++] = i;
		}
	}

	tempIndex = getRandomRange(0, 1);
	secondIndex = othersIndexes[tempIndex];

	thirdIndex = 3 - (firstIndex + secondIndex);

	arr[firstIndex] = speed1;
	arr[secondIndex] = speed2;
	arr[thirdIndex] = speed3;
}

//Function which receiving an array, min and max values
//and then randomize speeds which are different by at least 20%
void determineSpeed(int arr[], int min, int max)
{
	int _min, _max, speed1, speed2, speed3;

	speed1 = getRandomRange(min, max);

	_min = (int)(0.8 * speed1);
	_max = (int)(1.2 * speed1);

	speed2 = getRandomRange(10, _min);
	speed3 = getRandomRange(_max, 100);

	randomInsertArr(arr, speed1, speed2, speed3);
}

//Function which initializing the player by falling down the ladder
void player_initialize_down_onLadder()
{
	if (player.initializing == TRUE && delay_schedule % 7 == 0)
	{
		if (player.position.y < floors[4].y - player.height)
			player.position.y++;
		else
		{
			player.initializing = FALSE;
			ladder.isPassed = FALSE;
			elevatorsMoving = FALSE;
			ladder.passed_from_left = FALSE;
			ladder.passed_from_right = FALSE;
			player.currentFloor = floors[4];
			player.inLeveling = FALSE;
		}
	}
}

//Function which checks if the player got on the solar by calculating the position in axis x and y
int check_player_on_solar()
{
	int target_x_start;

	if (solar.direction == LTR)
	{
		target_x_start = solar.position.x + solar.width - 1;

		if ((player.position.x == target_x_start || player.position.x == target_x_start + 1) && (player.position.y == solar.position.y))
			return TRUE;
		else
			return FALSE;
	}
	else if (solar.direction == RTL)
	{
		target_x_start = solar.position.x;

		if ((player.position.x == target_x_start || player.position.x == target_x_start - 1) && (player.position.y == solar.position.y))
			return TRUE;
		else
			return FALSE;
	}

	return FALSE;
}

//Function which receiving the player and an elevator
//and then checks if the player is standing on the current elevator by calculating the position in axis x and y
int check_onElevator(Player player, Elevator elevator)
{
	int x_detection = FALSE;

	int start_x_obj = elevator.position.x;
	int end_x_obj = start_x_obj + elevator.width - player.width;

	int start_x_player = player.position.x;
	int end_x_player = start_x_player + player.width - 2;

	if (player.position.y + player.height == elevator.position.y)
	{
		if (start_x_player >= start_x_obj && start_x_player <= end_x_obj)
			x_detection = TRUE;
		else if (end_x_player >= start_x_obj && end_x_player <= end_x_obj)
			x_detection = TRUE;
	}

	return x_detection;
}

//Function to reset the color of the ladder and floors color after blue effect
void reset_blocks()
{
	int i, j;

	// return original floors attributes
	for (i = 1; i < FLOORS_LENGTH; i++)
	{
		for (j = 0; j < COLS; j++)
		{
			int ladderWidth = 1;
			if ((j <= COLS / 2 - ladderWidth) || (j >= COLS / 2 + ladderWidth))
			{
				floors[i].blocks[j].attribute = 0x6;
			}
		}
	}

	// return original ladder attributes
	for (i = 0; i < ROWS; i++)
	{
		ladder.blocks[i].character = ' ';
		ladder.blocks[i].attribute = 0x6C;
	}

	floating_bits_status = EMPTY_FLOAT_COLLECTED;
}

//Function which turn the floor and ladder to blue
void blue_effect()
{
	int i, j;

	for (i = 1; i < FLOORS_LENGTH; i++)
	{
		for (j = 0; j < COLS; j++)
		{
			floors[i].blocks[j].attribute = 0x19;
		}
	}

	for (i = 0; i < ROWS; i++)
	{
		ladder.blocks[i].attribute = 0x19;
	}


	if (delay_schedule >= next_schedule)
	{
		reset_blocks();
	}
}

//Function which receiving the player, a position of an object, and it's width and height
//and then checks if the player is colliding the object by calculating the position in axis x and y
int check_collision(Player player, Position pos_object, int width_object, int height_object)
{
	int x_detection = FALSE, y_detection = FALSE;

	int start_x_obj = pos_object.x;
	int end_x_obj = start_x_obj + width_object - 1;

	int start_y_obj = pos_object.y + height_object - 1;
	int end_y_obj = pos_object.y;

	int start_x_player = player.position.x;
	int end_x_player = start_x_player + player.width - 1;

	int start_y_player = player.position.y + player.height - 1;
	int end_y_player = player.position.y;

	if (start_x_player >= start_x_obj && start_x_player <= end_x_obj)
		x_detection = TRUE;

	if (end_x_player >= start_x_obj && end_x_player <= end_x_obj)
		x_detection = TRUE;

	if (start_y_player <= start_y_obj && start_y_player >= end_y_obj)
		y_detection = TRUE;

	if (end_y_player <= start_y_obj && end_y_player >= end_y_obj)
		y_detection = TRUE;

	return (x_detection && y_detection);
}

//Function that checks if the player is standing on any elevator in his current floor
int is_player_onElevator()
{
	int onLeftElevator = check_onElevator(player, elevators[player.currentFloor.number * 2]);
	int onRightElevator = check_onElevator(player, elevators[player.currentFloor.number * 2 + 1]);

	if (onLeftElevator == TRUE)
	{
		player.elevator = &elevators[player.currentFloor.number * 2];
	}
	else if (onRightElevator == TRUE)
	{
		player.elevator = &elevators[player.currentFloor.number * 2 + 1];
	}

	return (onLeftElevator == TRUE || onRightElevator == TRUE);
}

//Function which receiving min and max number
//and then randomize a number between min to max
int getRandomRange(int min, int max)
{
	int num = (rand() %
		(max - min + 1)) + min;
	return num;
}

//Function which receiving a solar pointer
//and then updates the solar position in the game
void updateSolar(Solar* solar)
{
	if (solar->active)
	{
		if (delay_schedule % solar->speed == 0)
		{
			if (solar->direction == LTR)
			{
				if (solar->position.x < COLS - solar->width)
				{
					solar->position.x++;
				}
				else
				{
					solar->direction = RTL;
				}

				if (player.position.x + player.width - 1 < COLS - 1)
				{
					if (solar->playerHold == TRUE)
					{
						player.position.x++;
					}
				}
				else
				{
					solar->playerHold = FALSE;
				}


				solar->blocks[0].character = '-';
				solar->blocks[0].attribute = 0x0D;
				solar->blocks[1].character = '>';
				solar->blocks[1].attribute = 0x0D;


			}
			else if (solar->direction == RTL)
			{
				if (solar->position.x > 0)
				{
					solar->position.x--;
				}
				else
				{
					solar->direction = LTR;
					solar->position.x = 1;
				}


				if (player.position.x > 0)
				{
					if (solar->playerHold == TRUE)
					{
						player.position.x--;
					}
				}
				else
				{
					solar->playerHold = FALSE;
				}

				solar->blocks[0].character = '<';
				solar->blocks[0].attribute = 0x0D;
				solar->blocks[1].character = '-';
				solar->blocks[1].attribute = 0x0D;
			}
		}
	}
}

//Function which receiving a floating bit, status, floor and direction
//and then initializing the floating bit using these arguments
void initFloatingBit(FloatingBit* floatingBit, int active, int floor, int direction)
{
	int tempScore, tempSpeed;
	Block block;
	block.character = '*';
	block.attribute = 0x0E;
	floatingBit->active = active;
	floatingBit->floor = floor;
	floatingBit->height = 1;
	floatingBit->width = 1;
	floatingBit->blocks = block;
	floatingBit->direction = direction;
	floatingBit->position.y = floors[floor].y + 1;
	floatingBit->position.x = getRandomRange(0, COLS);
	switch (floor)
	{
	case 0: tempScore = 800; tempSpeed = 10; break;
	case 1: tempScore = 400; tempSpeed = 20; break;
	case 2: tempScore = 200; tempSpeed = 30; break;
	case 3: tempScore = 100; tempSpeed = 40; break;
	}
	floatingBit->score = tempScore;
	floatingBit->speed = tempSpeed;
}

//Function to initialize all of the floating bits
void initFloatingBits()
{
	initFloatingBit(&floatingBits[0], TRUE, 0, LTR);
	initFloatingBit(&floatingBits[1], TRUE, 0, RTL);
	initFloatingBit(&floatingBits[2], TRUE, 1, LTR);
	initFloatingBit(&floatingBits[3], TRUE, 2, RTL);
	initFloatingBit(&floatingBits[4], TRUE, 2, LTR);
	initFloatingBit(&floatingBits[5], TRUE, 3, RTL);
	initFloatingBit(&floatingBits[6], TRUE, 3, LTR);
}

//Function which receiving a solar pointer, floor, active and direction
//and then initializing the solar with these arguments
void initSolar(Solar* solar, int floor, int active, int direction)
{
	solar->blocks[0].character = '-';
	solar->blocks[0].attribute = 0x0D;
	solar->blocks[1].character = '>';
	solar->blocks[1].attribute = 0x0D;
	solar->active = active;
	solar->speed = 15;
	solar->floor = floor;
	solar->height = 1;
	solar->width = 2;
	solar->direction = direction;
	solar->position.y = floors[floor].y + 1;
	solar->position.x = getRandomRange(0, COLS);
}

//Function which receiving the player
//and then initializing the player data
void initPlayer(Player* player)
{
	int i;
	player->direction = STANDING;
	player->BLOCKS_LENGTH = 5;
	for (i = 0; i < player->BLOCKS_LENGTH; i++)
	{
		if (i % 2 == 0) {
			player->blocks[i].character = '^';
			player->blocks[i].attribute = 0x1D;
		}
		else {
			player->blocks[i].character = '+';
			player->blocks[i].attribute = 0x2D;
		}
	}

	player->canLevelUp = FALSE;
	player->inLeveling = FALSE;
	player->currentFloor = floors[FLOORS_LENGTH - 1];
	player->width = 2;
	player->height = 2;
	player->position.x = ladder.position.x;
	player->position.y = 0;
	player->initializing = TRUE;
	player->life = 3;
	player->isJumping = FALSE;
	player->isFalling = FALSE;
}

//Function which receiving an enemy pointer
//and then copies the enemy's display data to the display_draft matrix in the enemy's position by any enemy type
void displayEnemy(Enemy* enemy)
{
	if (enemy->type == Recognizer)
	{
		display_draft[enemy->position.y][enemy->position.x] = enemy->blocks[0];
		display_draft[enemy->position.y][enemy->position.x + 1] = enemy->blocks[1];
		display_draft[enemy->position.y + 1][enemy->position.x] = enemy->blocks[2];
		display_draft[enemy->position.y + 1][enemy->position.x + 1] = enemy->blocks[3];
	}
	else if (enemy->type == GridBug)
	{
		if (enemy->height == 2)
		{
			display_draft[enemy->position.y][enemy->position.x] = enemy->blocks[0];
			display_draft[enemy->position.y][enemy->position.x + 1] = enemy->blocks[1];
			display_draft[enemy->position.y + 1][enemy->position.x] = enemy->blocks[2];
			display_draft[enemy->position.y + 1][enemy->position.x + 1] = enemy->blocks[3];
		}
		else
		{
			display_draft[enemy->position.y + 1][enemy->position.x] = enemy->blocks[0];
			display_draft[enemy->position.y + 1][enemy->position.x + 1] = enemy->blocks[1];
		}
	}
	else if (enemy->type == Tank)
	{
		if (enemy->direction == RTL)
		{
			display_draft[enemy->position.y][enemy->position.x] = enemy->blocks[1];
			display_draft[enemy->position.y][enemy->position.x + 1] = enemy->blocks[0];
		}
		else if (enemy->direction == LTR)
		{
			display_draft[enemy->position.y][enemy->position.x] = enemy->blocks[0];
			display_draft[enemy->position.y][enemy->position.x + 1] = enemy->blocks[1];
		}

		display_draft[enemy->position.y + 1][enemy->position.x] = enemy->blocks[2];
		display_draft[enemy->position.y + 1][enemy->position.x + 1] = enemy->blocks[3];

		if (enemy->bullet.active == TRUE)
		{
			display_draft[enemy->bullet.position.y][enemy->bullet.position.x] = enemy->bullet.blocks;
		}
	}
}

//Function which receiving an enemy pointer
//and then updates the position of the enemy in the game and updates the bullet if it's a tank
void updateEnemyPosition(Enemy* enemy)
{
	if (enemy->direction == LTR && enemy->position.x < COLS - enemy->width)
	{
		enemy->position.x++;

		if (enemy->type == Tank && enemy->bullet.active == FALSE && (player.position.x > enemy->position.x) && (player.currentFloor.number == enemy->current_floor - 1))
		{
			enemy->bullet.position.x = enemy->position.x + enemy->width;
			enemy->bullet.position.y = enemy->position.y;
			enemy->bullet.active = TRUE;
		}
	}
	else if (enemy->direction == RTL && enemy->position.x > 0)
	{
		enemy->position.x--;

		if (enemy->type == Tank && enemy->bullet.active == FALSE && (player.position.x < enemy->position.x) && (player.currentFloor.number == enemy->current_floor - 1))
		{
			enemy->bullet.position.x = enemy->position.x - enemy->width;
			enemy->bullet.position.y = enemy->position.y;
			enemy->bullet.active = TRUE;
		}
	}
	else
	{
		if (level >= 2) {
			int random_direction = rand() % 2;
			enemy->direction = (random_direction == LTR) ? LTR : RTL;
		}

		if (enemy->direction == RTL) {
			enemy->position.x = COLS - enemy->width;
		}
		else if (enemy->direction == LTR) {
			enemy->position.x = 0;
		}

		if (enemy->current_floor == FLOORS_LENGTH - 1)
		{
			enemy->current_floor = 1;
			enemy->position.y = floors[1].y - enemy->height;
		}
		else
		{
			enemy->position.y = floors[enemy->current_floor + 1].y - enemy->height;
			enemy->current_floor++;
		}
	}
}
//Function which receiving an enemy pointer
//and if the enemy bullet is active, then updates the bullet position in the game
void updateBulletPosition(Enemy* enemy)
{
	if (enemy->direction == LTR && enemy->bullet.active == TRUE)
	{
		if (enemy->bullet.position.x < COLS - enemy->bullet.width)
			enemy->bullet.position.x++;
		else
			enemy->bullet.active = FALSE;
	}
	else if (enemy->direction == RTL && enemy->bullet.active == TRUE)
	{
		if (enemy->bullet.position.x > 0)
			enemy->bullet.position.x--;
		else
			enemy->bullet.active = FALSE;
	}
}

//Function initializing random speed to each enemy's group for level 3
void initRandomSpeed()
{
	int groupIndex, i;
	// rand 3 speed for 3 groups
	determineSpeed(groups_speeds, 40, 60);

	for (i = 0, groupIndex = 0; i < ENEMY_LENGTH; i++)
	{
		enemies[i].speed = groups_speeds[groupIndex];

		if (i == 2 || i == 4)
			groupIndex++;
	}
}

//Function which updates any enemy or bullets position in the game
void updateEnemiesPosition()
{
	int i, delay_static = delay_schedule;

	if (level == 3 && delay_static % 500 == 0)
	{
		initRandomSpeed();
	}

	for (i = 0; i < ENEMY_LENGTH; i++)
	{
		if (enemies[i].in_game)
		{
			if (delay_static % enemies[i].speed == 0)
			{
				updateEnemyPosition(&enemies[i]);
			}

			if (enemies[i].type == Tank && delay_static % (enemies[i].speed / 2) == 0)
			{
				updateBulletPosition(&enemies[i]);
			}
		}
	}
}
//Function which receiving an player pointer
//and then copies the player's display data to the display_draft matrix by the player position
void displayPlayer(Player* player)
{
	int i;

	if (gameOver == FALSE && gameWin == FALSE)
	{
		for (i = 0; i < player->width; i++) {
			display_draft[player->position.y][player->position.x + i] = player->blocks[i];
			display_draft[player->position.y + 1][player->position.x + i] = player->blocks[i];
		}
	}
}

//Function which receiving an enemy pointer, speed and direction
//and then initializing the enemy's data by these arguments

void initEnemy(Enemy* enemy, int speed, int direction)
{
	if (level >= 2) {
		int random_direction = rand() % 2;
		enemy->direction = (random_direction == LTR) ? LTR : RTL;
	}
	enemy->current_floor = 1;
	enemy->height = enemy->width = 2;
	enemy->position.y = floors[1].y - enemy->height;
	if (enemy->direction == LTR)
		enemy->position.x = 0;
	else if (enemy->direction == RTL)
		enemy->position.x = COLS - enemy->width;
	enemy->in_game = FALSE;
	enemy->speed = speed;
}

//Function which receiving a enemy pointer, speed and direction
//and then initializing the recognize type of enemy by these arguments
void initEnemy_recognizers(Enemy* enemy, int speed, int direction)
{
	initEnemy(enemy, speed, direction);

	enemy->type = Recognizer;

	enemy->blocks[0].character = 220;
	enemy->blocks[0].attribute = 30;
	enemy->blocks[1].character = 220;
	enemy->blocks[1].attribute = 30;
	enemy->blocks[2].character = 221;
	enemy->blocks[2].attribute = 30;
	enemy->blocks[3].character = 222;
	enemy->blocks[3].attribute = 30;
}


//Function which receiving a enemy pointer, speed and direction
//and then initializing the gridBug type of enemy by these arguments
void initEnemy_gridBug(Enemy* enemy, int speed, int direction)
{
	initEnemy(enemy, speed, direction);

	enemy->type = GridBug;

	enemy->blocks[0].character = ' ';
	enemy->blocks[0].attribute = 0x48;
	enemy->blocks[1].character = ' ';
	enemy->blocks[1].attribute = 0x48;
	enemy->blocks[2].character = '/';
	enemy->blocks[2].attribute = 0x0F;
	enemy->blocks[3].character = '\\';
	enemy->blocks[3].attribute = 0x0F;
}


//Function which receiving a enemy pointer, speed and direction
//and then initializing the cannonTank type of enemy by these arguments
void initEnemy_cannonTank(Enemy* enemy, int speed, int direction)
{
	initEnemy(enemy, speed, direction);

	enemy->type = Tank;

	enemy->bullet.active = FALSE;
	enemy->bullet.blocks.character = '*';
	enemy->bullet.blocks.attribute = 0xCE;
	enemy->bullet.height = 1;
	enemy->bullet.width = 1;
	enemy->bullet.position.y = enemy->position.y;
	enemy->bullet.position.x = enemy->position.x;

	enemy->blocks[0].character = ' ';
	enemy->blocks[0].attribute = 0x5D;
	enemy->blocks[1].character = '-';
	enemy->blocks[1].attribute = 0x0D;
	enemy->blocks[2].character = 153;
	enemy->blocks[2].attribute = 0x07;
	enemy->blocks[3].character = 153;
	enemy->blocks[3].attribute = 0x07;
}


//Function which receiving a enemy array
//and then initializing every type of enemy by depending in the current level
void initEnemies(Enemy enemies[])
{
	int i, speed, direction;
	enemyReleaseNum = 0;
	if (level == 1)
	{
		speed = 50;
		direction = LTR;

		for (i = 0; i < ENEMY_LENGTH; i++)
		{
			initEnemy_recognizers(&enemies[i], speed, direction);
		}
	}
	else if (level == 2)
	{
		speed = 30;

		for (i = 0; i < ENEMY_LENGTH; i++)
		{
			if (i < 3)
				initEnemy_recognizers(&enemies[i], speed, direction);
			else
				initEnemy_gridBug(&enemies[i], speed, direction);
		}
	}
	else if (level == 3)
	{
		speed = 30; // default value

		for (i = 0; i < ENEMY_LENGTH; i++)
		{
			if (i == 0)
				initEnemy_cannonTank(&enemies[i], speed, direction);
			else if (i < 3)
				initEnemy_gridBug(&enemies[i], speed, direction);
			else
				initEnemy_recognizers(&enemies[i], speed, direction);
		}

		initRandomSpeed();
	}
}

//Function which receiving an enemies array
//and then displays each enemy data to the draft_matrix
void displayEnemies(Enemy enemies[])
{
	int i;
	for (i = 0; i < ENEMY_LENGTH; i++)
	{
		if (enemies[i].in_game)
			displayEnemy(&enemies[i]);
	}
}

//Function which receiving an solar pointer
//and then copies the solar's display data to the display_draft matrix by the solar's position 
void displaySolar(Solar* solar)
{
	if (solar->active)
	{
		display_draft[solar->position.y][solar->position.x] = solar->blocks[0];
		display_draft[solar->position.y][solar->position.x + 1] = solar->blocks[1];
	}
}

//Function which receiving an floating bit pointer
//and then copies the floating bit's display data to the display_draft matrix in the floating bit's position
void displayFloatingBit(FloatingBit* floatingBit)
{
	if (floatingBit->active)
		display_draft[floatingBit->position.y][floatingBit->position.x] = floatingBit->blocks;
}

//Function which displaying all of the floating bits 
void displayFloatingBits()
{
	int i;
	for (i = 0; i < FLOATING_BITS_LENGTH; i++)
	{
		displayFloatingBit(&floatingBits[i]);
	}
}

//Function which receiving a ladder pointer
//and then initializing the ladder in the middle of the screen and with his display data
void initLadder(Ladder* ladder)
{
	int i;
	ladder->width = 2;
	ladder->position.y = ROWS;
	ladder->position.x = COLS / 2;
	ladder->passed_from_left = FALSE;
	ladder->passed_from_right = FALSE;
	ladder->isPassed = FALSE;
	for (i = 0; i < ROWS; i++)
	{
		ladder->blocks[i].character = ' ';
		ladder->blocks[i].attribute = 0x6C;
	}
}
//Function which receiving an elevators array
//and then initializing two elevators in each floor, and any elevator's data.
void initElevators(Elevator elevators[])
{
	int i;
	for (i = 0; i < ELEVATORS_LENGTH; i++)
	{
		char ch, attr;
		int j, distance = 3;
		elevators[i].width = 8;
		elevators[i].isActivated = TRUE;
		for (j = 0; j < elevators[i].width; j++)
		{
			elevators[i].blocks[j].character = 220;
			elevators[i].blocks[j].attribute = 0x0F;
		}
		elevators[i].distance = FLOORS_DISTANCE;
		elevators[i].position.y = floors[i / 2 + 1].y;
		if (i % 2 == 0)
			elevators[i].position.x = COLS / 4 - elevators[i].width / 2 + 1;
		else
			elevators[i].position.x = COLS * 3 / 4 - elevators[i].width / 2;
	}
}

//Function to display the status of the score, level and the player's life
void display_gameStatus()
{
	int i, j, score_length;
	char ch_score[6];
	floors[0].blocks[0].character = 'S';
	floors[0].blocks[1].character = 'C';
	floors[0].blocks[2].character = 'O';
	floors[0].blocks[3].character = 'R';
	floors[0].blocks[4].character = 'E';
	floors[0].blocks[5].character = ':';


	if (score < 10)
	{
		score_length = 1;
	}
	else if (score < 100)
	{
		score_length = 2;
	}
	else if (score < 1000)
	{
		score_length = 3;
	}
	else if (score < 10000)
	{
		score_length = 4;
	}
	else if (score < 100000)
	{
		score_length = 5;
	}

	ultoa(score, ch_score, 10);

	for (i = 0; i < score_length; i++)
	{
		floors[0].blocks[6 + i].character = ch_score[i];
	}

	floors[0].blocks[26].character = 'L';
	floors[0].blocks[27].character = 'E';
	floors[0].blocks[28].character = 'V';
	floors[0].blocks[29].character = 'E';
	floors[0].blocks[30].character = 'L';
	floors[0].blocks[31].character = ':';
	floors[0].blocks[32].character = ' ';

	for (i = 33; i < COLS; i += 2)
	{
		if (i < 33 + level + (level - 1))
		{
			floors[0].blocks[i].character = '*';
			floors[0].blocks[i].attribute = 0x7C;

			if (i + 1 < COLS)
				floors[0].blocks[i + 1].character = ' ';
		}
		else
		{
			floors[0].blocks[i].character = ' ';
		}
	}

	floors[0].blocks[69].character = 'L';
	floors[0].blocks[70].character = 'I';
	floors[0].blocks[71].character = 'F';
	floors[0].blocks[72].character = 'E';
	floors[0].blocks[73].character = ':';
	floors[0].blocks[74].character = ' ';

	for (i = 75; i < COLS; i += 2)
	{
		if (i < 75 + player.life + (player.life - 1))
		{
			floors[0].blocks[i].character = '#';
			floors[0].blocks[i].attribute = 0x7C;

			if (i + 1 < COLS)
				floors[0].blocks[i + 1].character = ' ';
		}
		else
		{
			floors[0].blocks[i].character = ' ';
		}
	}

	if (gameOver == TRUE)
	{
		floors[0].blocks[43].character = 'G';
		floors[0].blocks[44].character = 'A';
		floors[0].blocks[45].character = 'M';
		floors[0].blocks[46].character = 'E';
		floors[0].blocks[47].character = ' ';
		floors[0].blocks[48].character = 'O';
		floors[0].blocks[49].character = 'V';
		floors[0].blocks[50].character = 'E';
		floors[0].blocks[51].character = 'R';
		floors[0].blocks[52].character = '!';

		for (i = 43; i <= 52; i++)
			floors[0].blocks[i].attribute |= 0x80;
	}
	else if (gameWin == TRUE)
	{
		floors[0].blocks[43].character = 'W';
		floors[0].blocks[44].character = 'I';
		floors[0].blocks[45].character = 'N';
		floors[0].blocks[46].character = '!';
		floors[0].blocks[47].character = '!';
		floors[0].blocks[48].character = '!';

		for (i = 43; i <= 48; i++)
			floors[0].blocks[i].attribute |= 0x80;
	}
}

//Function which receiving a floors array
//and then initializing each floor with her data
void initFloors(Floor floors[])
{
	int i, y = 0;
	for (i = 0; i < FLOORS_LENGTH; i++, y += FLOORS_DISTANCE)
	{
		int j;

		if (i == 0)
			floors[i].number = -1;
		else
			floors[i].number = i - 1;

		floors[i].height = FLOORS_DISTANCE;
		floors[i].y = y;
		for (j = 0; j < COLS; j++) {
			int ladderWidth = 1;
			if ((j <= COLS / 2 - ladderWidth) || (j >= COLS / 2 + ladderWidth)) {
				if (i == 0)
				{
					floors[i].number = -1;
					floors[i].blocks[j].character = ' ';
					floors[i].blocks[j].attribute = 0x70;
				}
				else
				{
					floors[i].blocks[j].character = 220;
					floors[i].blocks[j].attribute = 0x6;
				}
			}
		}
	}
}
//Function which receiving a floating bit pointer
//and then updates his position in the game
void updateFloatingBit(FloatingBit* floatingBit)
{
	if (floatingBit->active)
	{
		if (delay_schedule % floatingBit->speed == 0)
		{
			if (floatingBit->direction == LTR)
			{
				if (floatingBit->position.x < COLS - 1)
					floatingBit->position.x++;
				else
				{
					floatingBit->direction = RTL;
				}
			}
			else if (floatingBit->direction == RTL)
			{
				if (floatingBit->position.x > 0)
					floatingBit->position.x--;
				else
				{
					floatingBit->direction = LTR;
					floatingBit->position.x = 1;
				}
			}
		}
	}
}

//Function which updates each floating bit position in the game
void updateFloatingBits()
{
	int i;
	for (i = 0; i < FLOATING_BITS_LENGTH; i++)
	{
		updateFloatingBit(&floatingBits[i]);
	}
}

//Function which doubles each enemy's speed
void doubleEnemiesSpeed()
{
	int i;

	for (i = 0; i < ENEMY_LENGTH; i++)
		enemies[i].speed /= 2;
}

//Function which calculates 1 minute, when the time ends, double each enemy's speed
void calcEndTimer()
{
	if (end_level_schedule < 6000)
	{
		end_level_schedule++;
	}
	else
	{
		if (isEnemiesSpeedDoubled == FALSE) {
			doubleEnemiesSpeed();
			isEnemiesSpeedDoubled = TRUE;
		}
	}
}
//Function to schedule each group of enemies entry time
void createEnemies()
{
	if (delay_schedule % 100 == 0) { // check every seceond

		if (enemyWait == FALSE) // release 3 enemies each time
		{
			if (enemyReleaseNum < ENEMY_LENGTH) {

				if (enemyReleaseNum == 0) // release the first enemy when start
				{
					enemies[enemyReleaseNum++].in_game = TRUE;
				}
				else if (delay_schedule % 1000 == 0) // wait 10 seconds between each enemy releasing
				{
					enemies[enemyReleaseNum++].in_game = TRUE;

					if (enemyReleaseNum % 3 == 0 || (level == 3 && (enemyReleaseNum == 1 || enemyReleaseNum == 3)))
					{
						enemyWait = TRUE; // wait after releasing 3 enemies for longer time
					}
				}
			}
		}
		else
		{
			if (delay_schedule % 5000 == 0) { // wait 50 seconds between each three of enemies
				enemyWait = FALSE;
			}
		}
	}
}
//Function which receiving score to add, 
//and then adding the score to the current score 
void addScore(int scoreToAdd)
{
	score += scoreToAdd;
}
//Function to intialize the next level by reset the enemies, floating bits and reset blocks blue effect
void initNextLevel()
{
	initEnemies(enemies);
	end_level_schedule = 0;
	initFloatingBits();
	floating_bits_counter = 0;
	floating_bits_status = EMPTY_FLOAT_COLLECTED;
	reset_blocks();
}
//Function to get the player up on the ladder
void upOnLadder()
{
	if (delay_schedule % 7 == 0)
	{
		if (player.position.y > floors[0].y)
		{
			player.position.x = ladder.position.x;
			player.position.y--;
		}
		else
		{
			addScore(2000);

			if (player.canLevelUp == TRUE) {
				if (level < 3)
				{
					level++;
					initNextLevel();


				}
				else
				{
					gameWin = TRUE;
				}

				player.canLevelUp = FALSE;

			}
		}
	}
}
//Define new handler for interrupt 0x70
INTPROC newISR70h(int mdevno)
{
	int i;

	asm{
		PUSH AX
		PUSH BX
		IN AL,70h   // Read existing port 70h
		MOV BX,AX
		MOV AL,0Ch  // Set up "Read status register C"
		OUT 70h,AL  //
		MOV AL,8Ch  //
		OUT 70h,AL  //
		IN AL,71h
		MOV AX,BX   //  Restore port 70h
		OUT 70h,AL  //
		MOV AL,20h   // Set up "EOI" value 
		OUT 0A0h,AL  // Notify Secondary PIC
		OUT 020h,AL  // Notify Primary PIC
		POP BX
		POP AX
	} // asm */

	delay_schedule++;
	if (delay_schedule >= 10000)
		delay_schedule = 0;

	if (speakers_status == ON)
	{
		if (delay_sound > 0)
			delay_sound--;
		else
			NoSound();
	}

	updateEnemiesPosition();
	updateFloatingBits();
	updateSolar(&solar);

	for (i = 0; i < FLOATING_BITS_LENGTH; i++)
	{
		if (player.initializing == FALSE && floatingBits[i].active == TRUE && check_collision(player, floatingBits[i].position, floatingBits[i].width, floatingBits[i].height) == TRUE)
		{
			floatingBits[i].active = FALSE;
			score += floatingBits[i].score;

			floating_bits_counter++;

			if (floating_bits_counter < FLOATING_BITS_LENGTH)
			{
				floating_bits_status = ONE_FLOAT_COLLECTED;
			}
			else
			{
				floating_bits_status = ALL_FLOATS_COLLECTED;
				player.canLevelUp = TRUE;
			}

			Sound(1500);
		}
	}

	if (player.canLevelUp) {
		calcEndTimer();
	}

	if (floating_bits_status == ALL_FLOATS_COLLECTED || floating_bits_status == REPEAT_FLOAT_EFFECT) // collect all bits
	{
		if (floating_bits_status != REPEAT_FLOAT_EFFECT)
		{
			next_schedule = (delay_schedule + 600) % 10000;
			floating_bits_status = REPEAT_FLOAT_EFFECT;
		}

		// next level effect
		blue_effect();
	}
	else if (floating_bits_status == ONE_FLOAT_COLLECTED || floating_bits_status == REPEAT_FLOAT_EFFECT) // collect single bit
	{
		if (floating_bits_status != REPEAT_FLOAT_EFFECT)
		{
			next_schedule = (delay_schedule + 100) % 10000;
			floating_bits_status = REPEAT_FLOAT_EFFECT;
		}

		// single collect effect
		blue_effect();
	}

	if (check_ladder_level_up() == TRUE)
	{
		player.inLeveling = TRUE;
		upOnLadder();
	}
	else if (player.position.y == floors[0].y)
	{
		player.position.x = ladder.position.x;
		player.initializing = TRUE;
	}

	player_initialize_down_onLadder();

	createEnemies();

}

//Function to check if the player is in state that can be moved
int canMove()
{
	return player.initializing == FALSE && player.inLeveling == FALSE && check_ladder_fall() == FALSE && gameOver == FALSE && gameWin == FALSE && solar.playerHold == FALSE;
}

//Define new handler for interrupt 0x9
INTPROC new_int9(int mdevno)
{
	char result = 0;
	int scan = 0;
	int ascii = 0;
	asm{
		MOV AH,1
		INT 16h
		JZ Skip1
		MOV AH,0
		INT 16h
		MOV BYTE PTR scan,AH
		MOV BYTE PTR ascii,AL
	} //asm

		if ((scan == 46) && (ascii == 3)) // Ctrl-C?
			asm INT 27; // terminate xinu 

	if (canMove() == TRUE)
	{
		if (scan == 75)
			result = LEFT;
		else if (scan == 72)
			result = 'w';
		else if (scan == 77)
			result = RIGHT;
		else if (scan == 80)
			result = DOWN;
		else if (scan == 57) // space
			result = 'j';

		send(receiver_pid, result);
	}
Skip1:
}
//Function to set a new handler for interrupt 0x9 in the sys_imp array.
void set_new_int9_newisr()
{
	int i;
	for (i = 0; i < 32; i++)
		if (sys_imp[i].ivec == 9)
		{
			sys_imp[i].newisr = new_int9;
			return;
		}
}
//Function which receiving a floor
//and then copies her display data to the display_draft matrix by y axis position
void displayFloor(Floor floor)
{
	int i;
	for (i = 0; i < COLS; i++)
	{
		display_draft[floor.y][i] = floor.blocks[i];
	}
}
//Function which receiving a ladder pointer
//and then copies his display data to the display_draft matrix by x and y axis position
void displayLadder(Ladder* ladder)
{
	int screenIndex;
	for (screenIndex = 0; screenIndex < ROWS; screenIndex++)
	{
		display_draft[screenIndex][COLS / 2] = ladder->blocks[screenIndex];
		display_draft[screenIndex][COLS / 2 + 1] = ladder->blocks[screenIndex];
	}
}

//Function which receiving an elevator pointer
//and then copies his display data to the display_draft matrix by x and y axis position
void displayElevator(Elevator* elevator)
{
	int i;
	for (i = 0; i < elevator->width; i++)
	{
		display_draft[elevator->position.y][i + elevator->position.x] = elevator->blocks[i];
	}
}
//Function which displaying all of the elevators 
void displayElevators(Elevator elevators[])
{
	int i;
	for (i = 0; i < ELEVATORS_LENGTH; i++)
		displayElevator(&elevators[i]);
}
//Function which displaying all of the floors
void displayFloors(Floor floors[])
{
	int i;
	for (i = 0; i < FLOORS_LENGTH; i++)
		displayFloor(floors[i]);
}
//Process that enables the solar to random floor for a while, and then hides it for another time
void createSolar(void)
{
	int floor;
	while (TRUE)
	{
		if (solar.playerHold == FALSE)
		{
			floor = getRandomRange(0, 3);
			initSolar(&solar, floor, TRUE, LTR);
		}

		sleep(30);

		if (solar.playerHold == FALSE)
		{
			solar.active = FALSE;
			sleep(20);
		}
	}
}

// Process that copies from display draft array to the screen 
void displayer(void)
{
	while (TRUE)
	{
		int i;
		receive();
		for (i = 0; i < ROWS*COLS * 2; i += 2)
		{
			b800h[i] = display[i / 2].character;
			b800h[i + 1] = display[i / 2].attribute;
		}
	} //while
} 

// Process that receive the character after pressing on keyboard
void receiver()
{
	while (TRUE)
	{
		char temp;
		temp = receive();
		rear++;
		ch_arr[rear] = temp;
		if (front == -1)
			front = 0;
		//getc(CONSOLE);
	} // while
} //  receiver

// Function that delete the upper elevators
void destroyElevator(Elevator* elevator)
{
	int i;
	for (i = 0; i < 16; i++)
	{
		elevator->blocks[i].character = ' ';
		elevator->blocks[i].attribute = 0x70;
	}
}

// Function that move all the elevators to the next floor
void moveElevators()
{
	int i;

	if (elevatorsActive == TRUE) 
	{
		elevatorsMoving = TRUE;
		for (i = 0; i < ELEVATORS_LENGTH; i++)
		{
			if (elevators[i].position.y > floors[i / 2 + 1].y - floors[i / 2 + 1].height) {

				elevators[i].position.y--;

				if (player.onElevator == TRUE)
					player.position.y = player.elevator->position.y - player.width;
			}
			else if (elevators[i].position.y == 0) // destroy the upper elevator when finish moving up
			{
				destroyElevator(&elevators[i]);
			}
			else if (elevators[ELEVATORS_LENGTH - 1].position.y <= floors[FLOORS_LENGTH - 1].y - floors[FLOORS_LENGTH - 1].height) // elevators finished moving
			{
				if (player.onElevator == TRUE)
				{
					switch (player.currentFloor.number)
					{
					case 3: player.currentFloor = floors[3]; player.position.y = player.currentFloor.y - player.height; break;
					case 2: player.currentFloor = floors[2]; player.position.y = player.currentFloor.y - player.height; break;
					case 1: player.currentFloor = floors[1]; player.position.y = player.currentFloor.y - player.height; break;
					}
					player.onElevator = FALSE;
				}

				elevatorsActive = FALSE;
			}
		}
	}
	else {
		initElevators(elevators);
		elevatorsMoving = FALSE;
	}
}

// Function that check if the player passed over the ladder
int checkForPassingLadder(int direction)
{
	if (direction == LEFT || direction == JUMP_LEFT)
	{
		if (ladder.passed_from_left == FALSE && player.position.x + player.width - 1 < ladder.position.x)
		{
			ladder.passed_from_left = TRUE;
			ladder.passed_from_right = FALSE;
			return TRUE;
		}
		else
			return FALSE;
	}
	else if (direction == RIGHT || direction == JUMP_RIGHT)
	{
		if (ladder.passed_from_right == FALSE && player.position.x > ladder.position.x + ladder.width - 1)
		{
			ladder.passed_from_right = TRUE;
			ladder.passed_from_left = FALSE;
			return TRUE;
		}
		else
			return FALSE;
	}

	return FALSE;
}

// Function that cause to the player to fall previous floor
void fallDownFloor()
{

	if (player.goDown && player.position.y < player.currentFloor.y - player.height)
	{
		player.position.y++;
	}
	else
	{
		player.goDown = FALSE;
		player.inLeveling = FALSE;
	}
}

// Function that check if the player need to fall down on the ladder without junping over it
int check_ladder_fall()
{
	return (!player.canLevelUp && player.position.x == ladder.position.x && !player.isJumping && player.currentFloor.number != 3);
}

// Function that check if the player is on the ladder for enable the next level
int check_ladder_level_up()
{
	return ((player.position.x >= ladder.position.x - 1 && player.position.x <= ladder.position.x + 1) && player.canLevelUp == TRUE);
}

// Function that moves the player, and check if there is a collosion with enemies, or solar
void handlePlayerMovement()
{
	int i;

	if (player.direction == LEFT || player.direction == JUMP_LEFT)
	{
		if (player.position.x > 0)
			player.position.x--;

		if (player.isJumping)
		{
			player.direction = JUMP_LEFT;
		}
		else
		{
			player.direction = STANDING;
		}
	}
	else if (player.direction == RIGHT || player.direction == JUMP_RIGHT)
	{
		if (player.position.x < COLS - player.width)
			player.position.x++;
		if (player.isJumping)
		{
			player.direction = JUMP_RIGHT;
		}
		else
		{
			player.direction = STANDING;
		}
	}
	// jumping process
	if (solar.playerHold == FALSE && (player.isJumping == TRUE || player.direction == JUMP_RIGHT || player.direction == JUMP_LEFT))
	{
		if (!player.isFalling)
		{
			if (player.position.y >= (player.currentFloor.y - player.height * 2)) {
				player.position.y--;
			}
			else {
				player.isFalling = TRUE;
			}
		}
		// fall down process
		else
		{
			if (player.position.y < player.currentFloor.y - player.height) {
				player.position.y++;
			}
			else {
				player.isFalling = FALSE;
				player.isJumping = FALSE;
			}
		}
	}


	// colliision enemy
	for (i = 0; i < ENEMY_LENGTH; i++)
	{
		if ((enemies[i].in_game == TRUE && check_collision(player, enemies[i].position, enemies[i].width, enemies[i].height) == TRUE) ||
			(enemies[i].bullet.active == TRUE && check_collision(player, enemies[i].bullet.position, enemies[i].bullet.width, enemies[i].bullet.height) == TRUE))
		{
			initEnemies(enemies);
			enemyWait = FALSE;

			if (player.life > 0)
			{
				player.life--;
			}

			if (player.life == 0)
			{
				// game over
				gameOver = TRUE;
			}
			else
			{
				player.position.x = ladder.position.x;
				player.position.y = 0;
				player.initializing = TRUE;
			}

			break;
		}
	}

	if (solar.active == TRUE && solar.playerHold == FALSE && player.goDown == FALSE && player.onElevator == FALSE && check_player_on_solar())
	{
		solar.playerHold = TRUE;
	}
}

// Process that updates the movements of player and elevators  
void updater()
{
	int i, j;
	char ch;
	while (TRUE)
	{
		receive();
		while (front != -1)
		{
			ch = ch_arr[front];
			if (front != rear)
				front++;
			else
				front = rear = -1;
			if (ch == LEFT) // left arrow
			{
				player.direction = LEFT;
			}
			else if (ch == RIGHT) // right arrow
			{
				player.direction = RIGHT;
			}
			else if (ch == DOWN)
			{
				if (player.goDown == FALSE) {

					switch (player.currentFloor.number)
					{
					case 0: player.currentFloor = floors[2]; break;
					case 1: player.currentFloor = floors[3]; break;
					case 2: player.currentFloor = floors[4]; break;
					}

					player.goDown = TRUE;
				}
			}
			else if ((ch == 'w') || (ch == 'W')) // up arrow
			{
				/*if can active elevators*/
				if (elevatorsMoving == FALSE && ladder.isPassed == TRUE && is_player_onElevator() == TRUE)
				{
					if (player.currentFloor.number != 0) {
						player.onElevator = TRUE;

						elevatorsActive = TRUE;
						ladder.isPassed = FALSE;
					}
				}
			}
			else if ((ch == 'j') || (ch == 'J')) // space
			{
				if (player.isJumping == FALSE)
				{
					player.isJumping = TRUE;
					player.isFalling = FALSE;

					Sound(500);
				}
			} // if
		} // while(front != -1)

		if (elevatorsMoving == FALSE)
		{
			int direction;

			if (solar.playerHold == TRUE)
			{
				if (solar.direction == RTL)
					direction = LEFT;
				else if (solar.direction == LTR)
					direction = RIGHT;
			}
			else
			{
				direction = player.direction;
			}

			if (checkForPassingLadder(direction) == TRUE)
			{
				ladder.isPassed = TRUE;
				elevatorsActive = TRUE;
			}
		}

		if (canMove())
		{
			handlePlayerMovement();
		}

		if (player.goDown == TRUE && player.isJumping == FALSE)
		{
			fallDownFloor();
		}

		if (check_ladder_fall() == TRUE)
		{
			if (player.position.y < floors[FLOORS_LENGTH - 1].y - player.height)
			{
				player.position.y++;
			}
			else
			{
				player.currentFloor = floors[FLOORS_LENGTH - 1];
				ladder.passed_from_left = ladder.passed_from_right = FALSE;
			}
		}

		blank.character = ' ';
		blank.attribute = 0;
		for (i = 0; i < 25; i++)
			for (j = 0; j < 80; j++)
			{
				display_draft[i][j] = blank;  // blank
			}

		display_gameStatus();
		displayFloors(floors);
		displayLadder(&ladder);
		displayElevators(elevators);
		displaySolar(&solar);
		displayPlayer(&player);
		displayEnemies(enemies);
		displayFloatingBits();


		/*
		b800h[86] = (enemies[0].speed / 10) + '0';
		b800h[88] = (enemies[0].speed % 10) + '0';

		b800h[92] = (enemies[3].speed / 10) + '0';
		b800h[94] = (enemies[3].speed % 10) + '0';

		b800h[98] = (enemies[5].speed / 10) + '0';
		b800h[100] = (enemies[5].speed % 10) + '0';
		*/

		moveElevators();

		for (i = 0; i < ROWS; i++)
			for (j = 0; j < COLS; j++)
				display[i * COLS + j] = display_draft[i][j];
	} // while(1)
} // updater

int sched_arr_pid[5];
int sched_arr_int[5];
SYSCALL schedule(int no_of_pids, int cycle_length, int pid1, ...)
{
	int i;
	int ps;
	int *iptr;
	disable(ps);
	gcycle_length = cycle_length;
	point_in_cycle = 0;
	gno_of_pids = no_of_pids;
	iptr = &pid1;
	for (i = 0; i < no_of_pids; i++)
	{
		sched_arr_pid[i] = *iptr;
		iptr++;
		sched_arr_int[i] = *iptr;
		iptr++;
	} // for
	restore(ps);
} // schedule

void set70h_handler()
{
	asm{
		CLI         // Disable interrupts
		PUSH AX     // Interrupt may occur while updating
		IN AL,0A1h  // Make sure IRQ8 is not masked
		MOV old_0A1h_mask,AL
		AND AL,0FEh // Set bit 0 of port 0A1 to zero
		OUT 0A1h,AL //
		IN AL,70h   // Set up "Write into status register A"
		MOV AL,0Ah  //
		OUT 70h,AL  //
		MOV AL,8Ah  //
		OUT 70h,AL  //
		IN AL,71h   //
		MOV BYTE PTR x71h1,AL  // Save old value
		MOV old_70h_A_mask,AL
		AND AL,11110000b // Change only Rate
		OR AL,1001b // Make sure it is Rate =0110 (1Khz)
		OUT 71h,AL  // Write into status register A
		IN AL,71h   // Read to confirm write
		IN AL,70h  // Set up "Write into status register B"
		MOV AL,0Bh //
		OUT 70h,AL //
		MOV AL,8Bh //
		OUT 70h,AL //
		IN AL,71h  //
		MOV BYTE PTR x71h2,AL // Save Old value
		AND AL,8Fh // Mask out PI,AI,UI
		OR AL,40h  // Enable periodic interrupts (PI=1) only
		OUT 71h,AL // Write into status register  B
		IN AL,71h  // Read to confirm write
		MOV byte ptr x71h3,AL // Save old value
		IN AL,021h  // Make sure IRQ2 is not masked
		AND AL,0FBh // Write 0 to bit 2 of port 21h
		OUT 021h,AL // Write to port 21h
		IN AL,70h  // Set up "Read into status resister C"
		MOV AL,0Ch // Required for for "Write into port 71h"
		OUT 70h,AL
		IN AL,70h
		MOV AL,8Ch //
		OUT 70h,AL
		IN AL,71h  // Read status register C
				   // (we do nothing with it)
		IN AL,70h  // Set up "Read into status resister C"
		MOV AL,0Dh // Required for for "Write into port 71h"
		OUT 70h,AL
		IN AL,70h
		MOV AL,8Dh
		OUT 70h,AL
		IN AL,71h  // Read status register D
				   // (we do nothing with it)
		STI
		POP AX
	} // asm
}
xmain()
{
	int i, solarpid, enemypid, uppid, dispid, recvpid;
	b800h = (unsigned char far *)0xB8000000;
	mapinit(0x70, newISR70h, 0x70);
	srand(time(0));
	initFloors(floors);
	initElevators(elevators);
	initLadder(&ladder);
	initEnemies(enemies);
	initFloatingBits(floatingBits);
	initPlayer(&player);
	resume(dispid = create(displayer, INITSTK, INITPRIO, "DISPLAYER", 0));
	resume(recvpid = create(receiver, INITSTK, INITPRIO + 3, "RECEIVER", 0));
	resume(uppid = create(updater, INITSTK, INITPRIO, "UPDATER", 0));
	resume(solarpid = create(createSolar, INITSTK, INITPRIO, "CREATE_SOLAR", 0));
	receiver_pid = recvpid;
	set_new_int9_newisr();
	set70h_handler();
	schedule(5, 2, dispid, 0, uppid, 1);
} // xmain
