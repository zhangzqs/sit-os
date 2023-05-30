#include "printf.h"
#include "rand.h"
#include "types.h"
#include "uart.h"

#define MAZE_MAX_SIZE 51
#define OBSTACLE 1
#define EMPTY 0

static int map[MAZE_MAX_SIZE][MAZE_MAX_SIZE];
static int width, height, beginX, beginY, endX, endY, dfsX, dfsY;

static void generate() {
  int x = dfsX;
  int y = dfsY;

  int dirsX[4] = {1, -1, 0, 0};
  int dirsY[4] = {0, 0, 1, -1};
  // 随机打乱方向
  for (int i = 0; i < 4; i++) {
    int r = rand() % 4;
    int temp = dirsX[i];
    dirsX[i] = dirsX[r];
    dirsX[r] = temp;
    temp = dirsY[i];
    dirsY[i] = dirsY[r];
    dirsY[r] = temp;
  }

  // 打破墙壁
  map[x][y] = EMPTY;
  for (int i = 0; i < 4; i++) {
    int dx = x + dirsX[i] * 2;
    int dy = y + dirsY[i] * 2;
    if (dx >= 1 && dx <= width - 2 && dy >= 1 && dy <= height - 2 &&
        map[dx][dy] == OBSTACLE) {
      map[x + dirsX[i]][y + dirsY[i]] = EMPTY;  // 挖通道

      dfsX = dx;
      dfsY = dy;
      generate();
    }
  }
}

static int currentX, currentY;

static void print_maze() {
  printf("Use 'w', 'a', 's', 'd' to move.\n");
  printf("Use 'q' to quit.\n");
  // 输出
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      if (map[i][j] == OBSTACLE) {
        printf("██");
      } else if (currentX == i && currentY == j) {
        printf("\033[34m▓▓\033[0m");
      } else {
        printf("··");
      }
    }
    printf("\n");
  }
}

int user_maze(int argc, char **argv) {
  if (argc < 3) {
    printf("Usage: maze <width> <height>\n");
    return -1;
  }
  width = atoi(argv[1]);
  height = atoi(argv[2]);
  if (width < 5 || width > MAZE_MAX_SIZE || height < 5 ||
      height > MAZE_MAX_SIZE) {
    printf("Invalid width or height max width: %d, max height: %d\n",
           MAZE_MAX_SIZE, MAZE_MAX_SIZE);
    return -1;
  }
  // width和height必须为奇数
  if (width % 2 == 0) {
    width++;
  }
  if (height % 2 == 0) {
    height++;
  }
  beginX = 0;
  beginY = 1;
  endX = width - 2;
  endY = height - 1;
  dfsX = (rand() % (width - 2) + 1) | 1;
  dfsY = (rand() % (height - 2) + 1) | 1;
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      map[i][j] = OBSTACLE;
    }
  }
  map[beginX][beginY] = EMPTY;
  map[endX][endY] = EMPTY;
  generate();

  currentX = beginX;
  currentY = beginY;

  while (1) {
    clear();
    print_maze();
    if (currentX == endX && currentY == endY) {
      printf("You win!\n");
      break;
    }
    switch (uart_getch()) {
      case 'w':
        if (map[currentX - 1][currentY] == EMPTY) {
          currentX--;
        }
        break;
      case 's':
        if (map[currentX + 1][currentY] == EMPTY) {
          currentX++;
        }
        break;
      case 'a':
        if (map[currentX][currentY - 1] == EMPTY) {
          currentY--;
        }
        break;
      case 'd':
        if (map[currentX][currentY + 1] == EMPTY) {
          currentY++;
        }
        break;
      case 'q':
        printf("You quit!\n");
        return 0;
      default:
        break;
    }
  }

  return 0;
}