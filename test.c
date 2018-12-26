/*

  CFI eval: quick and dirty evaluation of a CFI mechanism.

  CFI eval measures the precision and performance of a CFI mechanism
  for the forward edge and tests different dispatch mechanisms and
  pointer corruption types. If it breaks you may keep the pieces.

  Threats to validity:
  - The current version is aimed at CFI mechanisms that use function
    prototype matching and assumes that same prototypes will just fail
  - No support for C++ (yet)
  - Incomplete performance testing through a primitive microbenchmark
  - Non-address taken testing is limited to a single prototype

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>


/*****************************
 * Indirect helper functions *
 *****************************/

struct structii {
  int a;
  int b;
};

enum enumi {
  enuma,
  enumb,
  enumc
};

union unionsei {
  struct structii *ptr;
  enum enumi ei;
  int i;
};

__attribute__((noinline)) void vafunv(void) {
  printf("vafunv\n");
}

__attribute__((noinline)) void vfunv(void) {
  printf("vfunv\n");
}

__attribute__((noinline)) void vfunv2(void) {
  printf("vfunv2\n");
}

__attribute__((noinline)) void vfun() {
  printf("vfun\n");
}

__attribute__((noinline)) void vfuni(int arg) {
  printf("vfuni %d\n", arg);
}

__attribute__((noinline)) void vfune(enum enumi arg) {
  printf("vfune %d\n", arg);
}

__attribute__((noinline)) void vfunf(float arg) {
  printf("vfuni %f\n", arg);
}

__attribute__((noinline)) void vfuns(struct structii arg) {
  printf("vfuns %d\n", arg.a);
}

__attribute__((noinline)) void vfunpv(void *arg) {
  printf("vfunpv %p\n", arg);
}

__attribute__((noinline)) void vfunps(struct structii *arg) {
  printf("vfunps %p\n", (void*)arg);
}

__attribute__((noinline)) void vfunu(union unionsei arg) {
  printf("vfunu %p\n", (void*)(arg.ptr));
}

__attribute__((noinline)) void vfunia(int count, ...) {
  printf("vfunia %d\n", count);
}

__attribute__((noinline)) int ifunv(void) {
  printf("ifunv\n");
  return 0;
}

__attribute__((noinline)) int ifuni(int arg) {
  printf("ifuni %d\n", arg);
  return 0;
}

__attribute__((noinline)) float ffunv(void) {
  printf("ffunv\n");
  return 0;
}

__attribute__((noinline)) float ffuni(int arg) {
  printf("ffuni %d\n", arg);
  return 0;
}

__attribute__((noinline)) int *pifunv(void) {
  printf("pifunv\n");
  return 0;
}

__attribute__((noinline)) int *pifuni(int arg) {
  printf("pifuni %d\n", arg);
  return 0;
}

__attribute__((noinline)) void rawfun(void) {
  __asm__ volatile(
    "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
    "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
    "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
    "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
    "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
    "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
    "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
    "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
    "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
    "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
    );
  printf("raw fun\n");
}


/********************************************
 * Data structures for evaluation framework *
 ********************************************/

#define NRFUN (sizeof(struct supertype)/sizeof(void*))

struct supertype {
  void (*vfunv[1])(void);
  void (*vfun[1])();
  void (*vfuni[1])(int);
  void (*vfune[1])(enum enumi);
  void (*vfunf[1])(float);
  void (*vfuns[1])(struct structii);
  void (*vfunpv[1])(void *);
  void (*vfunps[1])(struct structii *);
  void (*vfunu[1])(union unionsei);
  void (*vfunia[1])(int, ...);
  int (*ifunv[1])(void);
  int (*ifuni[1])(int);
  float (*ffunv[1])(void);
  float (*ffuni[1])(int);
  int *(*pifunv[1])(void);
  int *(*pifuni[1])(int);
  void (*rawfun[1])(void);
};

const char *typestrs[] = {
  "void (*)(void);",
  "void (*)();",
  "void (*)(int);",
  "void (*)(enum enumi);",
  "void (*)(float);",
  "void (*)(struct structii);",
  "void (*)(void *);",
  "void (*)(struct structii *);",
  "void (*)(union unionsei);",
  "void (*)(int, ...);",
  "int (*)(void);",
  "int (*)(int);",
  "float (*)(void);",
  "float (*)(int);",
  "int *(*)(void);",
  "int *(*)(int);",
  "void (*)(void);"
};


/* Function pointers in struct */
struct supertype st = {
  .vfunv = {&vfunv},
  .vfun = {&vfun},
  .vfuni = {&vfuni},
  .vfune = {&vfune},
  .vfunf = {&vfunf},
  .vfuns = {&vfuns},
  .vfunpv = {&vfunpv},
  .vfunps = {&vfunps},
  .vfunu = {&vfunu},
  .vfunia = {&vfunia},
  .ifunv = {&ifunv},
  .ifuni = {&ifuni},
  .ffunv = {&ffunv},
  .ffuni = {&ffuni},
  .pifunv = {&pifunv},
  .pifuni = {&pifuni},
  .rawfun = {&rawfun}
};


/* Function pointers for offset checks */
void (*funptr0)(void) = &vfunv;
void (*funptr1)() = &vfun; 
void (*funptr2)(int) = &vfuni;
void (*funptr3)(enum enumi) = &vfune;
void (*funptr4)(float) = &vfunf;
void (*funptr5)(struct structii) = &vfuns;
void (*funptr6)(void *) = &vfunpv;
void (*funptr7)(struct structii *) = &vfunps;
void (*funptr8)(union unionsei) = &vfunu;
void (*funptr9)(int, ...) = &vfunia;
int (*funptr10)(void) = &ifunv;
int (*funptr11)(int) = &ifuni;
float (*funptr12)(void) = &ffunv;
float (*funptr13)(int) = &ffuni;
int *(*funptr14)(void) = &pifunv;
int *(*funptr15)(int) = &pifuni;
void (*funptr16)(void) = &rawfun;

unsigned long addresses[] = {
  (unsigned long)&vfunv, (unsigned long)&vfun,
  (unsigned long)&vfuni, (unsigned long)&vfune,
  (unsigned long)&vfunf, (unsigned long)&vfuns,
  (unsigned long)&vfunpv, (unsigned long)&vfunps,
  (unsigned long)&vfunu, (unsigned long)&vfunia,
  (unsigned long)&ifunv, (unsigned long)&ifuni,
  (unsigned long)&ffunv, (unsigned long)&ffuni,
  (unsigned long)&pifunv, (unsigned long)&pifuni,
  (unsigned long)&rawfun };


/**************************************************
 * Individual helper functions for test framework *
 **************************************************/

/**
 * Dispatch a function through the struct.
 * @src expected function offset
 * @dst function that is actually used
 * @off offset used for parameters
 */
void dispatchstruct(int src, int dst, int off) {
  struct structii arg = { .a = 42, .b = off };
  union unionsei arg2 = { .i = 23 };

  switch (src) {
    case 0:
      st.vfunv[dst-src](); break;
    case 1:
      st.vfun[dst-src](); break;
    case 2:
      st.vfuni[dst-src](src); break;
    case 3:
      st.vfune[dst-src](src); break;
    case 4:
      st.vfunf[dst-src]((float)(src)); break;
    case 5:
      st.vfuns[dst-src](arg); break;
    case 6:
      st.vfunpv[dst-src](&arg); break;
    case 7:
      st.vfunps[dst-src](&arg); break;
    case 8:
      st.vfunu[dst-src](arg2); break;
    case 9:
      st.vfunia[dst-src](src, dst);
      st.vfunia[dst-src](src, dst, src); break;
    case 10:
      st.ifunv[dst-src](); break;
    case 11:
      st.ifuni[dst-src](src); break;
    case 12:
      st.ffunv[dst-src](); break;
    case 13:
      st.ffuni[dst-src](src); break;
    case 14:
      st.pifunv[dst-src](); break;
    case 15:
      st.pifuni[dst-src](src); break;
    case 16:
      st.rawfun[dst-src](); break;
    default:
      printf("Source index out of range.\n");
  }
}


/**
 * Dispatch a function through a corrupted pointer
 * @src expected function offset
 * @dst function that is actually used
 * @soff offset used for parameters
 */
void dispatchoffset(int src, int dst, int soff) {
  struct structii arg = { .a = 42, .b = soff };
  union unionsei arg2 = { .i = 23 };
  long off = (long)(addresses[dst] - addresses[src]);
  long *ptr;
  switch (src) {
    case 0:
      ptr = (long*)&funptr0;
      *ptr += off;
      funptr0(); break;
    case 1:
      ptr = (long*)&funptr1;
      *ptr += off;
      funptr1(); break;
    case 2:
      ptr = (long*)&funptr2;
      *ptr += off;
      funptr2(src); break;
    case 3:
      ptr = (long*)&funptr3;
      *ptr += off;
      funptr3(src); break;
    case 4:
      ptr = (long*)&funptr4;
      *ptr += off;
      funptr4((float)src); break;
    case 5:
      ptr = (long*)&funptr5;
      *ptr += off;
      funptr5(arg); break;
    case 6:
      ptr = (long*)&funptr6;
      *ptr += off;
      funptr6(&arg); break;
    case 7:
      ptr = (long*)&funptr7;
      *ptr += off;
      funptr7(&arg); break;
    case 8:
      ptr = (long*)&funptr8;
      *ptr += off;
      funptr8(arg2); break;
    case 9:
      ptr = (long*)&funptr9;
      *ptr += off;
      funptr9(src, dst);
      funptr9(src, dst, src); break;
    case 10:
      ptr = (long*)&funptr10;
      *ptr += off;
      funptr10(); break;
    case 11:
      ptr = (long*)&funptr11;
      *ptr += off;
      funptr11(src); break;
    case 12:
      ptr = (long*)&funptr12;
      *ptr += off;
      funptr12(); break;
    case 13:
      ptr = (long*)&funptr13;
      *ptr += off;
      funptr13(src); break;
    case 14:
      ptr = (long*)&funptr14;
      *ptr += off;
      funptr14(); break;
    case 15:
      ptr = (long*)&funptr15;
      *ptr += off;
      funptr15(src); break;
    case 16:
      ptr = (long*)&funptr16;
      *ptr += off;
      funptr16(); break;
    default:
      printf("Source index out of range.\n");
  }
}


/**
 * Quick dispatch of a single function to test same prototype checks
 * @src offset to other function
 * @dst expected to be 0
 */
void dispatchsingle(int src, int dst, int off) {
  void (*ptr)(void) = &vfunv;
  unsigned long *lptr = ((unsigned long*)&ptr)+dst+src;
  *lptr += off;
  ptr();
}


/**
 * Fork and run a single test in a child process
 * @ptr test function
 * @src declared type
 * @dst runttime type
 * @off offset
 * @xpectcrash do we expect a crash?
 */
void crashtest(void (*ptr)(int, int, int), int src, int dst, int off, int xpectcrash) {
  pid_t pid = fork();
  if (pid == -1) { 
    printf("Fork error\n");
    exit(-1);
  }
  if (pid == 0) {
    ptr(src, dst, off);
    exit(0);
  } else {
    int status;
    waitpid(pid, &status, 0);
    if (status == 0 && xpectcrash) {
      printf("Err: No crash for %s -> %s\n", typestrs[src], typestrs[dst]);
    }
    if (status !=0 && !xpectcrash) {
      printf("Err: Unexpected crash %s -> %s\n", typestrs[src], typestrs[dst]);
    }
  }
}


/**
 * Quick runner that iterates through all tests in the struct.
 * Dispatching through an array access.
 */
void structfun(void) {
  for (unsigned long i = 0; i<NRFUN; i++) {
    for (unsigned long j = 0; j<NRFUN; j++) {
      int xpectcrash = (i!=j);
      if (i == NRFUN-1 && j == 0) xpectcrash = 0;
      if (i == NRFUN-1 && j == NRFUN-1) xpectcrash = 1;
      crashtest(&dispatchstruct, i, j, 23, xpectcrash);
    }
  }
}


/**
 * Quick runner that iterates through all tests for indirect dispatch
 * Dispatching through a function pointer
 */
void offsetfun(void) {
  for (unsigned long i = 0; i<NRFUN; i++) {
    for (unsigned long j = 0; j<NRFUN; j++) {
      int xpectcrash = (i!=j);
      if (i == NRFUN-1 && j == 0) xpectcrash = 0;
      if (i == NRFUN-1 && j == NRFUN-1) xpectcrash = 1;
      crashtest(&dispatchoffset, i, j, 23, xpectcrash);
    }
  }
}


/**
 * Dispatcher for same function checks
 */
void sameprotofun(void) {
  long offset = (long)&vfunv2 - (long)&vfunv;
  crashtest(&dispatchsingle, 0, 0, offset, 1);
}


void nottakenfun(long off) {
  crashtest(&dispatchsingle, 0, 0, off, 1);
}


/**********************************
 * Performance measurement helper *
 **********************************/

#define NRSPEED 1000000000
__attribute__((noinline)) int quickfun(int a, int b) {
  __asm__ volatile("nop\n");
  return a*b;
}

void speedfun(int (*ptr)(int, int), int a, int b) {
  unsigned long lo, hi;
  unsigned long start, end;
  __asm__ volatile ( "rdtsc" : "=a" (lo), "=d" (hi));
  start = lo | (hi << 32);
  for (unsigned long i =0; i < NRSPEED; i++)
    ptr(a, b);
  __asm__ volatile ( "rdtsc" : "=a" (lo), "=d" (hi));
  end = lo | (hi << 32);

  double execs = ((double) (end - start)) / NRSPEED;
  printf("Execution time: cycles %f per dispatch\n", execs);
}


long findfun(char *prog, char *fun1) {
#define MAXCMD 256
  char tmpl[] = "objdump -d %s|grep '<%s>'|awk '{print$1}'";
  char cmd[MAXCMD];
  FILE *fd;
  long ret;
  if (strlen(tmpl) - 4 + strlen(prog) + strlen(fun1) >= MAXCMD)
    exit(-1);
  sprintf(cmd, tmpl, prog, fun1);
  if ((fd = popen(cmd, "r")) == NULL)
    exit(-1);
  if (fscanf(fd, "%lx", (unsigned long*)&ret) != 1)
    exit(-1);
  pclose(fd);
  return ret;
}


long findoffset(char *prog, char *fun1, char *fun2) {
  return findfun(prog, fun2) - findfun(prog, fun1);
}


int main(int argc, char* argv[]) {
  // Initialize rawfun to point into the middle of the function
  unsigned char* rawfunchar = (unsigned char*)&(st.rawfun);
  *rawfunchar += 0x30;
  rawfunchar = (unsigned char*)&funptr16;
  *rawfunchar += 0x30;
  addresses[16] += 0x30;

  // Make sure calls are not devirtualized or dead code removed
  int (* volatile ptr)(int, int);
  ptr = (argc != 42) ? &quickfun : (int(*)(int,int))(&vfunv);
  if (argc == 42) vafunv();

  /* First precision test */
  printf("Running struct tests (indirect dispatch through a struct/array).\n");
  structfun();

  /* Second precision test */
  printf("\nRunning offset tests (indirect dispatch through a corrupted pointer).\n");
  offsetfun();

  /* Thirs precision test */
  printf("\nTesting same prototype precision.\n");
  sameprotofun();

  printf("\nTesting precision to non-address taken functions.\n");
  nottakenfun(findoffset(argv[0], "vfunv", "vafunv"));

  /* Speed test */
  printf("\nTesting dispatch speed\n");
  speedfun(ptr, argc, atoi(argv[0]));

  printf("\nAll tests completed.\n"); 
  
  return 0;
}
