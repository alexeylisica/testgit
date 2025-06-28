#include <stdio.h>

// Размеры игрового поля
#define WIDTH 80         // ширина поля
#define HEIGHT 25        // высота поля
#define PADDLE_SIZE 3    // размер ракетки (3 символа)
#define MAX_SCORE 21     // очки для победы

// Позиции ракеток по вертикали (ось Y)
int left_y = HEIGHT / 2 - PADDLE_SIZE / 2;   // левая ракетка, изначально посередине
int right_y = HEIGHT / 2 - PADDLE_SIZE / 2;  // правая ракетка, изначально посередине

// Позиция мяча и направление движения
int ball_x = WIDTH / 2;   // по горизонтали в центре
int ball_y = HEIGHT / 2;  // по вертикали в центре
int ball_dx = -1;         // направление по X (-1 — влево, 1 — вправо)
int ball_dy = -1;         // направление по Y (-1 — вверх, 1 — вниз)

// Счёт игроков
int score_left = 0;
int score_right = 0;

// Функция очистки экрана (использует ANSI-коды)
// Работает в терминалах, поддерживающих ANSI (Linux, Windows Terminal, Git Bash)
void clear_screen() {
    printf("\033[2J\033[H");  // Очистка экрана и установка курсора в начало
}

// Функция отрисовки игрового поля
void draw_field() {
    clear_screen();  // очищаем экран перед рисованием

    // Проходим по всем строкам (Y)
    for (int y = 0; y < HEIGHT; y++) {
        // Проходим по всем символам в строке (X)
        for (int x = 0; x < WIDTH; x++) {

            // Рисуем верхнюю и нижнюю границу из символа '&'
            if (y == 0 || y == HEIGHT - 1) {
                putchar('&');
            }
            // Рисуем левую и правую границу из символа '&'
            else if (x == 0 || x == WIDTH - 1) {
                putchar('&');
            }
            // Рисуем среднюю вертикальную линию посередине поля
            else if (x == WIDTH / 2) {
                putchar('|');
            }
            // Рисуем левую ракетку (вторая колонка, внутри границ)
            else if (x == 1 && y >= left_y && y < left_y + PADDLE_SIZE) {
                putchar('|');
            }
            // Рисуем правую ракетку (предпоследняя колонка, внутри границ)
            else if (x == WIDTH - 2 && y >= right_y && y < right_y + PADDLE_SIZE) {
                putchar('|');
            }
            // Рисуем мяч
            else if (x == ball_x && y == ball_y) {
                putchar('O');
            }
            // Всё остальное — пустое пространство
            else {
                putchar(' ');
            }
        }
        putchar('\n');  // переходим на новую строку после каждого ряда
    }

    // Выводим текущий счёт
    printf("Score: Left %d : %d Right\n", score_left, score_right);
    // Подсказка по управлению
    printf("Controls: A/Z (Left)  K/M (Right)  SPACE (pass)\n");
    printf("Your move: ");  // приглашение к вводу
}

// Функция перемещения ракеток в зависимости от нажатой клавиши
void move_paddles(char c) {
    // Для левой ракетки: 'a' — вверх, 'z' — вниз
    // Проверяем, чтобы ракетка не вышла за границу
    if (c == 'a' && left_y > 1)
        left_y--;
    if (c == 'z' && left_y + PADDLE_SIZE < HEIGHT - 1)
        left_y++;

    // Для правой ракетки: 'k' — вверх, 'm' — вниз
    if (c == 'k' && right_y > 1)
        right_y--;
    if (c == 'm' && right_y + PADDLE_SIZE < HEIGHT - 1)
        right_y++;
}

// Функция перемещения мяча и обработки столкновений
void move_ball() {
    // Сдвигаем мяч по текущему направлению
    ball_x += ball_dx;
    ball_y += ball_dy;

    // Отскок мяча от верхней и нижней стенки (границ)
    if (ball_y <= 1 || ball_y >= HEIGHT - 2)
        ball_dy = -ball_dy;

    // Обработка столкновения с левой ракеткой или голом
    if (ball_x == 2) {
        // Если мяч попал в ракетку — меняем направление по X (отскок)
        if (ball_y >= left_y && ball_y < left_y + PADDLE_SIZE)
            ball_dx = -ball_dx;
        else {
            // Иначе засчитываем гол правому игроку
            score_right++;
            // Возвращаем мяч в центр
            ball_x = WIDTH / 2;
            ball_y = HEIGHT / 2;
            ball_dx = 1;  // мяч движется вправо
        }
    }

    // Обработка столкновения с правой ракеткой или голом
    if (ball_x == WIDTH - 3) {
        // Отскок от ракетки
        if (ball_y >= right_y && ball_y < right_y + PADDLE_SIZE)
            ball_dx = -ball_dx;
        else {
            // Гол левому игроку
            score_left++;
            // Сброс мяча в центр
            ball_x = WIDTH / 2;
            ball_y = HEIGHT / 2;
            ball_dx = -1;  // мяч движется влево
        }
    }
}

int main() {
    // Основной цикл игры: продолжаем пока кто-то не наберёт MAX_SCORE
    while (score_left < MAX_SCORE && score_right < MAX_SCORE) {
        draw_field();  // рисуем текущее состояние поля

        char input;
        scanf(" %c", &input);  // читаем один символ с пробелами

        move_paddles(input);   // обновляем позицию ракеток
        move_ball();           // обновляем позицию мяча
    }

    // После завершения игры выводим итоговое поле
    draw_field();

    // Объявляем победителя
    if (score_left >= MAX_SCORE)
        printf("Left player wins!\n");
    else
        printf("Right player wins!\n");

    return 0;
}
