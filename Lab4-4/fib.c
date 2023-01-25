//
// Created by Kangkook Jee on 10/15/20.
//

/*********************************************************************
 *
 *  fibonacci function (recursive).
 *
 *   This is the target program to be profiled.
 *
 *********************************************************************/

#include <stdint.h>
#include <stdio.h>

uint64_t user_prog(uint64_t i) __attribute__((alias("fib")));

/*
 * the original implementation, exponential complexity.
 */

uint64_t fib(uint64_t i) {
  if (i <= 1) {
    return 1;
  }

  return fib(i - 1) + fib(i - 2);
}
