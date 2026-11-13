/*
 * CROSS ROAD GAME - Computer Graphics Mini Project
 * Language: C | Graphics: OpenGL + GLUT / FreeGLUT
 *
 * Controls:
 *   Arrow keys : Move the player
 *   ENTER/SPACE: Start or continue to the next level
 *   R          : Restart the entire game (3 lives, Level 1, score 0)
 *   Q / ESC    : Quit
 *
 * Features:
 *   - Three difficulty levels: Easy, Medium and Hard
 *   - Three lives shared across all three levels
 *   - A collision costs one life and returns the player to the start
 *   - Game over when all lives are lost
 *   - Score: +10 for each forward step, +100 for each level completed
 *   - All code is contained in this single C file
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <GL/glu.h>

#define WINDOW_W 900
#define WINDOW_H 700
#define PI 3.14159265358979323846f
#define CAR_COUNT 12
#define LANE_COUNT 6
#define MAX_LIVES 3

enum {
    SCREEN_INSTRUCTIONS,
    SCREEN_PLAYING,
    SCREEN_LEVEL_COMPLETE,
    SCREEN_GAME_OVER,
    SCREEN_GAME_WON
};

typedef struct {
    float x, y, speed, width, height;
    float r, g, b;
} Car;

typedef struct {
    float x, y;
} Player;

static int gameState = SCREEN_INSTRUCTIONS;
static Player player = {50.0f, 7.0f};
static Car cars[CAR_COUNT];
static int level = 1;
static int lives = MAX_LIVES;
static int score = 0;
static int collisionFlash = 0;
static int windowWidth = WINDOW_W;
static int windowHeight = WINDOW_H;

static const float laneY[LANE_COUNT] = {20, 31, 42, 53, 64, 75};
static const float levelSpeed[3] = {0.80f, 1.10f, 1.70f};
static const char *levelName[3] = {"EASY", "MEDIUM", "HARD"};

void setColor(float r, float g, float b)
{
    glColor3f(r, g, b);
}

void drawRect(float x, float y, float w, float h,
              float r, float g, float b)
{
    setColor(r, g, b);
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
    glEnd();
}

void drawCircle(float cx, float cy, float radius,
                float r, float g, float b)
{
    int i;
    setColor(r, g, b);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (i = 0; i <= 40; ++i) {
            float a = 2.0f * PI * (float)i / 40.0f;
            glVertex2f(cx + cosf(a) * radius,
                       cy + sinf(a) * radius);
        }
    glEnd();
}

void drawText(float x, float y, const char *text, void *font)
{
    const unsigned char *p;
    glRasterPos2f(x, y);
    for (p = (const unsigned char *)text; *p; ++p)
        glutBitmapCharacter(font, *p);
}

/* Draw a heart from two circles and a triangle. */
void drawHeart(float x, float y, int filled)
{
    float r = filled ? 0.95f : 0.30f;
    float g = filled ? 0.12f : 0.30f;
    float b = filled ? 0.18f : 0.30f;

    drawCircle(x - 0.8f, y + 0.5f, 1.15f, r, g, b);
    drawCircle(x + 0.8f, y + 0.5f, 1.15f, r, g, b);

    setColor(r, g, b);
    glBegin(GL_TRIANGLES);
        glVertex2f(x - 1.9f, y + 0.3f);
        glVertex2f(x + 1.9f, y + 0.3f);
        glVertex2f(x, y - 2.0f);
    glEnd();
}

void drawLives(void)
{
    int i;
    for (i = 0; i < MAX_LIVES; ++i)
        drawHeart(78.0f + i * 5.0f, 96.0f, i < lives);
}

void drawRoad(void)
{
    int i;

    drawRect(0, 0, 100, 100, 0.20f, 0.58f, 0.22f);
    drawRect(0, 13, 100, 74, 0.22f, 0.23f, 0.25f);
    drawRect(0, 13, 100, 2, 0.72f, 0.72f, 0.70f);
    drawRect(0, 85, 100, 2, 0.72f, 0.72f, 0.70f);
    drawRect(0, 15, 100, 0.45f, 0.95f, 0.80f, 0.10f);
    drawRect(0, 84.55f, 100, 0.45f, 0.95f, 0.80f, 0.10f);

    setColor(0.88f, 0.88f, 0.86f);
    glLineWidth(2.0f);
    for (i = 0; i < 5; ++i) {
        float y = 25.5f + i * 11.0f;
        int x;
        glBegin(GL_LINES);
        for (x = 2; x < 100; x += 10) {
            glVertex2f((float)x, y);
            glVertex2f((float)x + 5.0f, y);
        }
        glEnd();
    }

    drawRect(0, 87, 100, 4, 0.95f, 0.85f, 0.15f);
    setColor(0.10f, 0.10f, 0.10f);
    drawText(43, 88.5f, "FINISH", GLUT_BITMAP_HELVETICA_12);
    drawRect(0, 3, 100, 2, 0.95f, 0.85f, 0.15f);
}

void drawCar(const Car *car)
{
    float x = car->x, y = car->y;
    float w = car->width, h = car->height;

    drawRect(x + w * 0.16f, y - 0.7f, w * 0.20f, 1.4f, 0.05f, 0.05f, 0.05f);
    drawRect(x + w * 0.64f, y - 0.7f, w * 0.20f, 1.4f, 0.05f, 0.05f, 0.05f);
    drawRect(x + w * 0.16f, y + h - 0.7f, w * 0.20f, 1.4f, 0.05f, 0.05f, 0.05f);
    drawRect(x + w * 0.64f, y + h - 0.7f, w * 0.20f, 1.4f, 0.05f, 0.05f, 0.05f);

    drawRect(x, y, w, h, car->r, car->g, car->b);
    drawRect(x + w * 0.28f, y + h * 0.18f, w * 0.42f, h * 0.64f,
             0.20f, 0.72f, 0.88f);
    drawRect(x + w - 0.8f, y + 0.5f, 0.55f, 0.8f, 1.0f, 0.95f, 0.55f);
    drawRect(x + w - 0.8f, y + h - 1.3f, 0.55f, 0.8f, 1.0f, 0.95f, 0.55f);
}

void drawCars(void)
{
    int i;
    for (i = 0; i < CAR_COUNT; ++i)
        drawCar(&cars[i]);
}

void drawPlayer(void)
{
    float x = player.x, y = player.y;

    drawCircle(x - 1.5f, y - 1.0f, 1.6f, 0.10f, 0.42f, 0.12f);
    drawCircle(x + 1.5f, y - 1.0f, 1.6f, 0.10f, 0.42f, 0.12f);
    drawCircle(x, y, 3.0f, 0.20f, 0.78f, 0.25f);
    drawCircle(x - 1.6f, y + 2.0f, 1.3f, 0.25f, 0.88f, 0.30f);
    drawCircle(x + 1.6f, y + 2.0f, 1.3f, 0.25f, 0.88f, 0.30f);
    drawCircle(x - 1.6f, y + 2.1f, 0.55f, 1, 1, 1);
    drawCircle(x + 1.6f, y + 2.1f, 0.55f, 1, 1, 1);
    drawCircle(x - 1.6f, y + 2.1f, 0.23f, 0.02f, 0.02f, 0.02f);
    drawCircle(x + 1.6f, y + 2.1f, 0.23f, 0.02f, 0.02f, 0.02f);
}

/* Car speed increases with each level. */
void initialiseCars(void)
{
    int i;
    float startX[CAR_COUNT] = {
        -5, 34, 75, 108, 8, 55, 95, -20, 20, 65, 110, -35
    };
    float colours[6][3] = {
        {0.95f, 0.12f, 0.15f}, {0.10f, 0.25f, 0.95f},
        {0.85f, 0.10f, 0.80f}, {0.10f, 0.80f, 0.25f},
        {0.95f, 0.55f, 0.08f}, {0.05f, 0.75f, 0.85f}
    };

    for (i = 0; i < CAR_COUNT; ++i) {
        int lane = i % LANE_COUNT;
        int colour = i % 6;
        float baseSpeed = 0.55f + 0.08f * (i % 3);

        cars[i].x = startX[i];
        cars[i].y = laneY[lane] - 2.2f;
        cars[i].width = 12.0f;
        cars[i].height = 4.4f;
        cars[i].speed = ((lane % 2 == 0) ? baseSpeed : -baseSpeed)
                       * levelSpeed[level - 1];
        cars[i].r = colours[colour][0];
        cars[i].g = colours[colour][1];
        cars[i].b = colours[colour][2];
    }
}

void resetPlayer(void)
{
    player.x = 50.0f;
    player.y = 7.0f;
}

/* Restart the whole game, including level, score and all three lives. */
void restartGame(void)
{
    level = 1;
    lives = MAX_LIVES;
    score = 0;
    collisionFlash = 0;
    resetPlayer();
    initialiseCars();
    gameState = SCREEN_PLAYING;
    glutPostRedisplay();
}

int checkCollision(void)
{
    int i;
    float playerLeft = player.x - 2.2f;
    float playerRight = player.x + 2.2f;
    float playerBottom = player.y - 2.0f;
    float playerTop = player.y + 3.0f;

    for (i = 0; i < CAR_COUNT; ++i) {
        float carLeft = cars[i].x;
        float carRight = cars[i].x + cars[i].width;
        float carBottom = cars[i].y;
        float carTop = cars[i].y + cars[i].height;

        if (playerRight > carLeft && playerLeft < carRight &&
            playerTop > carBottom && playerBottom < carTop)
            return 1;
    }
    return 0;
}

void updateCars(void)
{
    int i;
    for (i = 0; i < CAR_COUNT; ++i) {
        cars[i].x += cars[i].speed;

        if (cars[i].speed > 0.0f && cars[i].x > 105.0f)
            cars[i].x = -cars[i].width - 5.0f;
        else if (cars[i].speed < 0.0f && cars[i].x < -cars[i].width - 5.0f)
            cars[i].x = 105.0f;
    }
}

void timer(int value)
{
    (void)value;

    if (gameState == SCREEN_PLAYING) {
        updateCars();

        if (checkCollision()) {
            --lives;
            collisionFlash = 45;
            resetPlayer();

            if (lives <= 0)
                gameState = SCREEN_GAME_OVER;
        }

        if (gameState == SCREEN_PLAYING && player.y >= 90.0f) {
            score += 100;
            if (level < 3) {
                gameState = SCREEN_LEVEL_COMPLETE;
            } else {
                gameState = SCREEN_GAME_WON;
            }
        }

        if (collisionFlash > 0)
            --collisionFlash;
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void drawInstructions(void)
{
    drawRect(0, 0, 100, 100, 0.035f, 0.045f, 0.08f);
    setColor(1.0f, 0.85f, 0.15f);
    drawText(34, 80, "CROSS ROAD GAME", GLUT_BITMAP_TIMES_ROMAN_24);

    setColor(1, 1, 1);
    drawText(29, 66, "THREE LEVEL CHALLENGE", GLUT_BITMAP_HELVETICA_18);
    drawText(20, 56, "Level 1: EASY     Level 2: MEDIUM     Level 3: HARD", GLUT_BITMAP_HELVETICA_18);
    drawText(20, 47, "You have 3 lives for the entire game.", GLUT_BITMAP_HELVETICA_18);
    drawText(20, 39, "A collision costs one life. Lose all 3 and it is GAME OVER.", GLUT_BITMAP_HELVETICA_12);
    drawText(20, 31, "Score: +10 for each forward step, +100 for each level.", GLUT_BITMAP_HELVETICA_12);
    drawText(20, 22, "Arrow keys: Move     R: Restart     Q / ESC: Quit", GLUT_BITMAP_HELVETICA_12);

    setColor(0.25f, 1.0f, 0.35f);
    drawText(34, 11, "PRESS ENTER OR SPACE TO START", GLUT_BITMAP_HELVETICA_18);
}

void drawLevelComplete(void)
{
    char buffer[80];

    drawRect(0, 0, 100, 100, 0.025f, 0.06f, 0.08f);
    setColor(0.25f, 1.0f, 0.35f);
    drawText(31, 70, "LEVEL COMPLETE!", GLUT_BITMAP_TIMES_ROMAN_24);

    setColor(1, 1, 1);
    sprintf(buffer, "You completed Level %d: %s", level, levelName[level - 1]);
    drawText(30, 57, buffer, GLUT_BITMAP_HELVETICA_18);
    sprintf(buffer, "Current score: %d", score);
    drawText(39, 47, buffer, GLUT_BITMAP_HELVETICA_18);
    sprintf(buffer, "Lives remaining: %d", lives);
    drawText(39, 39, buffer, GLUT_BITMAP_HELVETICA_18);

    setColor(1.0f, 0.85f, 0.15f);
    sprintf(buffer, "NEXT: LEVEL %d - %s", level + 1, levelName[level]);
    drawText(33, 27, buffer, GLUT_BITMAP_HELVETICA_18);

    setColor(0.25f, 1.0f, 0.35f);
    drawText(30, 14, "PRESS ENTER OR SPACE TO CONTINUE", GLUT_BITMAP_HELVETICA_18);
}

void drawGameOver(void)
{
    char buffer[60];

    drawRect(0, 0, 100, 100, 0.10f, 0.015f, 0.025f);
    setColor(1.0f, 0.20f, 0.20f);
    drawText(39, 68, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);

    setColor(1, 1, 1);
    drawText(29, 53, "You used all 3 lives.", GLUT_BITMAP_HELVETICA_18);
    sprintf(buffer, "Level reached: %d    Score: %d", level, score);
    drawText(28, 43, buffer, GLUT_BITMAP_HELVETICA_18);

    setColor(1.0f, 0.85f, 0.15f);
    drawText(31, 27, "Press R to try again.", GLUT_BITMAP_HELVETICA_18);
    setColor(1, 1, 1);
    drawText(34, 17, "Press Q or ESC to quit.", GLUT_BITMAP_HELVETICA_12);
}

void drawGameWon(void)
{
    char buffer[60];

    drawRect(0, 0, 100, 100, 0.02f, 0.08f, 0.045f);
    setColor(0.25f, 1.0f, 0.35f);
    drawText(31, 70, "YOU WON THE GAME!", GLUT_BITMAP_TIMES_ROMAN_24);

    setColor(1, 1, 1);
    drawText(30, 55, "Congratulations! You completed all 3 levels.", GLUT_BITMAP_HELVETICA_18);
    sprintf(buffer, "FINAL SCORE: %d", score);
    drawText(39, 43, buffer, GLUT_BITMAP_HELVETICA_18);
    sprintf(buffer, "Lives remaining: %d", lives);
    drawText(39, 34, buffer, GLUT_BITMAP_HELVETICA_18);

    setColor(1.0f, 0.85f, 0.15f);
    drawText(31, 19, "Press R to play again.", GLUT_BITMAP_HELVETICA_18);
}

void display(void)
{
    char buffer[80];

    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    switch (gameState) {
        case SCREEN_INSTRUCTIONS:
            drawInstructions();
            break;

        case SCREEN_LEVEL_COMPLETE:
            drawLevelComplete();
            break;

        case SCREEN_GAME_OVER:
            drawGameOver();
            break;

        case SCREEN_GAME_WON:
            drawGameWon();
            break;

        case SCREEN_PLAYING:
            drawRoad();
            drawCars();
            drawPlayer();

            setColor(1, 1, 1);
            sprintf(buffer, "LEVEL %d: %s", level, levelName[level - 1]);
            drawText(2, 96, buffer, GLUT_BITMAP_HELVETICA_18);

            sprintf(buffer, "SCORE: %d", score);
            drawText(38, 96, buffer, GLUT_BITMAP_HELVETICA_18);

            drawLives();
            drawText(2, 8, "ARROWS: MOVE", GLUT_BITMAP_HELVETICA_12);
            drawText(73, 8, "R: RESTART  Q: QUIT", GLUT_BITMAP_HELVETICA_12);

            if (collisionFlash > 0) {
                setColor(1.0f, 0.25f, 0.20f);
                drawText(43, 11, "OUCH! -1 LIFE", GLUT_BITMAP_HELVETICA_12);
            }
            break;
    }

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    windowWidth = w;
    windowHeight = h;
    if (h == 0) h = 1;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 100.0, 0.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void specialKey(int key, int x, int y)
{
    float oldY;

    (void)x;
    (void)y;

    if (gameState != SCREEN_PLAYING)
        return;

    oldY = player.y;

    switch (key) {
        case GLUT_KEY_UP:
            player.y += 4.0f;
            break;
        case GLUT_KEY_DOWN:
            player.y -= 4.0f;
            break;
        case GLUT_KEY_LEFT:
            player.x -= 4.0f;
            break;
        case GLUT_KEY_RIGHT:
            player.x += 4.0f;
            break;
        default:
            return;
    }

    if (player.x < 3.0f) player.x = 3.0f;
    if (player.x > 97.0f) player.x = 97.0f;
    if (player.y < 6.0f) player.y = 6.0f;
    if (player.y > 93.0f) player.y = 93.0f;

    /* Award points only when the player actually moves forward. */
    if (player.y > oldY)
        score += 10;

    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
    (void)x;
    (void)y;

    if (key == 27 || key == 'q' || key == 'Q')
        exit(0);

    if (key == 'r' || key == 'R') {
        restartGame();
        return;
    }

    if ((key == 13 || key == ' ') &&
        gameState == SCREEN_INSTRUCTIONS) {
        restartGame();
        return;
    }

    if ((key == 13 || key == ' ') &&
        gameState == SCREEN_LEVEL_COMPLETE) {
        ++level;
        resetPlayer();
        collisionFlash = 0;
        initialiseCars();
        gameState = SCREEN_PLAYING;
        glutPostRedisplay();
    }
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_W, WINDOW_H);
    glutInitWindowPosition(100, 50);
    glutCreateWindow("CROSS ROAD GAME - C / OpenGL Mini Project");

    glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 100.0, 0.0, 100.0);

    initialiseCars();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialKey);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}
