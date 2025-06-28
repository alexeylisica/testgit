#include <stdio.h>

#define WIDTH 80           // Ширина игрового поля
#define HEIGHT 25          // Высота игрового поля
#define PADDLE_SIZE 3      // Размер ракетки (3 символа по вертикали)
#define MAX_SCORE 21       // Счёт для победы

// Позиция верхней точки левой ракетки по Y
int left_y = HEIGHT / 2 - PADDLE_SIZE / 2;
// Позиция верхней точки правой ракетки по Y
int right_y = HEIGHT / 2 - PADDLE_SIZE / 2;

// Координаты мяча
int ball_x = WIDTH / 2;
int ball_y = HEIGHT / 2;
// Направление движения мяча по X и Y (значения +1 или -1)
int ball_dx = -1;
int ball_dy = -1;

// Счёт игроков
int score_left = 0;
int score_right = 0;

// Чей сейчас ход: 0 — левый игрок, 1 — правый игрок
int current_player = 0;

// Очистка экрана с помощью ANSI escape-последовательностей
void clear_screen() {
    printf("\033[2J\033[H");
}

// Отрисовка игрового поля, границ, ракеток, мяча и информации об игре
void draw_field() {
    clear_screen();  // Сначала очистка экрана

    // Проходим по всем строкам (y) и столбцам (x) поля
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            // Верхняя и нижняя границы поля — символ '&'
            if (y == 0 || y == HEIGHT - 1) {
                putchar('&');
            }
            // Левая и правая границы поля — символ '&'
            else if (x == 0 || x == WIDTH - 1) {
                putchar('&');
            }
            // Вертикальная линия посередине поля — символ '|'
            else if (x == WIDTH / 2) {
                putchar('|');
            }
            // Левая ракетка — столбец 1, рисуем '|', если y внутри ракетки
            else if (x == 1 && y >= left_y && y < left_y + PADDLE_SIZE) {
                putchar('|');
            }
            // Правая ракетка — столбец WIDTH-2, рисуем '|', если y внутри ракетки
            else if (x == WIDTH - 2 && y >= right_y && y < right_y + PADDLE_SIZE) {
                putchar('|');
            }
            // Мяч — рисуем символ 'O' в его текущих координатах
            else if (x == ball_x && y == ball_y) {
                putchar('O');
            }
            // Пустое пространство
            else {
                putchar(' ');
            }
        }
        putchar('\n');  // Переход на новую строку после каждого ряда
    }

    // Вывод текущего счёта игроков
    printf("Score: Left %d : %d Right\n", score_left, score_right);

    // Информация о том, кто ходит сейчас, и подсказки по управлению
    if (current_player == 0) {
        printf("Left player's turn (A/Z to move paddle, SPACE to pass)\n");
    } else {
        printf("Right player's turn (K/M to move paddle, SPACE to pass)\n");
    }
    printf("Your move: ");
}

// Функция обработки хода текущего игрока:
// Принимает символ, введённый с клавиатуры.
// Если ход корректный (движение или пропуск), возвращает 1.
// Если введена неверная клавиша, возвращает 0 (ход не засчитывается).
int move_paddle(char c) {
    if (current_player == 0) {
        // Ход левого игрока — допустимы 'a', 'z' и ' ' (пробел)
        if (c == 'a' && left_y > 1) {  // Движение вверх, если не у границы
            left_y--;
            return 1;
        } else if (c == 'z' && left_y + PADDLE_SIZE < HEIGHT - 1) {  // Вниз
            left_y++;
            return 1;
        } else if (c == ' ') {  // Пропуск хода
            return 1;
        }
    } else {
        // Ход правого игрока — допустимы 'k', 'm' и ' '
        if (c == 'k' && right_y > 1) {  // Вверх
            right_y--;
            return 1;
        } else if (c == 'm' && right_y + PADDLE_SIZE < HEIGHT - 1) {  // Вниз
            right_y++;
            return 1;
        } else if (c == ' ') {  // Пропуск
            return 1;
        }
    }
    return 0;  // Неверный ввод — ход не засчитывается
}

// Функция обновляет положение мяча и проверяет столкновения
void move_ball() {
    // Сдвигаем мяч по текущему направлению
    ball_x += ball_dx;
    ball_y += ball_dy;

    // Отскок мяча от верхней и нижней границ поля
    if (ball_y <= 1 || ball_y >= HEIGHT - 2)
        ball_dy = -ball_dy;

    // Обработка попадания мяча в левую ракетку или гол слева
    if (ball_x == 2) {
        if (ball_y >= left_y && ball_y < left_y + PADDLE_SIZE) {
            ball_dx = -ball_dx;  // Отражение по горизонтали

            // Меняем вертикальное направление мяча в зависимости от части ракетки
            if (ball_y == left_y)            // Верхняя часть ракетки
                ball_dy = -1;                // Мяч летит вверх
            else if (ball_y == left_y + PADDLE_SIZE - 1)  // Нижняя часть ракетки
                ball_dy = 1;                 // Мяч летит вниз
            // Если мяч попал в среднюю часть — dy не меняется
        } else {
            // Если промах — увеличиваем счёт правого игрока
            score_right++;
            // Сбрасываем мяч в центр поля и задаём направление движения
            ball_x = WIDTH / 2;
            ball_y = HEIGHT / 2;
            ball_dx = 1;   // Мяч летит вправо
            ball_dy = -1;  // Стартовое вертикальное направление
        }
    }

    // Аналогичная обработка для правой ракетки и гола справа
    if (ball_x == WIDTH - 3) {
        if (ball_y >= right_y && ball_y < right_y + PADDLE_SIZE) {
            ball_dx = -ball_dx;

            if (ball_y == right_y)
                ball_dy = -1;
            else if (ball_y == right_y + PADDLE_SIZE - 1)
                ball_dy = 1;
        } else {
            score_left++;
            ball_x = WIDTH / 2;
            ball_y = HEIGHT / 2;
            ball_dx = -1;  // Мяч летит влево
            ball_dy = -1;
        }
    }
}

int main() {
    // Основной игровой цикл — продолжается, пока кто-то не набрал MAX_SCORE
    while (score_left < MAX_SCORE && score_right < MAX_SCORE) {
        draw_field();  // Рисуем игровое поле и интерфейс

        char input;
        scanf(" %c", &input);  // Считываем символ от игрока (пробел, буква)

        // Пытаемся сделать ход — если ход корректный, меняем очередь и двигаем мяч
        if (move_paddle(input)) {
            current_player = 1 - current_player;  // Переключаем очередь ходов
            move_ball();
        }
        // Если ввод некорректный — ход не засчитывается, ждем следующий ввод
    }

    draw_field();  // Финальная отрисовка после окончания игры

    // Объявляем победителя
    if (score_left >= MAX_SCORE)
        printf("Left player wins!\n");
    else
        printf("Right player wins!\n");

    return 0;
}
