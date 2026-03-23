#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    // Nếu người dùng quên gõ số giây (VD: chỉ gõ "sleep" thay vì "sleep 10")
    if(argc != 2){
        fprintf(2, "Cach dung: sleep <so_giay>\n");
        exit(1);
    }

    // Đổi chữ "10" thành số 10 rồi gọi system call báo cho Kernel đi ngủ
    sleep(atoi(argv[1]));
 
    exit(0);
}
