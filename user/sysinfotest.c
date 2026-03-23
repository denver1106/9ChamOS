#include "kernel/types.h"
#include "kernel/riscv.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

void
testmem()
{
  struct sysinfo info;
  uint64 n0, n1;

  if (sysinfo(&info) < 0) {
    printf("sysinfo failed\n");
    exit(1);
  }
  n0 = info.freemem;

  void *p = sbrk(4096);
  if (p == (void*)-1) {
    printf("sbrk failed\n");
    exit(1);
  }

  if (sysinfo(&info) < 0) {
    printf("sysinfo failed\n");
    exit(1);
  }
  n1 = info.freemem;

  if (n1 != n0 - 4096) {
    printf("free memory did not decrease by 4096; n0 %d n1 %d\n", (int)n0, (int)n1);
    exit(1);
  }

  sbrk(-4096);
  if (sysinfo(&info) < 0) {
    printf("sysinfo failed\n");
    exit(1);
  }

  if (info.freemem != n0) {
    printf("free memory did not return to n0; n0 %d n1 %d\n", (int)n0, (int)info.freemem);
    exit(1);
  }
}

void
testproc()
{
  struct sysinfo info;
  uint64 n0, n1;

  if (sysinfo(&info) < 0) {
    printf("sysinfo failed\n");
    exit(1);
  }
  n0 = info.nproc;

  int pid = fork();
  if (pid < 0) {
    printf("fork failed\n");
    exit(1);
  }
  if (pid == 0) {
    sleep(1);
    exit(0);
  }

  if (sysinfo(&info) < 0) {
    printf("sysinfo failed\n");
    exit(1);
  }
  n1 = info.nproc;

  wait(0);

  if (n1 != n0 + 1) {
    printf("nproc did not increase by 1; n0 %d n1 %d\n",(int)n0,(int)n1);
    exit(1);
  }
}

int
main(int argc, char *argv[])
{
  testmem();
  testproc();
  printf("sysinfotest: OK\n");
  exit(0);
}
