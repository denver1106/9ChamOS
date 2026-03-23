#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

// Tao file rong
static void touch(char *path) {
  int fd = open(path, O_CREATE | O_WRONLY);
  if (fd < 0) {
    fprintf(2, "treetest: cannot create %s\n", path);
    return;
  }
  close(fd);
}

// Chay tree va bat output vao buf, tra ve so byte doc duoc
static int run_tree_capture(char *arg, char *buf, int bufsz) {
  int p[2];
  pipe(p);

  int pid = fork();
  if (pid == 0) {
    // Child: redirect stdout sang pipe
    close(1);
    dup(p[1]);
    close(p[0]);
    close(p[1]);

    if (arg) {
      char *argv[] = {"tree", arg, 0};
      exec("tree", argv);
    } else {
      char *argv[] = {"tree", 0};
      exec("tree", argv);
    }
    fprintf(2, "treetest: exec tree failed\n");
    exit(1);
  }

  // Parent: doc output tu pipe
  close(p[1]);
  int total = 0;
  int n;
  while ((n = read(p[0], buf + total, bufsz - total - 1)) > 0)
    total += n;
  buf[total] = 0;
  close(p[0]);
  wait(0);
  return total;
}

// Chay tree va bat stderr vao buf (cho test loi)
static int run_tree_capture_err(char *arg, char *buf, int bufsz) {
  int p[2];
  pipe(p);

  int pid = fork();
  if (pid == 0) {
    // Child: redirect stderr sang pipe
    close(2);
    dup(p[1]);
    close(p[0]);
    close(p[1]);

    char *argv[] = {"tree", arg, 0};
    exec("tree", argv);
    exit(1);
  }

  close(p[1]);
  int total = 0;
  int n;
  while ((n = read(p[0], buf + total, bufsz - total - 1)) > 0)
    total += n;
  buf[total] = 0;
  close(p[0]);
  wait(0);
  return total;
}

// So sanh 2 chuoi, tra ve 1 neu giong
static int streq(char *a, char *b) {
  while (*a && *b) {
    if (*a != *b)
      return 0;
    a++;
    b++;
  }
  return *a == *b;
}

// Kiem tra actual co chua expected khong
static int strcontains(char *actual, char *expected) {
  int la = strlen(actual);
  int le = strlen(expected);
  if (le > la)
    return 0;
  for (int i = 0; i <= la - le; i++) {
    int match = 1;
    for (int j = 0; j < le; j++) {
      if (actual[i + j] != expected[j]) {
        match = 0;
        break;
      }
    }
    if (match)
      return 1;
  }
  return 0;
}

static int passed = 0;
static int failed = 0;

static void check(char *testname, char *actual, char *expected, int exact) {
  int ok;
  if (exact)
    ok = streq(actual, expected);
  else
    ok = strcontains(actual, expected);

  if (ok) {
    printf("  [PASS] %s\n", testname);
    passed++;
  } else {
    printf("  [FAIL] %s\n", testname);
    printf("    Mong doi:\n---\n%s---\n", expected);
    printf("    Thuc te:\n---\n%s---\n", actual);
    failed++;
  }
}

char buf[4096];

int main(int argc, char *argv[]) {
  printf("========================================\n");
  printf("  TREE AUTO TEST\n");
  printf("========================================\n\n");

  // ========== TEST 1 ==========
  printf("--- TEST 1: Cau truc co ban ---\n");
  mkdir("td");
  touch("td/b");
  mkdir("td/aa");
  touch("td/aa/b");
  touch("td/aa/c");
  mkdir("td/ab");
  touch("td/ab/d");

  run_tree_capture("td", buf, sizeof(buf));
  check("Cau truc co ban", buf,
        "td/\n"
        "  b\n"
        "  aa/\n"
        "    b\n"
        "    c\n"
        "  ab/\n"
        "    d\n",
        1);

  // ========== TEST 2 ==========
  printf("--- TEST 2: Thu muc long sau ---\n");
  mkdir("td/aa/aaa");
  touch("td/aa/aaa/file");

  run_tree_capture("td", buf, sizeof(buf));
  check("Thu muc long 3 cap", buf,
        "td/\n"
        "  b\n"
        "  aa/\n"
        "    b\n"
        "    c\n"
        "    aaa/\n"
        "      file\n"
        "  ab/\n"
        "    d\n",
        1);

  // ========== TEST 3 ==========
  printf("--- TEST 3: Thu muc rong ---\n");
  mkdir("emptydir");

  run_tree_capture("emptydir", buf, sizeof(buf));
  check("Thu muc rong", buf, "emptydir/\n", 1);

  // ========== TEST 4 ==========
  printf("--- TEST 4: tree \".\" ---\n");
  run_tree_capture(".", buf, sizeof(buf));
  check("Bat dau bang ./", buf, "./\n",
        0); // chi kiem tra co chua "./\n"

  // ========== TEST 5 ==========
  printf("--- TEST 5: tree \"/\" ---\n");
  run_tree_capture("/", buf, sizeof(buf));
  check("Bat dau bang /", buf, "/\n",
        0); // chi kiem tra co chua "/\n"

  // ========== TEST 6 ==========
  printf("--- TEST 6: tree khong tham so ---\n");
  run_tree_capture(0, buf, sizeof(buf));
  check("Mac dinh dung \".\"", buf, "./\n", 0);

  // ========== TEST 7 ==========
  printf("--- TEST 7: Thu muc khong ton tai ---\n");
  run_tree_capture_err("khongtontai", buf, sizeof(buf));
  check("Bao loi cannot open", buf, "cannot open", 0);

  // ========== TEST 8 ==========
  printf("--- TEST 8: Nhieu file ---\n");
  mkdir("mf");
  touch("mf/f1");
  touch("mf/f2");
  touch("mf/f3");

  run_tree_capture("mf", buf, sizeof(buf));
  check("Nhieu file trong 1 dir", buf,
        "mf/\n"
        "  f1\n"
        "  f2\n"
        "  f3\n",
        1);

  // ========== KET QUA ==========
  printf("\n========================================\n");
  printf("  KET QUA: %d PASSED, %d FAILED\n", passed, failed);
  if (failed == 0)
    printf("  >> TAT CA TEST PASSED! <<\n");
  else
    printf("  >> CO TEST FAIL, KIEM TRA LAI! <<\n");
  printf("========================================\n");

  exit(0);
}
