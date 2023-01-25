//
// Created by Kangkook Jee on 10/15/20.
//

/*********************************************************************
 *
 *  isPrime() function.
 *
 *   This is the target program to be profiled.
 *   primality test with a single arg + global context.
 *
 *********************************************************************/

#include <stdint.h>
#include <stdio.h>

#include <stdint.h>
#include <stdio.h>

uint64_t user_prog(void *i) __attribute__((alias("isPrime")));

int chk = 0;
uint64_t isPrime(int n) {

  if (chk == 0) {
    chk = n / 2;
    isPrime(n);
  } else if (chk == 1) {
    return 1;
  } else {
    if (n % chk == 0) {
      return 0;
    } else {
      chk = chk - 1;
      isPrime(n);
    }
  }
}
