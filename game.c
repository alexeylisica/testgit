#include <ncurses.h>    // Эта библиотека позволяет рисовать в терминале: писать, двигать курсор, ловить нажатия клавиш
#include <stdlib.h>     // Здесь функции для работы с памятью, например malloc() и free()
#include <stdio.h>      // Это для вывода ошибок и работы с файлами

// Размер игрового поля: 80 клеток в ширину и 25 в высоту
#define WIDTH 80
#define HEIGHT 25

// Стартовая скорость (в микросекундах): чем больше число — тем медленнее
#define INIT_SPEED 200000

// Функция создаёт пустое поле (двумерный массив)
int **create_field() {
    int **field = malloc(HEIGHT * sizeof(int *)); // Выделяем память для строк (по количеству HEIGHT)
    if (!field) return NULL; // Если память не удалось выделить — возвращаем NULL

    int success = 1; // Переменная для проверки, всё ли прошло успешно

    for (int i = 0; i < HEIGHT && success; i++) { // Для каждой строки
        field[i] = malloc(WIDTH * sizeof(int));   // Выделяем память для столбцов
        if (!field[i]) success = 0;               // Если не удалось — запоминаем неудачу
    }

    // Если произошла ошибка — очищаем уже выделенную память
    if (!success) {
        for (int i = 0; i < HEIGHT; i++) free(field[i]);
        free(field);
        field = NULL; // Указываем, что поле недействительно
    }

    return field; // Возвращаем указатель на поле
}

// Функция освобождает память, которую мы выделяли для поля
void free_field(int **field) {
    if (field) { // Если поле существует
        for (int i = 0; i < HEIGHT; i++) free(field[i]); // Освобождаем каждую строку
        free(field); // И сам массив указателей
    }
}

// Функция читает игровое поле из файла
int read_field(int **field, FILE *f) {
    for (int i = 0; i < HEIGHT; i++) { // Проходим по всем строкам
        for (int j = 0; j < WIDTH; j++) { // Проходим по всем столбцам
            if (fscanf(f, "%d", &field[i][j]) != 1) { // Читаем число из файла
                fprintf(stderr, "Error reading at [%d][%d]\n", i, j); // Если ошибка — сообщаем координаты
                return 0; // Возвращаем 0 — ошибка
            }

            // Проверяем, что в ячейке либо 0, либо 1
            if (field[i][j] != 0 && field[i][j] != 1) {
                fprintf(stderr, "Invalid value at [%d][%d]: %d (must be 0 or 1)\n", i, j, field[i][j]);
                return 0;
            }
        }
    }
    return 1; // Всё считано успешно
}

// Функция считает количество живых соседей вокруг клетки
int neighbors(int **f, int x, int y) {
    int count = 0; // Сколько соседей живы
    for (int dx = -1; dx <= 1; dx++) { // Смещения по строкам
        for (int dy = -1; dy <= 1; dy++) { // Смещения по столбцам
            if (dx || dy) { // Пропускаем саму клетку
                // Находим координаты соседа (с учётом перехода по краям — "тор")
                int nx = (x + dx + HEIGHT) % HEIGHT;
                int ny = (y + dy + WIDTH) % WIDTH;
                count += f[nx][ny]; // Если сосед жив — добавим 1
            }
        }
    }
    return count; // Возвращаем количество живых соседей
}

// Функция создаёт новое поколение клеток на основе текущего
void next_gen(int **curr, int **next) {
    for (int i = 0; i < HEIGHT; i++) { // Проходим по всем строкам
        for (int j = 0; j < WIDTH; j++) { // И по всем столбцам
            int n = neighbors(curr, i, j); // Считаем соседей у клетки
            // Правила жизни: если живая и 2 или 3 соседа — живёт
            // Если мёртвая и ровно 3 соседа — оживает
            next[i][j] = (curr[i][j] && (n == 2 || n == 3)) || (!curr[i][j] && n == 3);
        }
    }
}

// Функция очищает поле (заполняет его нулями)
void clear_field(int **field) {
    for (int i = 0; i < HEIGHT; i++)
        for (int j = 0; j < WIDTH; j++)
            field[i][j] = 0;
}

// Функция рисует поле и показывает задержку и последнюю нажатую клавишу
void draw(int **field, int speed, int ch) {
    clear(); // Очищаем экран
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            // Если клетка живая — рисуем 'O', иначе — точку '.'
            mvaddch(i, j, field[i][j] ? 'O' : '.');
        }
    }
    // Внизу экрана выводим информацию
    mvprintw(HEIGHT, 0, "Delay: %d us | Controls: A - slower, Z - faster, SPACE - exit", speed);
    mvprintw(HEIGHT + 1, 0, "Last key: code = %3d, char = '%c'    ",
             ch, (ch >= 32 && ch <= 126 ? ch : ' ')); // Показываем код клавиши
    refresh(); // Обновляем экран
}

// Функция делает задержку (пауза между кадрами)
void delay(int microseconds) {
    napms(microseconds / 1000); // Переводим в миллисекунды и ждём
}

// Главная функция программы — всё начинается отсюда
int main(int argc, char *argv[]) {
    int **curr = NULL, **next = NULL; // Указатели на текущую и следующую матрицы
    int result = 0; // Итоговый код возврата (0 — всё хорошо)

    if (argc != 2) { // Если аргументов не два (включая имя программы)
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]); // Подсказка, как запускать
        return 1; // Завершаем с ошибкой
    }

    FILE *f = fopen(argv[1], "r"); // Открываем файл для чтения
    if (!f) { // Если не получилось открыть
        fprintf(stderr, "Cannot open file: %s\n", argv[1]); // Сообщаем об этом
        return 1; // Ошибка
    }

    // Создаём поля
    curr = create_field();
    next = create_field();
    if (!curr || !next) { // Если не получилось выделить память
        fprintf(stderr, "Memory allocation error\n");
        result = 1;
    } else if (!read_field(curr, f)) { // Если не получилось прочитать карту
        fprintf(stderr, "Error reading field\n");
        result = 1;
    } else {
        int speed = INIT_SPEED; // Задержка по умолчанию
        int ch = ERR; // Сюда будем сохранять код клавиши

        // Инициализация ncurses
        initscr();              // Подготовка экрана
        cbreak();               // Клавиши сразу передаются программе
        noecho();               // Не выводить нажатые символы
        keypad(stdscr, TRUE);   // Разрешить специальные клавиши (стрелки и т.д.)
        nodelay(stdscr, TRUE);  // getch() не ждёт ввода
        curs_set(0);            // Убрать мигающий курсор

        while (1) { // Главный игровой цикл
            draw(curr, speed, ch); // Рисуем поле
            ch = getch(); // Читаем нажатие клавиши

            if (ch != ERR) { // Если клавиша была нажата
                if (ch == ' ') break; // Если пробел — выходим
                else if (ch == 'a' || ch == 'A') {
                    if (speed < 1000000) speed += 50000; // Медленнее
                } else if (ch == 'z' || ch == 'Z') {
                    if (speed > 50000) speed -= 50000; // Быстрее
                }
            }

            delay(speed); // Пауза между кадрами
            next_gen(curr, next); // Создаём следующее поколение

            // Меняем местами curr и next
            int **tmp = curr;
            curr = next;
            next = tmp;

            clear_field(next); // Очищаем поле next, чтобы не было мусора
        }

        endwin(); // Завершаем работу ncurses
    }

    fclose(f); // Закрываем файл
    free_field(curr); // Освобождаем память
    free_field(next);
    return result; // Возвращаем код завершения (0 — успех)
}
