#include "rand.h"

// 初始的随机数种子
unsigned int rand_seed = 1;

// 生成随机数
unsigned int rand() {
  // X_n+1 = (A * X_n + C) mod m
  rand_seed = (16807 * rand_seed) % 2147483647;
  return rand_seed;
}