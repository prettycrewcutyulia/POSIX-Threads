#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

pthread_mutex_t mutex;
pthread_cond_t cond;

struct cell {
    int x;
    int y;
    char value;
};

struct cell island[100][100];
struct cell inspect[10000];
int inspectOrder = 0;

struct cell treasures[10000];
int treasuresCounter = 0;

int islandSide = 5;

const int treasureChance = 30;

void generateIsland(int size) {
    islandSide = size;
    for (int i = 0; i < islandSide; i++) {
        for (int j = 0; j < islandSide; j++) {
            island[i][j].x = i;
            island[i][j].y = j;
            island[i][j].value = '.';
            if (rand() % 100 < treasureChance) {
                island[i][j].value = 'X';
            }
        }
    }
}

void printIsland() {
    for (int i = 0; i < islandSide; i++) {
        for (int j = 0; j < islandSide; j++) {
            printf("%c ", island[i][j].value);
        }
        printf("\n");
    }
}

void *pirateTeamTask(void *arg) {
    int teamNumber = (int)arg;
    printf("Команда пиратов %d исследует остров...\n", teamNumber);
    while (true) {
        pthread_mutex_lock(&mutex);
        if (inspectOrder >= islandSide * islandSide) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        struct cell cell = inspect[inspectOrder];
        inspectOrder++;
        pthread_mutex_unlock(&mutex);
        printf("Команда пиратов %d исследует клетку [%d %d] (%c) в течение 1 секунды\n", teamNumber,
               cell.x, cell.y, cell.value);
        sleep(1);
        if (cell.value == 'X') {
            pthread_mutex_lock(&mutex);
            treasures[treasuresCounter] = cell;
            treasuresCounter++;
            pthread_mutex_unlock(&mutex);
        }
    }
    printf("Команда пиратов %d закончила исследование острова.\n", teamNumber);
    return NULL;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    int size;
    if (argc > 1) {
        size = atoi(argv[1]);
    } else {
        printf("Не задан размер острова в аргументе запуска \n");
        printf("Введите размер острова (от 1 до 10): ");
        scanf("%d", &size);
    }
    if (size < 1 || size > 10) {
        printf("Неверный размер острова, установлено значение по умолчанию 5\n");
        size = 5;
        islandSide = size;
    }

    printf("Создаём остров размером %d на %d...\n", size, size);
    generateIsland(size);
    printIsland();

    int teams;
    if (argc > 2) {
        teams = atoi(argv[2]);
    } else {
        printf("Не задано количество команд пиратов в аргументе запуска \n");
        printf("Введите количество команд пиратов: ");
        scanf("%d", &teams);
    }
    if (teams < 1 || teams > 100) {
        printf("Неверное количество команд, установлено значение по умолчанию 5\n");
        teams = 5;
    }

    for (int i = 0; i < islandSide; i++) {
        for (int j = 0; j < islandSide; j++) {
            inspect[i * islandSide + j] = island[i][j];
        }
    }

    printf("Команды пиратов отправляются исследовать остров.\n");
    pthread_t pirateTeams[teams];
    for (int i = 0; i < teams; i++) {
        pthread_create(&pirateTeams[i], NULL, pirateTeamTask, (void *)i);
    }

    for (int i = 0; i < teams; i++) {
        pthread_join(pirateTeams[i], NULL);
    }

    printf("Сокровищ найдено: %d шт.\n", treasuresCounter);
    for (int i = 0; i < treasuresCounter; i++) {
        printf("Сокровище %d: (%d, %d)\n", i + 1, treasures[i].x, treasures[i].y);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}