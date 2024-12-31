#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // For usleep
#include <termios.h> // For terminal input handling
#include <fcntl.h>   // For non-blocking input
#include <time.h>    // For random number generation
#include <ctype.h>   // For tolower function
#include <string.h>  // For string handling

#define WIDTH 30
#define HEIGHT 30
#define INITIAL_SPEED 200000
#define SCORE_FILE "scores.txt"

// Directions
enum
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

typedef struct
{
    int x, y;
} Position;

typedef struct
{
    char name[50];
    int score;
} PlayerScore;

PlayerScore playerScores[100];
int playerCount = 0;

Position snake[900]; // Max length of the snake
int snakeLength;
Position food;
int direction;
int score;
int speed; // Speed of the game
char playerName[50];

// Function prototypes
void updateScoreFile();
void loadScores();
void drawGrid();
void initializeGame();
int isSnake(int x, int y);
void updateSnakeBody();
void updateSnakeHead();
void checkCollisions();
void increaseSpeed();
void moveSnake();
void changeDirection(char key);
void setNonBlockingInput();
int kbhit();
char getch();

void setNonBlockingInput()
{
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~ICANON; // Disable canonical mode
    t.c_lflag &= ~ECHO;   // Disable echo
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

int kbhit()
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

char getch()
{
    return getchar();
}

void loadScores()
{
    FILE *file = fopen(SCORE_FILE, "r");
    if (file == NULL)
        return;

    playerCount = 0;
    while (fscanf(file, "%s %d", playerScores[playerCount].name, &playerScores[playerCount].score) != EOF)
    {
        playerCount++;
    }

    fclose(file);
}

void updateScoreFile()
{
    int found = 0;

    // Update score if player exists
    for (int i = 0; i < playerCount; i++)
    {
        if (strcmp(playerScores[i].name, playerName) == 0)
        {
            if (score > playerScores[i].score)
            {
                playerScores[i].score = score; // Update only if the score is higher
            }
            found = 1;
            break;
        }
    }

    // Add new player if not found
    if (!found)
    {
        strcpy(playerScores[playerCount].name, playerName);
        playerScores[playerCount].score = score;
        playerCount++;
    }

    // Save updated scores
    FILE *file = fopen(SCORE_FILE, "w");
    for (int i = 0; i < playerCount; i++)
    {
        fprintf(file, "%s %d\n", playerScores[i].name, playerScores[i].score);
    }

    fclose(file);
}

void initializeGame()
{
    snakeLength = 1;
    snake[0].x = WIDTH / 2;
    snake[0].y = HEIGHT / 2;
    direction = RIGHT;
    score = 0;
    speed = INITIAL_SPEED; // Start with the initial speed

    srand(time(0));
    food.x = rand() % WIDTH;
    food.y = rand() % HEIGHT;

    loadScores();
}

int isSnake(int x, int y)
{
    for (int i = 0; i < snakeLength; i++)
    {
        if (snake[i].x == x && snake[i].y == y)
            return 1;
    }
    return 0;
}

void drawGrid()
{
    system("clear");
    printf("Player: %s | Score: %d\n", playerName, score);

    int maxScore = 0;
    char maxPlayer[50];
    for (int i = 0; i < playerCount; i++)
    {
        if (playerScores[i].score > maxScore)
        {
            maxScore = playerScores[i].score;
            strcpy(maxPlayer, playerScores[i].name);
        }
    }
    printf("Highest Score: %s (%d)\n", maxPlayer, maxScore);

    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            if (isSnake(j, i))
            {
                printf("O ");
            }
            else if (food.x == j && food.y == i)
            {
                printf("F ");
            }
            else
            {
                printf(". ");
            }
        }
        printf("\n");
    }
}

void updateSnakeBody()
{
    for (int i = snakeLength - 1; i > 0; i--)
    {
        snake[i] = snake[i - 1];
    }
}

void updateSnakeHead()
{
    switch (direction)
    {
    case UP:
        snake[0].y--;
        break;
    case DOWN:
        snake[0].y++;
        break;
    case LEFT:
        snake[0].x--;
        break;
    case RIGHT:
        snake[0].x++;
        break;
    }
}

void checkCollisions()
{
    if (snake[0].x < 0 || snake[0].x >= WIDTH || snake[0].y < 0 || snake[0].y >= HEIGHT)
    {
        printf("Game Over! Final Score: %d\n", score);
        updateScoreFile();
        exit(0);
    }

    for (int i = 1; i < snakeLength; i++)
    {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
        {
            printf("Game Over! Final Score: %d\n", score);
            updateScoreFile();
            exit(0);
        }
    }
}

void increaseSpeed()
{
    if (snakeLength % 5 == 0)
    {
        speed = speed > 50000 ? speed - 2000 : speed;
    }
}

void moveSnake()
{
    updateSnakeBody();
    updateSnakeHead();
    checkCollisions();

    if (snake[0].x == food.x && snake[0].y == food.y)
    {
        snakeLength++;
        score++;
        food.x = rand() % WIDTH;
        food.y = rand() % HEIGHT;
        increaseSpeed();
    }
}

void changeDirection(char key)
{
    if (key == 27)
    {
        char secondKey = getchar();
        if (secondKey == 91)
        {
            char arrowKey = getchar();
            switch (arrowKey)
            {
            case 'A':
                if (direction != DOWN)
                    direction = UP;
                break;
            case 'B':
                if (direction != UP)
                    direction = DOWN;
                break;
            case 'C':
                if (direction != LEFT)
                    direction = RIGHT;
                break;
            case 'D':
                if (direction != RIGHT)
                    direction = LEFT;
                break;
            }
        }
    }
    else
    {
        switch (tolower(key))
        {
        case 'w':
            if (direction != DOWN)
                direction = UP;
            break;
        case 's':
            if (direction != UP)
                direction = DOWN;
            break;
        case 'a':
            if (direction != RIGHT)
                direction = LEFT;
            break;
        case 'd':
            if (direction != LEFT)
                direction = RIGHT;
            break;
        }
    }
}

int main()
{
    setNonBlockingInput();
    initializeGame();

    int isNameValid;

    do
    {
        isNameValid = 1;
        printf("Enter your name:\n");
        scanf("%s", playerName);

        for (int i = 0; i < playerCount; i++)
        {
            if (strcmp(playerScores[i].name, playerName) == 0)
            {
                printf("Name '%s' already exists.\nDo you want to use this name? (y/n):\n", playerName);
                char choice;
                scanf(" %c", &choice);

                if (tolower(choice) == 'y')
                {
                    printf("Welcome back, %s!\n", playerName);
                    isNameValid = 1;
                }
                else
                {
                    printf("Please enter a different name.\n");
                    isNameValid = 0;
                }
                break;
            }
        }
    } while (!isNameValid);

    printf("Welcome, %s!\n", playerName);
    printf("Press any key to start the game...\n");
    while (!kbhit())
        ;
    getch();

    while (1)
    {
        drawGrid();
        if (kbhit())
        {
            char key = getch();
            if (key == 'p')
            {
                printf("Game Paused. Press any key to continue...\n");
                while (!kbhit())
                    ;
                getch();
            }
            else
            {
                changeDirection(key);
            }
        }
        moveSnake();
        usleep(speed);
    }

    return 0;
}