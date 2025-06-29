#include <stdio.h>

#define WIDTH 80
#define HEIGHT 25
#define PADDLE_SIZE 3
#define MAX_SCORE 21

// Положение верхней части левой ракетки
int leftY = HEIGHT / 2 - PADDLE_SIZE / 2;
// Положение верхней части правой ракетки
int rightY = HEIGHT / 2 - PADDLE_SIZE / 2;

// Координаты мяча
int ballX = WIDTH / 2;
int ballY = HEIGHT / 2;
// Направление движения мяча
int ballDx = -1;
int ballDy = -1;

// Счёт
int scoreLeft = 0;
int scoreRight = 0;

// 0 — левый игрок, 1 — правый
int currentPlayer = 0;

// Очистка экрана терминала
void ClearScreen() {
  printf("\033[2J\033[H");
}

// Отрисовка игрового поля, ракеток, мяча и интерфейса
void DrawField() {
  ClearScreen();

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      if (y == 0 || y == HEIGHT - 1) {
        putchar('&');  // верхняя и нижняя границы
      } else if (x == 0 || x == WIDTH - 1) {
        putchar('&');  // боковые границы
      } else if (x == WIDTH / 2) {
        putchar('|');  // центральная линия
      } else if (x == 1 && y >= leftY && y < leftY + PADDLE_SIZE) {
        putchar('|');  // левая ракетка
      } else if (x == WIDTH - 2 && y >= rightY && y < rightY + PADDLE_SIZE) {
        putchar('|');  // правая ракетка
      } else if (x == ballX && y == ballY) {
        putchar('O');  // мяч
      } else {
        putchar(' ');
      }
    }
    putchar('\n');
  }

  // Счёт и подсказка по управлению
  printf("Score: Left %d : %d Right\n", scoreLeft, scoreRight);
  if (currentPlayer == 0) {
    printf("Left player's turn (A/Z to move, SPACE to pass)\n");
  } else {
    printf("Right player's turn (K/M to move, SPACE to pass)\n");
  }
  printf("Your move: ");
}

// Обработка хода ракетки текущего игрока
int MovePaddle(char input) {
  if (currentPlayer == 0) {
    if (input == 'a' && leftY > 1) {
      leftY--;
      return 1;
    } else if (input == 'z' && leftY + PADDLE_SIZE < HEIGHT - 1) {
      leftY++;
      return 1;
    } else if (input == ' ') {
      return 1;
    }
  } else {
    if (input == 'k' && rightY > 1) {
      rightY--;
      return 1;
    } else if (input == 'm' && rightY + PADDLE_SIZE < HEIGHT - 1) {
      rightY++;
      return 1;
    } else if (input == ' ') {
      return 1;
    }
  }
  return 0;
}

// Обработка движения мяча и столкновений
void MoveBall() {
  ballX += ballDx;
  ballY += ballDy;

  // Отскок от верхней/нижней границы
  if (ballY <= 1 || ballY >= HEIGHT - 2) {
    ballDy = -ballDy;
  }

  // Левая сторона
  if (ballX == 2) {
    if (ballY >= leftY && ballY < leftY + PADDLE_SIZE) {
      ballDx = -ballDx;
      if (ballY == leftY) {
        ballDy = -1;
      } else if (ballY == leftY + PADDLE_SIZE - 1) {
        ballDy = 1;
      }
    } else {
      scoreRight++;
      ballX = WIDTH / 2;
      ballY = HEIGHT / 2;
      ballDx = 1;
      ballDy = -1;
    }
  }

  // Правая сторона
  if (ballX == WIDTH - 3) {
    if (ballY >= rightY && ballY < rightY + PADDLE_SIZE) {
      ballDx = -ballDx;
      if (ballY == rightY) {
        ballDy = -1;
      } else if (ballY == rightY + PADDLE_SIZE - 1) {
        ballDy = 1;
      }
    } else {
      scoreLeft++;
      ballX = WIDTH / 2;
      ballY = HEIGHT / 2;
      ballDx = -1;
      ballDy = -1;
    }
  }
}

// Основная функция игры
int main() {
  while (scoreLeft < MAX_SCORE && scoreRight < MAX_SCORE) {
    DrawField();

    char input;
    scanf(" %c", &input);

    if (MovePaddle(input)) {
      currentPlayer = 1 - currentPlayer;  // переключаем игрока
      MoveBall();  // двигаем мяч
    }
  }

  DrawField();

  if (scoreLeft >= MAX_SCORE) {
    printf("Left player wins!\n");
  } else {
    printf("Right player wins!\n");
  }

  return 0;
}
