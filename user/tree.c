#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

static void
print_indent(int depth)
{
  for (int i = 0; i < depth; i++)
    printf("  "); // 2 spaces mỗi level
}

static char*
base_name(char *path)
{
  int n = strlen(path);

  while (n > 1 && path[n - 1] == '/')
    n--;

  int start = n - 1;
  while (start >= 0 && path[start] != '/')
    start--;

  if (start < 0)
    return path;

  return path + start + 1;
}

static void
tree(char *path, int depth)
{
  int fd;
  struct stat st;

  fd = open(path, 0);
  if (fd < 0) {
    fprintf(2, "tree: cannot open %s\n", path);
    exit(1);
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "tree: cannot stat %s\n", path);
    close(fd);
    exit(1);
  }

  // Nếu path là file thường: chỉ in file và return
  if (st.type != T_DIR) {
    char *name = base_name(path);
    print_indent(depth);
    printf("%s\n", name);
    close(fd);
    return;
  }

  print_indent(depth);
  if (strcmp(path, "/") == 0)
    printf("/\n");
  else if (strcmp(path, ".") == 0)
    printf("./\n");
  else {
    char *dname = base_name(path);
    printf("%s/\n", dname);
  }

  // Duyệt các entry trong directory
  struct dirent de;
  char buf[512], *p;

  if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
    fprintf(2, "tree: path too long %s\n", path);
    close(fd);
    return;
  }

  strcpy(buf, path);
  p = buf + strlen(buf);
  if (p > buf && *(p-1) != '/')
    *p++ = '/';

  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0)
      continue;

    char name[DIRSIZ + 1];
    memmove(name, de.name, DIRSIZ);
    name[DIRSIZ] = 0;

    // Skip "." and ".."
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
      continue;

    // Build child path: parent/name
    memmove(p, name, strlen(name));
    p[strlen(name)] = 0;

    struct stat st2;
    if (stat(buf, &st2) < 0) {
      fprintf(2, "tree: cannot stat %s\n", buf);
      continue; // yêu cầu: stat fail thì báo lỗi nhưng vẫn tiếp tục
    }

    if (st2.type == T_DIR) {
      tree(buf, depth + 1);
    } else {
      print_indent(depth + 1);
      printf("%s\n", name);
    }
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  if (argc > 2) {
    fprintf(2, "usage: tree [directory]\n");
    exit(1);
  }

  char *start = (argc == 2) ? argv[1] : ".";
  tree(start, 0);
  exit(0);
}