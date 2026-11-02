#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/ptree.h"

// Hàm đệ quy để in cây tiến trình theo dạng thụt lề
void print_tree(int ppid, int indent, struct ptreeinfo *buf, int count, int *visited) {
    for(int i = 0; i < count; i++) {
        if(buf[i].ppid == ppid && !visited[i]) {
            visited[i] = 1;
            // In khoảng trắng thụt lề tương ứng với cấp bậc
            for(int j = 0; j < indent; j++) {
                printf("  ");
            }
            // In thông tin chi tiết tiến trình
            printf("%d %s state=%d mem=%d\n", buf[i].pid, buf[i].name, buf[i].state, (int)buf[i].memsize);
            // Đệ quy in các tiến trình con của tiến trình hiện tại
            print_tree(buf[i].pid, indent + 1, buf, count, visited);
        }
    }
}

int main() {
    struct ptreeinfo buf[64];
    int n = ptree(buf, 64);
    
    if(n < 0) {
        printf("pstree: error reading process tree\n");
        exit(1);
    }

    int visited[64] = {0};

    // Bắt đầu duyệt từ gốc (ppid = 0, thường là tiến trình init)
    print_tree(0, 0, buf, n, visited);

    // Xử lý vét cạn các tiến trình mồ côi (nếu còn sót lại chưa được ghé thăm)
    for(int i = 0; i < n; i++) {
        if(!visited[i]) {
            printf("%d %s state=%d mem=%d\n", buf[i].pid, buf[i].name, buf[i].state, (int)buf[i].memsize);
            visited[i] = 1;
            print_tree(buf[i].pid, 1, buf, n, visited);
        }
    }

    exit(0);
}