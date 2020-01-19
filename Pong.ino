#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define PADDLELENGTH 10

// Selection Time in selection screen
float selectionTime = (1/*seconds*/) * 60 /* frames*/;

// Possible gamenmodes
enum GameMode {
  PongAlone,
  PongVersus
  };
GameMode mode = PongAlone;
GameMode lastInput = PongAlone;

// Score-tracking
int maxScore = 5; // Score needed before game restarts
int p1Score = 0;
int p2Score = 0;

// Gamestart
bool gameStarted = false;

// Distance of paddles from edge
int paddlePadding = 10;

// Ball velocity
int ballvX = 2;
int ballvY = 1;

// Ball position
int ballX = 127/2;
int ballY = 63/2;

// Ball Size
int ballSize = 1;

// Paddle y-positions
int paddle01 = 0;
int paddle02 = 0;

// Did someone just score?
int justScored = -1;

void setup() {
  Serial.begin(9600);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();

  display.setTextColor(WHITE);
  /*someoneScored(0);
  display.display();
  delay(10000);*/

  display.setTextSize(2); // Smallest Size
  display.setTextColor(WHITE);
  display.setCursor((128/2)-23, (64/2)-6);
  display.println("PONG");
  display.setTextSize(1);
  display.setCursor(10, (64/2)+10);
  display.println("by Julien Schweter");
  display.display();

  //delay(6000);
}

void loop() {
  if (gameStarted) {
    display.clearDisplay();
    drawPaddles();
    drawBall();
    drawScore();
    display.display();
    if (justScored != -1) {
      tone(3, 220, 200);
      delay(1000);
      someoneScored(justScored);

      if (p1Score >= maxScore || p2Score >= maxScore) {
        drawWinner();
        p1Score = 0;
        p2Score = 0;
        }
      
      ballX = 128/2;
      ballY = 64/2;
      drawBeginGame();
      }
    }
  if (!gameStarted) {
    pongGamePrep();
    if (selectionTime <= 0) { gameStarted = true;  drawBeginGame();}
    } 
}


void pongGamePrep() {
  // Display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(128/2-27, 2);
  display.println("GAME MODE");
  display.setCursor(0,0);
  display.println(selectionTime/60);
  
  display.setCursor(20, 64/2);
  display.println("ALONE");

  display.setCursor(128/2+10, 64/2);
  display.println("VERSUS");

  if (getInput(0) > 0.5f) { mode = PongAlone; } else mode = PongVersus;
  if (mode == PongAlone) { display.drawLine(18, 64/2 + 10, 50, 64/2 + 10, SSD1306_WHITE); } 
    else display.drawLine(128/2+8, 64/2 + 10, 128/2+46, 64/2 + 10, SSD1306_WHITE);

  if (lastInput != mode) {tone(3, 800, 100); lastInput = mode;}
   
  selectionTime -= 1.0f; // Not accounting for delta time
  display.display();
  }

/* Gives you the input value of one of the potentiometers in the range of
 *  0 to 1.0 given the wanted player Index. 
 */
float getInput(int playerIndex) {
  float output = -1.0f;

  // Check if demanded index is valid
  if (playerIndex < 0 || playerIndex > 1) { return output;}

  // Calculates a normalized input of 0.0 to 1.0
  output = analogRead(playerIndex);
  output = float(output+1)/1024.0;

  return output;
  }

void drawBeginGame() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(128/2-13, 64/2);
  display.println("Ready?");
  display.display();
  delay(1000);
  for (int i = 0; i != 3; i++) {
    display.clearDisplay();
    display.setCursor(128/2-2,64/2);
    display.println(3-i);
    display.display();
    tone(3, 100, 100);
    delay(1000);
  }
  display.clearDisplay();
  display.setCursor(128/2-4,64/2);
  display.println("GO!");
  display.display();
  tone(3, 180, 100);
  delay(1000);
  
  
  }

void drawWinner() {
  int playerIndex;
  display.clearDisplay();
  display.setCursor(128/2-10, 10);
  display.setTextSize(2);

  if (p1Score > p2Score) {playerIndex = 0;} else playerIndex = 1;
  if (playerIndex == 0) { display.println("P1"); } else display.println("P2");
  display.setCursor(128/2-44, 64/2+5);
  display.println("HAS WON!");
  display.display();
  tone(3, 350, 1000);
  delay(7500);
  }

void someoneScored(int playerIndex) {
  display.clearDisplay();
  display.setCursor(128/2-10, 10);
  display.setTextSize(2);
  if (playerIndex == 0) { display.println("P1"); } else display.println("P2");
  display.setCursor(128/2-38, 64/2+5);
  display.println("SCORED!");
  display.display();
  tone(3, 250, 800);
  delay(3000);

  justScored = -1;
  }

void score(int playerIndex) {
  if (playerIndex == 0) { p1Score += 1; }
  if (playerIndex == 1) { p2Score += 1; }

  justScored = playerIndex;
  }

void drawScore() {
  display.setCursor(128/2-2, 0);
  display.setTextSize(1);
  display.print(p1Score);
  display.print(":");
  display.print(p2Score);
  }

void drawPaddles(void) {
  if (mode == PongVersus) {
    paddle01 = int(getInput(0)*64.0f);
    paddle02 = int(getInput(1)*64.0f);
    }
  if (mode == PongAlone) {
    paddle01 = int(getInput(0)*64.0f);
    paddle02 = paddle01;
    }
   
  display.drawLine(paddlePadding, paddle01, paddlePadding, paddle01 + PADDLELENGTH, SSD1306_WHITE);
  display.drawLine(128-paddlePadding, paddle02, 128-paddlePadding, paddle02 + PADDLELENGTH, SSD1306_WHITE);  
  }

void drawBall(void) {
  int newPositionX;
  int newPositionY;

  // When colliding with wall
  if (ballY >= 64 or ballY <= 0) {ballvY *= -1;}
  if (ballX >= 128) { score(0); }
  if (ballX <= 0) { score(1); }

  // Bounce on paddle 01 
  // First do check if ball is in x coordinate of paddle
  if (ballX <= paddlePadding + ballSize and ballvX < 0) {
    // Now see if ball is in y coordinate of paddle
    if (ballY > paddle01 - ballSize && ballY < paddle01 + PADDLELENGTH) {
      bounceSound();
      ballvX *= -1;
      }
    }

  // Bounce on paddle 02
  if (ballX >= 128 - 1- paddlePadding - ballSize && ballX <= 128 - paddlePadding - ballSize && ballvX > 0) {
    if (ballY > paddle02 - ballSize && ballY < paddle02 + PADDLELENGTH) {
      bounceSound();
      ballvX *= -1;
      }
    }
  
  newPositionX = ballX + ballvX;
  newPositionY = ballY + ballvY;
  
  ballX = newPositionX;
  ballY = newPositionY;

  display.drawPixel(ballX, ballY, SSD1306_WHITE);
  }

void bounceSound() {
  // Pin, Frequency, Length
  tone(3, 160, 80);
  }
