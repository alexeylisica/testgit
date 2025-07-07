#include <ncurses.h>  // Библиотека для работы с терминалом (отображение и обработка клавиш)
#include <stdio.h>  // Для стандартного ввода-вывода, freopen, fprintf, fgets, getchar
#include <stdlib.h>  // Для функций динамического выделения памяти  malloc, free

#define WIDTH 80   // Ширина игрового поля в клетках
#define HEIGHT 25  // Высота игрового поля в клетках
#define INIT_SPEED 200000  // Начальная задержка между кадрами (в микросекундах)

// Создаём двумерное поле размером HEIGHT x WIDTH, выделяем динамически память
int **create_field() {
    int **field = malloc(HEIGHT * sizeof(int *));  // Массив указателей на строки
    int success = 1;  // Флаг успешного выделения памяти

    if (field) {  // Проверяем, что выделение прошло успешно
        for (int i = 0; i < HEIGHT && success; i++) {  // Для каждой строки
            field[i] = malloc(WIDTH * sizeof(int));    // Выделяем память под строку
            if (!field[i]) success = 0;  // Если выделение не удалось — ставим ошибку
        }
    } else {
        success = 0;  // Ошибка выделения памяти под массив указателей
    }

    if (!success) {  // Если что-то пошло не так
        if (field) {  // Если хоть что-то выделилось — освобождаем всю память
            for (int i = 0; i < HEIGHT; i++) free(field[i]);  // Освобождаем строки
            free(field);  // Освобождаем массив указателей
        }
        field = NULL;  // Помечаем, что памяти нет
    }

    return field;  // Возвращаем указатель на поле или NULL при ошибке
}

// Освобождаем память поля
void free_field(int **field) {
    for (int i = 0; i < HEIGHT; i++) {  // Для каждой строки
        free(field[i]);                 // Освобождаем память строки
    }
    free(field);  // Освобождаем массив указателей
}

// Читаем игровое поле из stdin построчно, проверяем входные данные
int read_field(int **field) {
    int success = 1;  // Флаг успеха чтения
    char line[4096];  // Буфер для строки

    for (int i = 0; i < HEIGHT && success; i++) {           // Считываем ровно 25 строк
        if (!fgets(line, sizeof(line), stdin)) {            // Считываем строку из stdin
            fprintf(stderr, "Error reading line %d\n", i);  // Если не удалось — ошибка
            success = 0;
        } else {
            int count = 0;     // Счётчик чисел в строке
            char *ptr = line;  // Указатель на текущий символ в строке

            for (int j = 0; j < WIDTH; j++) {  // Читаем 80 чисел в строке
                int val, offset;  // val — прочитанное число, offset — сколько символов прочитано
                if (sscanf(ptr, "%d%n", &val, &offset) != 1) {  // Пытаемся прочитать число
                    fprintf(stderr, "Line %d: expected integer at position %d\n", i + 1, count + 1);
                    success = 0;  // Ошибка — число не прочитано
                    break;
                }
                if (val != 0 && val != 1) {  // Проверяем, что число либо 0, либо 1
                    fprintf(stderr, "Line %d: invalid value %d at position %d (only 0 or 1 allowed)\n", i + 1,
                            val, count + 1);
                    success = 0;  // Ошибка — неверное значение
                    break;
                }
                field[i][j] = val;  // Записываем значение в поле
                ptr += offset;  // Смещаем указатель на количество прочитанных символов
                count++;  // Увеличиваем счётчик прочитанных чисел
            }

            // Проверяем, нет ли лишних символов после 80 чисел
            while (*ptr != '\0') {                                  // Пока не конец строки
                if (*ptr != ' ' && *ptr != '\n' && *ptr != '\t') {  // Если не пробельный символ — ошибка
                    fprintf(stderr, "Line %d: extra characters after 80 numbers\n", i + 1);
                    success = 0;
                    break;
                }
                ptr++;  // Идём дальше по строке
            }
        }
    }

    // Проверяем, что после 25 строк нет других символов кроме пробелов и переводов строки
    int c = getchar();                             // Читаем следующий символ
    while (c != EOF) {                             // Пока не конец файла
        if (c != ' ' && c != '\n' && c != '\t') {  // Если не пробельный символ — ошибка
            fprintf(stderr, "Input has extra lines beyond 25 rows\n");
            success = 0;
            break;
        }
        c = getchar();  // Следующий символ
    }

    return success;  // Возвращаем 1 если успешно, иначе 0
}

// Подсчёт количества живых соседей у клетки с координатами (x, y)
// Учтено замыкание поля по горизонтали и вертикали (тор)
int neighbors(int **f, int x, int y) {
    int count = 0;  // Кол-во живых соседей

    for (int dx = -1; dx <= 1; dx++) {      // Проходим по смещению по вертикали
        for (int dy = -1; dy <= 1; dy++) {  // Проходим по смещению по горизонтали
            if (dx || dy) {  // Пропускаем центральную клетку (dx=0, dy=0)
                int nx = (x + dx + HEIGHT) % HEIGHT;  // Координата соседа по вертикали с учётом замыкания
                int ny = (y + dy + WIDTH) % WIDTH;  // Координата соседа по горизонтали с учётом замыкания
                count += f[nx][ny];  // Прибавляем 1 если сосед жив
            }
        }
    }

    return count;  // Возвращаем число живых соседей
}

// Вычисление следующего поколения клеток по правилам "Жизни"
void next_gen(int **curr, int **next) {
    for (int i = 0; i < HEIGHT; i++) {      // Для каждой строки
        for (int j = 0; j < WIDTH; j++) {   // Для каждого столбца
            int n = neighbors(curr, i, j);  // Считаем соседей у клетки (i,j)
            // Клетка живёт, если у неё 2 или 3 соседа, или оживает, если у мёртвой 3 соседа
            next[i][j] = (curr[i][j] && (n == 2 || n == 3)) || (!curr[i][j] && n == 3);
        }
    }
}

// Очистка поля — делаем все клетки мёртвыми (0)
void clear_field(int **f) {
    for (int i = 0; i < HEIGHT; i++) {     // Для каждой строки
        for (int j = 0; j < WIDTH; j++) {  // Для каждого столбца
            f[i][j] = 0;                   // Обнуляем клетку
        }
    }
}

// Отрисовка игрового поля и информационной панели
void draw(int **f, int speed, int ch) {
    clear();  // Очищаем экран терминала

    for (int i = 0; i < HEIGHT; i++) {           // Проходим по строкам
        for (int j = 0; j < WIDTH; j++) {        // Проходим по столбцам
            mvaddch(i, j, f[i][j] ? 'O' : '.');  // Рисуем 'O' если клетка жива, иначе '.'
        }
    }

    // Выводим снизу строку с текущей задержкой и подсказкой по управлению
    mvprintw(HEIGHT, 0, "Delay: %d us | Controls: A - slower, Z - faster, SPACE - exit", speed);

    // Выводим код и символ последней нажатой клавиши (если это печатный символ)
    mvprintw(HEIGHT + 1, 0, "Last key: code = %3d, char = '%c'", ch, (ch >= 32 && ch <= 126) ? ch : ' ');

    refresh();  // Обновляем экран, чтобы все изменения стали видны
}

// Задержка между кадрами в микросекундах (использует napms, работающий с миллисекундами)
void delay(int microseconds) {
    napms(microseconds / 1000);  // napms принимает миллисекунды, делим микросекунды на 1000
}

// Главная функция программы
int main(int argc, const char *argv[]) {
    int result = 0;     // Код результата, 0 — успех, иначе ошибка
    int **curr = NULL;  // Текущее поколение игрового поля
    int **next = NULL;  // Следующее поколение игрового поля

    if (argc != 2) {  // Проверяем, что передан один аргумент — имя файла с начальными данными
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);  // Если нет — выводим подсказку
        result = 1;                              // Устанавливаем код ошибки
    } else if (!freopen(argv[1], "r", stdin)) {  // Перенаправляем stdin на файл с входными данными
        fprintf(stderr, "Cannot open file: %s\n", argv[1]);  // Если открыть не удалось — ошибка
        result = 1;
    } else {
        curr = create_field();  // Выделяем память под текущее поколение
        next = create_field();  // Выделяем память под следующее поколение

        if (!curr || !next) {                              // Если память не выделилась
            fprintf(stderr, "Memory allocation error\n");  // Выводим ошибку
            result = 1;
        } else if (!read_field(curr)) {  // Считываем начальное состояние поля
            fprintf(stderr, "Error reading field\n");  // Ошибка при чтении
            result = 1;
        } else {
            // Возвращаем stdin обратно на терминал для обработки клавиатуры
            if (!freopen("/dev/tty", "r", stdin)) {
                fprintf(stderr, "Error: cannot reopen /dev/tty for stdin\n");
                free_field(curr);  // Освобождаем память перед выходом
                free_field(next);
                return 1;
            }

            initscr();  // Инициализируем ncurses
            cbreak();  // Отключаем буферизацию ввода (символы принимаются сразу)
            noecho();  // Не показываем вводимые символы на экране
            keypad(stdscr, TRUE);  // Включаем поддержку клавиш стрелок и функциональных клавиш
            nodelay(stdscr, TRUE);  // Делает getch() неблокирующим — не ждёт ввода
            curs_set(0);            // Скрываем курсор

            int speed = INIT_SPEED;  // Начальная скорость (задержка)
            int ch = ERR;            // Код последней нажатой клавиши
            int stop = 0;            // Флаг для выхода из игрового цикла

            while (!stop) {             // Игровой цикл
                draw(curr, speed, ch);  // Отрисовываем поле и информацию
                ch = getch();           // Считываем клавишу (если нажата)

                if (ch != ERR) {      // Если клавиша была нажата
                    if (ch == ' ') {  // Если пробел — выходим из игры
                        stop = 1;
                    } else if (ch == 'a' || ch == 'A') {  // A — увеличить задержку (медленнее)
                        if (speed < 1000000) speed += 50000;
                    } else if (ch == 'z' || ch == 'Z') {  // Z — уменьшить задержку (быстрее)
                        if (speed > 50000) speed -= 50000;
                    }
                }

                delay(speed);          // Ждём заданное время между кадрами
                next_gen(curr, next);  // Вычисляем следующее поколение

                int **tmp = curr;  // Меняем указатели местами для переключения поколений
                curr = next;
                next = tmp;

                clear_field(next);  // Очищаем поле для следующего поколения
            }

            endwin();  // Завершаем работу с ncurses (восстанавливаем терминал)
        }
    }

    if (curr) free_field(curr);  // Освобождаем память текущего поколения
    if (next) free_field(next);  // Освобождаем память следующего поколения

    return result;  // Возвращаем код результата
}
