#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#define W 30
#define H 30
#define SPD 200000
#define SCORES_FILE "scores.txt"

enum
{
    U,
    D,
    L,
    R
};

typedef struct
{
    int x, y;
} Pos;

typedef struct
{
    char n[50];
    int s;
} Score;

Score scores[100];
int numPlayers = 0;

Pos snake[900];
int len;
Pos food;
int dir;
int pts;
int spd;
char name[50];

void saveScores();
void loadScores();
void draw();
void init();
int isSnake(int x, int y);
void moveBody();
void moveHead();
void check();
void faster();
void move();
void changeDir(char k);
void setInput();
int keyPressed();
char getKey();

void setInput()
{
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~ICANON;
    t.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

int keyPressed()
{
    struct termios old, new;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &old);
    new = old;
    new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

char getKey() { return getchar(); }

void loadScores()
{
    FILE *file = fopen(SCORES_FILE, "r");
    if (!file)
        return;

    numPlayers = 0;
    while (fscanf(file, "%s %d", scores[numPlayers].n, &scores[numPlayers].s) != EOF)
    {
        numPlayers++;
    }
    fclose(file);
}

void saveScores()
{
    int found = 0;

    for (int i = 0; i < numPlayers; i++)
    {
        if (strcmp(scores[i].n, name) == 0)
        {
            if (pts > scores[i].s)
            {
                scores[i].s = pts;
            }
            found = 1;
            break;
        }
    }

    if (!found)
    {
        strcpy(scores[numPlayers].n, name);
        scores[numPlayers].s = pts;
        numPlayers++;
    }

    FILE *file = fopen(SCORES_FILE, "w");
    for (int i = 0; i < numPlayers; i++)
    {
        fprintf(file, "%s %d\n", scores[i].n, scores[i].s);
    }
    fclose(file);
}

void init()
{
    len = 1;
    snake[0].x = W / 2;
    snake[0].y = H / 2;
    dir = R;
    pts = 0;
    spd = SPD;

    srand(time(0));
    food.x = rand() % W;
    food.y = rand() % H;

    loadScores();
}

int isSnake(int x, int y)
{
    for (int i = 0; i < len; i++)
    {
        if (snake[i].x == x && snake[i].y == y)
            return 1;
    }
    return 0;
}

void draw()
{
    system("clear");
    printf("Player: %s | Score: %d\n", name, pts);

    int max = 0;
    char top[50];
    for (int i = 0; i < numPlayers; i++)
    {
        if (scores[i].s > max)
        {
            max = scores[i].s;
            strcpy(top, scores[i].n);
        }
    }
    printf("Highest: %s (%d)\n", top, max);

    for (int i = 0; i < H; i++)
    {
        for (int j = 0; j < W; j++)
        {
            if (isSnake(j, i))
                printf("O ");
            else if (food.x == j && food.y == i)
                printf("F ");
            else
                printf(". ");
        }
        printf("\n");
    }
}

void moveBody()
{
    for (int i = len - 1; i > 0; i--)
    {
        snake[i] = snake[i - 1];
    }
}

void moveHead()
{
    switch (dir)
    {
    case U:
        snake[0].y--;
        break;
    case D:
        snake[0].y++;
        break;
    case L:
        snake[0].x--;
        break;
    case R:
        snake[0].x++;
        break;
    }
}

void check()
{
    if (snake[0].x < 0 || snake[0].x >= W || snake[0].y < 0 || snake[0].y >= H)
    {
        printf("Game Over! Score: %d\n", pts);
        saveScores();
        exit(0);
    }

    for (int i = 1; i < len; i++)
    {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
        {
            printf("Game Over! Score: %d\n", pts);
            saveScores();
            exit(0);
        }
    }
}

void faster()
{
    if (len % 5 == 0)
    {
        spd = spd > 50000 ? spd - 2000 : spd;
    }
}

void move()
{
    moveBody();
    moveHead();
    check();

    if (snake[0].x == food.x && snake[0].y == food.y)
    {
        len++;
        pts++;
        food.x = rand() % W;
        food.y = rand() % H;
        faster();
    }
}

void changeDir(char k)
{
    if (k == 27)
    {
        char k2 = getKey();
        if (k2 == 91)
        {
            char k3 = getKey();
            switch (k3)
            {
            case 'A':
                if (dir != D)
                    dir = U;
                break;
            case 'B':
                if (dir != U)
                    dir = D;
                break;
            case 'C':
                if (dir != L)
                    dir = R;
                break;
            case 'D':
                if (dir != R)
                    dir = L;
                break;
            }
        }
    }
    else
    {
        switch (tolower(k))
        {
        case 'w':
            if (dir != D)
                dir = U;
            break;
        case 's':
            if (dir != U)
                dir = D;
            break;
        case 'a':
            if (dir != R)
                dir = L;
            break;
        case 'd':
            if (dir != L)
                dir = R;
            break;
        }
    }
}

int main()
{
    setInput();
    init();

    int valid;
    do
    {
        valid = 1;
        printf("Enter name:\n");
        scanf("%s", name);

        for (int i = 0; i < numPlayers; i++)
        {
            if (strcmp(scores[i].n, name) == 0)
            {
                printf("Name '%s' exists.\nUse it? (y/n):\n", name);
                char c;
                scanf(" %c", &c);

                if (tolower(c) == 'y')
                {
                    printf("Welcome back, %s!\n", name);
                    valid = 1;
                }
                else
                {
                    printf("Enter new name.\n");
                    valid = 0;
                }
                break;
            }
        }
    } while (!valid);

    printf("Start game...\n");
    while (!keyPressed())
        ;
    getKey();

    while (1)
    {
        draw();
        if (keyPressed())
        {
            char k = getKey();
            if (k == 'p')
            {
                printf("Paused. Press key...\n");
                while (!keyPressed())
                    ;
                getKey();
            }
            else
            {
                changeDir(k);
            }
        }
        move();
        usleep(spd);
    }

    return 0;
}
