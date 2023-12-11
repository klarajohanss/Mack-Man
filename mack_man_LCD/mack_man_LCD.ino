/*
   File: mack_man_LCD.ino
   Author: Klara Johansson
   Date: 2023-11-27
   Description: This program implements a simple game with a joystick-controlled icon, the "Mack Man",
   a randomly moving ghost, and tomatoes to collect. The player's goal is to avoid collisions with
   the ghost while collecting tomatoes. The game ends when a collision occurs, displaying the score
   and potentially a new high score.
*/

//Include libraries
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"

//Define digital and analog pins
#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8
#define VRX_PIN A1
#define VRY_PIN A0

//Define the color "MY_LIGHTBROWN"
#define MY_LIGHTBROWN 0xEFCCA2 

Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

// Variables for the joystick, Mack Man, ghost, tomato and score
int xValue = 0;
int yValue = 0;

int iconX = 140;
int iconY = 180;
int iconWidth = 60;
int iconHeight = 30;
int rectangleWidth = 40;
int rectangleHeight = 20;

int ghostX = 50;
int ghostY = 50;
int ghostWidth = 40;
int ghostHeight = 55;
unsigned long lastRectangleTime = 0;
unsigned long rectangleInterval = 3000;

int tomatoSize = 10;
int tomatoX = 0;
int tomatoY = 0;

int scoreCounter = 0;
int highestScore = 0;

void setup() {
  Serial.begin(9600);
  tft.begin();

  uint8_t x = tft.readcommand8(HX8357_RDPOWMODE);

  tft.fillScreen(HX8357_BLACK);
  delay(600);
  tft.setRotation(0);
  tft.fillScreen(HX8357_CYAN);
  delay(800);

  //The title screen is displayed
  for (uint8_t rotation = 3; rotation < 4; rotation++) {
    tft.setRotation(rotation);
    introText(); 
    delay(3000);
  }
  tft.fillScreen(HX8357_BLACK);
}

void loop() {
  //Checking the x- and y-values of the joystick and updating Mack Man's position accordingly
  UpdateJoystick(); 
  int deltaX = map(xValue, 0, 1023, -5, 5);
  int deltaY = map(yValue, 0, 1023, -5, 5);
  iconX = constrain(iconX + deltaX, 0, tft.width() - iconWidth);
  iconY = constrain(iconY + deltaY, 0, tft.height() - iconHeight);
  tft.startWrite();
  drawSandwich(iconX, iconY, iconWidth, iconHeight, rectangleWidth, rectangleHeight);
  tft.endWrite();
  unsigned long currentMillis = millis();
  
  // Drawing ghosts on random positions at intervals of 3000 milliseconds
  if (currentMillis - lastRectangleTime >= rectangleInterval) {
    updateGhostPosition();
    tft.startWrite();
    drawGhost(ghostX, ghostY, ghostWidth, ghostHeight);
    tft.endWrite();
    lastRectangleTime = currentMillis;
  }

  //Checking for collision between Mack Man and a ghost
  checkCollision(iconX, iconY, iconWidth, iconHeight, ghostX, ghostY, ghostWidth, ghostHeight);

  // Checking if a tomato is displayed, and if not, drawing a tomato randomly
  if (tomatoX == 0 && tomatoY == 0) {
    tft.startWrite();
    drawTomatoes();
    tft.endWrite();
  }

  // Checking for collision between Mack man and tomato, and adding 100 to the score counter at each collision
  if (checkTomatoCollision(iconX, iconY, iconWidth, iconHeight, tomatoX, tomatoY, tomatoSize, tomatoSize)) {
    drawTomatoes();
    scoreCounter += 100;
  }
}

/* 
   Draws an ellipse using pixel-by-pixel approach to use for icons.
   Parameters: 
   - int x: X-coordinate of the center of the ellipse.
   - int y: Y-coordinate of the center of the ellipse.
   - int width: Width of the ellipse.
   - int height: Height of the ellipse.
   - uint16_t color: Color of the ellipse.
   Returns: void
*/
void drawEllipse(int x, int y, int width, int height, uint16_t color) {
  for (int i = 0; i < 360; i++) {
    float radian = i * 0.0174533;
    int xPos = x + cos(radian) * (width / 2);
    int yPos = y + sin(radian) * (height / 2);
    tft.drawPixel(xPos, yPos, color);
  }
}

/* 
   Draws a red circle, tomato, at a random position on the screen.
   Parameters: none
   Returns: void
   */
void drawTomatoes() {
  tomatoX = random(tft.width() - tomatoSize);
  tomatoY = random(tft.height() - tomatoSize);
  tft.fillCircle(tomatoX, tomatoY, 7, HX8357_RED);
}

/* 
   Checks if there is a collision between Mack Man and the tomato.
   Parameters:
   - int x1: X-coordinate of the top-left corner of Mack Man.
   - int y1: Y-coordinate of the top-left corner of Mack Man.
   - int w1: Width of Mack Man.
   - int h1: Height of Mack Man.
   - int x2: X-coordinate of the top-left corner of the tomato.
   - int y2: Y-coordinate of the top-left corner of the tomato.
   - int w2: Width of the tomato.
   - int h2: Height of the tomato.
   Returns: true or false
*/
bool checkTomatoCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
  return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

/* Draws a ghost on the screen with eyes, pupils, and mouth.
   Parameters:
   int x: X-coordinate of the top-left corner of the ghost.
   int y: Y-coordinate of the top-left corner of the ghost.
   int width: Width of the ghost.
   int height: Height of the ghost. 
   Returns: void
*/
void drawGhost(int x, int y, int width, int height) {
  drawEllipse(x, y, width, height, HX8357_CYAN);
  int eyeRadius = 4;
  int eyeOffsetX = width / 4;
  int eyeOffsetY = height / 4;
  tft.fillCircle(x - eyeOffsetX, y - eyeOffsetY, eyeRadius, HX8357_WHITE);
  tft.fillCircle(x + eyeOffsetX, y - eyeOffsetY, eyeRadius, HX8357_WHITE);
  int pupilRadius = 2;
  tft.fillCircle(x - eyeOffsetX, y - eyeOffsetY, pupilRadius, HX8357_BLACK);
  tft.fillCircle(x + eyeOffsetX, y - eyeOffsetY, pupilRadius, HX8357_BLACK);
  int mouthWidth = 10;
  int mouthHeight = 15;
  int mouthOffsetY = 5;
  drawEllipse(x, y + mouthOffsetY, mouthWidth, mouthHeight, HX8357_WHITE);
}

/* Updates the position of the ghost randomly within the screen boundaries.
   Parameters: none
   Returns: void
*/
void updateGhostPosition() {
  ghostX = random(tft.width() - ghostWidth);
  ghostY = random(tft.height() - ghostHeight);
}

/* Draws the Mack Man with a black rectangle around it.
   Parameters: 
   int x: X-coordinate of the top-left corner of the black rectangle.
   int y: Y-coordinate of the top-left corner of the black rectangle.
   int iconWidth: Width of Mack Man.
   int iconHeight: Height of Mack Man.
   int rectangleWidth: Width of the yellow rectangle on top of the black one.
   int rectangleHeight: Height of the yellow rectangle.
   Returns: void
*/
void drawSandwich(int x, int y, int iconWidth, int iconHeight, int rectangleWidth, int rectangleHeight) {
  tft.fillRect(x - 10, y - 10, iconWidth + 20, iconHeight + 20, HX8357_BLACK);
  tft.fillRoundRect(x, y, iconWidth, iconHeight, 20, MY_LIGHTBROWN); 
  tft.fillRect(x + (iconWidth / 2) - (rectangleWidth / 2), y, rectangleWidth, rectangleHeight, HX8357_YELLOW);
}

/* Reads analog values from the joystick pins and updates xValue and yValue.
   Parameters: none
   Returns: void
*/
void UpdateJoystick() {
  xValue = analogRead(VRX_PIN);
  yValue = analogRead(VRY_PIN);
}

/* Displays the title screen with the name of the game, the authors name, and Mack Man
   Parameters: none
   Returns: the time taken to execute the specified actions in microseconds
*/
unsigned long introText() {
  tft.fillScreen(HX8357_BLUE);
  unsigned long start = micros();
  tft.setCursor(55, 55);
  tft.setTextSize(8);
  tft.setTextColor(HX8357_YELLOW);
  tft.println("MACK-MAN");
  tft.setCursor(80, 130);
  tft.setTextColor(HX8357_WHITE);
  tft.setTextSize(3);
  tft.println("By Klara Johansson");
  tft.fillRoundRect(iconX, iconY, iconWidth, iconHeight, 20, MY_LIGHTBROWN); // Draw sandwich icon
  tft.fillRect(iconX + (iconWidth / 2) - (rectangleWidth / 2), iconY, rectangleWidth, rectangleHeight, HX8357_YELLOW);
  return micros() - start;
}

/* Checks for collision between the sandwich and the ghost and displays the "game over" screen if there is a collision.
   Parameters:
   int sandwichX: X-coordinate of the top-left corner of Mack Man.
   int sandwichY: Y-coordinate of the top-left corner of the Mack Man.
   int sandwichWidth: Width of the Mack Man.
   int sandwichHeight: Height of the Mack Man.
   int ghostX: X-coordinate of the top-left corner of the ghost.
   int ghostY: Y-coordinate of the top-left corner of the ghost.
   int ghostWidth: Width of the ghost.
   int ghostHeight: Height of the ghost. 
   Returns: void
*/
void checkCollision(int sandwichX, int sandwichY, int sandwichWidth, int sandwichHeight,
  int ghostX, int ghostY, int ghostWidth, int ghostHeight) {
  int sandwichLeft = sandwichX;
  int sandwichRight = sandwichX + sandwichWidth;
  int sandwichTop = sandwichY;
  int sandwichBottom = sandwichY + sandwichHeight;
  int ghostLeft = ghostX - ghostWidth / 2;
  int ghostRight = ghostX + ghostWidth / 2;
  int ghostTop = ghostY - ghostHeight / 2;
  int ghostBottom = ghostY + ghostHeight / 2;

  if (sandwichRight > ghostLeft && sandwichLeft < ghostRight &&
      sandwichBottom > ghostTop && sandwichTop < ghostBottom) {
    // Collision detected, display game over screen
    if (highestScore < scoreCounter) {
      highestScore = scoreCounter;
    }
    tft.fillScreen(HX8357_BLUE);
    unsigned long start = micros();
    tft.setCursor(55, 70);
    tft.setTextSize(7);
    tft.setTextColor(HX8357_RED);
    tft.println("GAME OVER");
    tft.setTextColor(HX8357_YELLOW);
    tft.setCursor(55, 180);
    tft.setTextSize(3);
    if (scoreCounter == highestScore) {
      tft.print("Score:");
      tft.print(scoreCounter);
      tft.setTextSize(2);
      tft.println(" NEW HIGHSCORE!");
    } else {
      tft.print("Score:");
      tft.println(scoreCounter);
    } 
    delay(1000);
    tft.setCursor(120, 250);
    tft.setTextSize(2);
    tft.setTextColor(HX8357_RED);
    tft.println("Try again in 5 seconds...");
    delay(4000);
    tomatoX = 0;
    tomatoY = 0;
    scoreCounter = 0;
    tft.fillScreen(HX8357_BLACK);
  }
}
