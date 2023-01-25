//
// Created by Kangkook Jee on 10/15/20.
//

/*********************************************************************
 *
 *  fibonacci function (iterative).
 *
 *   This is the target program to be profiled.
 *
 *********************************************************************/

#include <stdint.h>
#include <stdio.h>

uint64_t user_prog(void *a) __attribute__((alias("fibp")));

uint64_t fibp(void *a) {
  int32_t n = *((int32_t *)a);

  /* Declare an array to store Fibonacci numbers. */
  int f[n + 2]; // 1 extra to handle case, n = 0
  int i;

  /* 0th and 1st number of the series are 0 and 1*/
  f[0] = 1;
  f[1] = 1;

  for (i = 2; i <= n; i++) {
    /* Add the previous 2 numbers in the series
       and store it */
    f[i] = f[i - 1] + f[i - 2];
  }
  return f[n];
}
