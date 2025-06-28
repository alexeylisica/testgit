#include <stdio.h>

#define WIDTH 80           // Ширина игрового поля
#define HEIGHT 25          // Высота игрового поля
#define PADDLE_SIZE 3      // Размер ракетки (3 символа по вертикали)
#define MAX_SCORE 21       // Максимальный счет для победы

// Позиция верхней точки левой ракетки (по Y)
int left_y = HEIGHT / 2 - PADDLE_SIZE / 2;
// Позиция верхней точки правой ракетки (по Y)
int right_y = HEIGHT / 2 - PADDLE_SIZE / 2;

// Позиция мяча по X и Y
int ball_x = WIDTH / 2;
int ball_y = HEIGHT / 2;
// Направление движения мяча по X и Y (1 или -1)
int ball_dx = -1;
int ball_dy = -1;

// Счет игроков
int score_left = 0;
int score_right = 0;

// Функция очистки экрана терминала с помощью ANSI кодов
void clear_screen() {
    printf("\033[2J\033[H"); // Очистить экран и поставить курсор в левый верхний угол
}

// Функция отрисовки игрового поля, ракеток, мяча и интерфейса
void draw_field() {
    clear_screen();  // Очищаем экран перед рисованием

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {

            // Рисуем верхнюю и нижнюю границы символом &
            if (y == 0 || y == HEIGHT - 1) {
                putchar('&');  
            }
            // Рисуем левую и правую границы символом &
            else if (x == 0 || x == WIDTH - 1) {
                putchar('&');  
            }
            // Рисуем вертикальную среднюю линию по центру поля
            else if (x == WIDTH / 2) {
                putchar('|');
            }
            // Рисуем левую ракетку (в столбце 1), если текущий y в пределах ракетки
            else if (x == 1 && y >= left_y && y < left_y + PADDLE_SIZE) {
                putchar('|');
            }
            // Рисуем правую ракетку (в столбце WIDTH-2), если текущий y в пределах ракетки
            else if (x == WIDTH - 2 && y >= right_y && y < right_y + PADDLE_SIZE) {
                putchar('|');
            }
            // Рисуем мяч символом 'O' в его текущей позиции
            else if (x == ball_x && y == ball_y) {
                putchar('O');
            }
            // Во всех остальных местах рисуем пробел
            else {
                putchar(' ');
            }
        }
        putchar('\n');  // Переход на новую строку после каждой строки поля
    }

    // Выводим текущий счет игроков под полем
    printf("Score: Left %d : %d Right\n", score_left, score_right);
    // Выводим подсказки по управлению
    printf("Controls: A/Z (Left)  K/M (Right)  SPACE (pass)\n");
    printf("Your move: ");
}

// Функция для изменения положения ракеток в зависимости от нажатой клавиши
void move_paddles(char c) {
    // Перемещение левой ракетки вверх, если не достигли верхней границы
    if (c == 'a' && left_y > 1)         
        left_y--;
    // Перемещение левой ракетки вниз, если не достигли нижней границы
    if (c == 'z' && left_y + PADDLE_SIZE < HEIGHT - 1)
        left_y++;

    // Перемещение правой ракетки вверх, если не достигли верхней границы
    if (c == 'k' && right_y > 1)
        right_y--;
    // Перемещение правой ракетки вниз, если не достигли нижней границы
    if (c == 'm' && right_y + PADDLE_SIZE < HEIGHT - 1)
        right_y++;
}

// Функция обновления позиции мяча и обработки столкновений
void move_ball() {
    // Сдвигаем мяч по направлению dx и dy
    ball_x += ball_dx;
    ball_y += ball_dy;

    // Отскок мяча от верхней или нижней границы поля
    if (ball_y <= 1 || ball_y >= HEIGHT - 2)
        ball_dy = -ball_dy;

    // Обработка столкновения с левой ракеткой или гола слева
    if (ball_x == 2) {
        // Если мяч попал в ракетку (по Y координате)
        if (ball_y >= left_y && ball_y < left_y + PADDLE_SIZE) {
            ball_dx = -ball_dx;  // Отражаем по X

            // Меняем вертикальное направление мяча в зависимости от части ракетки
            if (ball_y == left_y)            // Верхняя часть ракетки
                ball_dy = -1;                // Мяч летит вверх
            else if (ball_y == left_y + PADDLE_SIZE - 1)  // Нижняя часть ракетки
                ball_dy = 1;                 // Мяч летит вниз
            // Средняя часть — dy не меняется
        }
        else {
            // Мяч промахнулся — очко правому игроку
            score_right++;
            // Сброс мяча в центр поля
            ball_x = WIDTH / 2;
            ball_y = HEIGHT / 2;
            ball_dx = 1;     // Мяч летит в сторону проигравшего игрока
            ball_dy = -1;    // Можно задать стартовое вертикальное направление мяча
        }
    }

    // Обработка столкновения с правой ракеткой или гола справа
    if (ball_x == WIDTH - 3) {
        if (ball_y >= right_y && ball_y < right_y + PADDLE_SIZE) {
            ball_dx = -ball_dx;  // Отражаем по X

            if (ball_y == right_y)             // Верхняя часть ракетки
                ball_dy = -1;
            else if (ball_y == right_y + PADDLE_SIZE - 1) // Нижняя часть ракетки
                ball_dy = 1;
            // Средняя часть — dy не меняется
        }
        else {
            // Очко левому игроку
            score_left++;
            // Сброс мяча в центр поля
            ball_x = WIDTH / 2;
            ball_y = HEIGHT / 2;
            ball_dx = -1;   // Мяч летит в сторону проигравшего
            ball_dy = -1;   // Стартовое вертикальное направление мяча
        }
    }
}

int main() {
    // Основной цикл игры — продолжаем, пока кто-то не наберет MAX_SCORE
    while (score_left < MAX_SCORE && score_right < MAX_SCORE) {
        draw_field();   // Рисуем игровое поле

        char input;
        scanf(" %c", &input);  // Считываем команду игрока (пробел, буквы и т.д.)

        move_paddles(input);    // Двигаем ракетки в зависимости от ввода
        move_ball();            // Обновляем позицию мяча и проверяем столкновения
    }

    draw_field();  // Показываем финальное поле с итоговым счетом

    // Выводим сообщение о победителе
    if (score_left >= MAX_SCORE)
        printf("Left player wins!\n");
    else
        printf("Right player wins!\n");

    return 0;
}
