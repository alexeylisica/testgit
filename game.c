#include <ncurses.h>  // Библиотека для работы с консольным интерфейсом
#include <stdlib.h>   // Для malloc, free
#include <stdio.h>    // Для работы с файлами и вводом/выводом

#define WIDTH 80     // Ширина игрового поля (кол-во колонок)
#define HEIGHT 25    // Высота игрового поля (кол-во строк)
#define INIT_SPEED 200000  // Начальная задержка в микросекундах (скорость игры)

// Функция выделяет память под двумерный массив (поле размером HEIGHT x WIDTH)
int **create_field() {
    // Выделяем память под массив указателей на строки
    int **field = malloc(HEIGHT * sizeof(int *));
    if (!field) return NULL;  // Если malloc не сработал, возвращаем NULL

    int success = 1;  // Флаг успеха
    for (int i = 0; i < HEIGHT && success; i++) {
        // Выделяем память под каждую строку шириной WIDTH
        field[i] = malloc(WIDTH * sizeof(int));
        if (!field[i]) success = 0;  // Если не удалось выделить, отмечаем ошибку
    }

    if (!success) {
        // Если была ошибка при выделении, освобождаем уже выделенную память
        for (int i = 0; i < HEIGHT; i++) free(field[i]);
        free(field);
        field = NULL;  // Возвращаем NULL, чтобы показать ошибку
    }
    return field;  // Возвращаем указатель на поле
}

// Функция освобождает память поля, чтобы не было утечек памяти
void free_field(int **field) {
    if (field) {
        for (int i = 0; i < HEIGHT; i++) free(field[i]);  // Освобождаем каждую строку
        free(field);  // Освобождаем массив указателей
    }
}

// Функция читает состояние поля из файла
// Ожидается, что в файле будут числа 0 или 1, разделённые пробелами
int read_field(int **field, FILE *f) {
    for (int i = 0; i < HEIGHT; i++) {         // Для каждой строки
        for (int j = 0; j < WIDTH; j++) {      // Для каждого столбца
            if (fscanf(f, "%d", &field[i][j]) != 1) { // Считываем число
                fprintf(stderr, "Error reading at [%d][%d]\n", i, j);
                return 0;  // Ошибка чтения — выходим с ошибкой
            }
        }
    }
    return 1;  // Успешно прочитали поле
}

// Функция считает, сколько живых соседей у клетки [x][y]
// Клетки соседствуют по горизонтали, вертикали и диагоналям (8 клеток вокруг)
int neighbors(int **f, int x, int y) {
    int count = 0;  // Счётчик живых соседей
    for (int dx = -1; dx <= 1; dx++) {   // Перебираем смещение по строкам (-1,0,1)
        for (int dy = -1; dy <= 1; dy++) { // Перебираем смещение по столбцам (-1,0,1)
            if (dx || dy) { // Пропускаем саму клетку (dx=0 и dy=0)
                // Используем "тороидальную" адресацию — если вышли за край, "обертываемся"
                int nx = (x + dx + HEIGHT) % HEIGHT;  // Новая строка (по модулю)
                int ny = (y + dy + WIDTH) % WIDTH;    // Новый столбец (по модулю)
                count += f[nx][ny];  // Добавляем значение соседа (0 или 1)
            }
        }
    }
    return count;  // Возвращаем количество живых соседей
}

// Функция вычисляет следующее поколение поля
// По правилам игры:  
// - живая клетка с 2 или 3 соседями остаётся живой  
// - мёртвая клетка с ровно 3 соседями становится живой  
// - все остальные клетки умирают или остаются мёртвыми
void next_gen(int **curr, int **next) {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            int n = neighbors(curr, i, j);  // Считаем соседей
            // Применяем правила жизни
            next[i][j] = (curr[i][j] && (n == 2 || n == 3)) || (!curr[i][j] && n == 3);
        }
    }
}

// Функция очищает поле, устанавливая все клетки в 0 (мертвые)
void clear_field(int **field) {
    for (int i = 0; i < HEIGHT; i++)
        for (int j = 0; j < WIDTH; j++)
            field[i][j] = 0;
}

// Функция рисует поле и информацию в консоли через ncurses
void draw(int **field, int speed, int ch) {
    clear();  // Очищаем экран
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            // Рисуем символ 'O' для живой клетки и '.' для мёртвой
            mvaddch(i, j, field[i][j] ? 'O' : '.');
        }
    }
    // Рисуем строку с информацией о скорости и управлении
    mvprintw(HEIGHT, 0,
             "Delay: %d us | Controls: A - slower, Z - faster, SPACE - exit", speed);
    // Рисуем информацию о последней нажатой клавише
    mvprintw(HEIGHT + 1, 0, "Last key: code = %3d, char = '%c'    ",
             ch, (ch >= 32 && ch <= 126 ? ch : ' '));
    refresh();  // Обновляем экран, чтобы отобразить изменения
}

// Задержка для контроля скорости игры
void delay(int microseconds) {
    // napms принимает задержку в миллисекундах, поэтому делим на 1000
    napms(microseconds / 1000);
}

int main(int argc, char *argv[]) {
    int **curr = NULL, **next = NULL;
    int result = 0;

    // Проверяем, что имя файла с паттерном передано в аргументах командной строки
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // Открываем файл для чтения
    FILE *f = fopen(argv[1], "r");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s\n", argv[1]);
        return 1;
    }

    // Создаём два поля для текущего и следующего поколения
    curr = create_field();
    next = create_field();

    if (!curr || !next) {  // Если не удалось выделить память
        fprintf(stderr, "Memory allocation error\n");
        result = 1;
    } else if (!read_field(curr, f)) {  // Читаем поле из файла
        fprintf(stderr, "Error reading field\n");
        result = 1;
    } else {
        int speed = INIT_SPEED;  // Начальная скорость
        int ch = ERR;            // Код нажатой клавиши, изначально ошибок нет

        // Инициализация ncurses
        initscr();       // Запуск ncurses
        cbreak();        // Режим, при котором ввод сразу доступен без ожидания Enter
        noecho();        // Отключаем вывод вводимых символов
        keypad(stdscr, TRUE); // Включаем поддержку специальных клавиш (стрелки и др.)
        nodelay(stdscr, TRUE); // getch() не блокирует, если нет нажатия - возвращает ERR
        curs_set(0);     // Скрываем курсор

        // Главный игровой цикл
        while (1) {
            draw(curr, speed, ch);  // Рисуем поле и информацию
            ch = getch();           // Считываем нажатую клавишу (если есть)

            if (ch != ERR) {        // Если клавиша нажата
                if (ch == ' ' || ch == 32) break;  // Пробел — выход из игры
                else if (ch == 'a' || ch == 'A') {   // 'A' — увеличить задержку (медленнее)
                    if (speed < 1000000) speed += 50000;
                } else if (ch == 'z' || ch == 'Z') { // 'Z' — уменьшить задержку (быстрее)
                    if (speed > 50000) speed -= 50000;
                }
            }

            delay(speed);           // Пауза между поколениями
            next_gen(curr, next);   // Вычисляем следующее поколение

            // Меняем указатели, чтобы curr стал следующим поколением
            int **tmp = curr;
            curr = next;
            next = tmp;

            clear_field(next);      // Обнуляем поле next для следующего шага
        }

        endwin();  // Завершаем работу с ncurses и возвращаем терминал в нормальный режим
    }

    fclose(f);         // Закрываем файл
    free_field(curr);  // Освобождаем память
    free_field(next);

    return result;     // Возвращаем 0 при успешном выполнении
}
