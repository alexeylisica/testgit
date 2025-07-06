#include <ncurses.h>   // Подключаем библиотеку для работы с графикой в терминале
#include <stdlib.h>    // Для функций malloc, free, exit и других
#include <stdio.h>     // Для ввода/вывода (fopen, fscanf, fprintf и др.)

#define WIDTH 80       // Ширина игрового поля (80 столбцов)
#define HEIGHT 25      // Высота игрового поля (25 строк)
#define INIT_SPEED 200000 // Начальная задержка между поколениями в микросекундах

// Функция для создания (выделения памяти под) двумерное игровое поле
int **create_field() {
    int **field = malloc(HEIGHT * sizeof(int *)); // Выделяем память под массив указателей на строки
    if (!field) return NULL; // Если не удалось выделить — возвращаем NULL

    int success = 1; // Флаг успеха
    for (int i = 0; i < HEIGHT && success; i++) {
        field[i] = malloc(WIDTH * sizeof(int)); // Выделяем память под каждую строку
        if (!field[i]) success = 0; // Если не удалось — сбрасываем флаг
    }

    if (!success) { // Если хотя бы одна строка не выделилась
        for (int i = 0; i < HEIGHT; i++) free(field[i]); // Освобождаем уже выделенные строки
        free(field); // Освобождаем массив указателей
        field = NULL; // Устанавливаем указатель в NULL
    }

    return field; // Возвращаем указатель на поле
}

// Функция для освобождения памяти, выделенной под поле
void free_field(int **field) {
    if (field) { // Проверка, что поле не NULL
        for (int i = 0; i < HEIGHT; i++) free(field[i]); // Освобождаем каждую строку
        free(field); // Освобождаем массив указателей
    }
}

// Функция для чтения игрового поля из файла
int read_field(int **field, FILE *f) {
    for (int i = 0; i < HEIGHT; i++) { // Перебираем строки
        for (int j = 0; j < WIDTH; j++) { // Перебираем столбцы
            int value; // Переменная для хранения считанного значения
            if (fscanf(f, "%d", &value) != 1) { // Пытаемся считать значение
                fprintf(stderr, "Error reading at [%d][%d]\n", i, j); // Ошибка: недочтено значение
                return 0; // Возвращаем 0 — ошибка
            }
            if (value != 0 && value != 1) { // Дополнительная проверка: значение должно быть 0 или 1
                fprintf(stderr, "Invalid value at [%d][%d]: %d\n", i, j, value); // Сообщаем об ошибке
                return 0; // Ошибка
            }
            field[i][j] = value; // Сохраняем значение в поле
        }
    }
    return 1; // Успешное чтение
}

// Функция для подсчета живых соседей у клетки с координатами (x, y)
int neighbors(int **f, int x, int y) {
    int count = 0; // Счетчик живых соседей
    for (int dx = -1; dx <= 1; dx++) { // Сдвиг по строкам от -1 до 1
        for (int dy = -1; dy <= 1; dy++) { // Сдвиг по столбцам от -1 до 1
            if (dx || dy) { // Пропускаем саму клетку (dx == 0 && dy == 0)
                int nx = (x + dx + HEIGHT) % HEIGHT; // Координата соседа по вертикали с учетом тороидальности
                int ny = (y + dy + WIDTH) % WIDTH;   // Координата соседа по горизонтали с учетом тороидальности
                count += f[nx][ny]; // Добавляем значение соседа (0 или 1)
            }
        }
    }
    return count; // Возвращаем количество живых соседей
}

// Функция для генерации следующего поколения поля
void next_gen(int **curr, int **next) {
    for (int i = 0; i < HEIGHT; i++) { // Перебор строк
        for (int j = 0; j < WIDTH; j++) { // Перебор столбцов
            int n = neighbors(curr, i, j); // Считаем соседей
            next[i][j] = (curr[i][j] && (n == 2 || n == 3)) || (!curr[i][j] && n == 3); // Правила жизни
        }
    }
}

// Функция для очистки поля (все клетки становятся 0)
void clear_field(int **field) {
    for (int i = 0; i < HEIGHT; i++) // Перебор строк
        for (int j = 0; j < WIDTH; j++) // Перебор столбцов
            field[i][j] = 0; // Устанавливаем 0
}

// Функция для отрисовки игрового поля
void draw(int **field, int speed, int ch) {
    clear(); // Очистка экрана
    for (int i = 0; i < HEIGHT; i++) { // Перебор строк
        for (int j = 0; j < WIDTH; j++) { // Перебор столбцов
            mvaddch(i, j, field[i][j] ? 'O' : '.'); // Рисуем 'O' для живых и '.' для мертвых
        }
    }
    // Печатаем строку состояния внизу
    mvprintw(HEIGHT, 0, "Delay: %d us | Controls: A - slower, Z - faster, SPACE - exit", speed);
    // Печатаем код последней нажатой клавиши
    mvprintw(HEIGHT + 1, 0, "Last key: code = %3d, char = '%c'    ", ch, (ch >= 32 && ch <= 126 ? ch : ' '));
    refresh(); // Обновляем экран
}

// Простая функция задержки с использованием napms (миллисекунды)
void delay(int microseconds) {
    napms(microseconds / 1000); // Переводим микро в миллисекунды
}

// Главная функция
int main(int argc, char *argv[]) {
    int **curr = NULL, **next = NULL; // Указатели на текущее и следующее поколение
    int result = 0; // Код возврата (0 — успех, 1 — ошибка)

    if (argc != 2) { // Проверка: должен быть 1 аргумент — путь к файлу
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]); // Подсказка пользователю
        return 1; // Завершаем с ошибкой
    }

    FILE *f = fopen(argv[1], "r"); // Открываем файл для чтения
    if (!f) { // Проверка на успешность открытия
        fprintf(stderr, "Cannot open file: %s\n", argv[1]); // Ошибка
        return 1;
    }

    curr = create_field(); // Создаем текущее поле
    next = create_field(); // Создаем следующее поле
    if (!curr || !next) { // Проверка выделения памяти
        fprintf(stderr, "Memory allocation error\n"); // Сообщение об ошибке
        result = 1;
    } else if (!read_field(curr, f)) { // Чтение из файла
        fprintf(stderr, "Error reading field\n"); // Сообщение об ошибке
        result = 1;
    } else {
        int speed = INIT_SPEED; // Устанавливаем начальную скорость
        int ch = ERR;           // Код нажатой клавиши

        initscr();              // Инициализируем ncurses
        cbreak();               // Ввод символов без Enter
        noecho();               // Не показывать нажатые клавиши
        keypad(stdscr, TRUE);   // Включить поддержку стрелок и спец. клавиш
        nodelay(stdscr, TRUE);  // Не ждать нажатия (getch возвращает ERR)
        curs_set(0);            // Скрываем курсор

        while (1) { // Основной игровой цикл
            draw(curr, speed, ch); // Отрисовка поля
            ch = getch(); // Читаем клавишу

            if (ch != ERR) { // Если была нажата клавиша
                if (ch == ' ' || ch == 32) break; // Пробел — выход
                else if (ch == 'a' || ch == 'A') {
                    if (speed < 1000000) speed += 50000; // Увеличиваем задержку (медленнее)
                } else if (ch == 'z' || ch == 'Z') {
                    if (speed > 50000) speed -= 50000; // Уменьшаем задержку (быстрее)
                }
            }

            delay(speed); // Задержка между кадрами

            next_gen(curr, next); // Вычисляем следующее поколение

            int **tmp = curr; // Меняем местами указатели
            curr = next;
            next = tmp;

            clear_field(next); // Очищаем "будущее" поле
        }

        endwin(); // Завершаем работу с ncurses
    }

    fclose(f); // Закрываем файл
    free_field(curr); // Освобождаем поле
    free_field(next); // Освобождаем следующее поле

    return result; // Возвращаем результат (0 — успех, 1 — ошибка)
}
