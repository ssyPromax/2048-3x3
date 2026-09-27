// 2048 (3x3) - 控制台版
// 操作: 方向键 或 W/A/S/D 移动, 按 Q 退出
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>

const int N = 3;           // 3x3 棋盘
const int WIN_TILE = 2048; // 获胜方块

int board[N][N];
long long score = 0;
bool winShown = false;

void clearScreen() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD c = {0, 0};
    SetConsoleCursorPosition(h, c);
}

void init() {
    srand((unsigned)time(NULL));
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            board[i][j] = 0;
    score = 0;
    winShown = false;
}

// 在随机空格放一个 2 (90%) 或 4 (10%)
void addTile() {
    int empty[N * N][2], cnt = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            if (board[i][j] == 0) {
                empty[cnt][0] = i;
                empty[cnt][1] = j;
                cnt++;
            }
    if (cnt == 0) return;
    int k = rand() % cnt;
    board[empty[k][0]][empty[k][1]] = (rand() % 10 == 0) ? 4 : 2;
}

bool canMove() {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            if (board[i][j] == 0) return true;
            if (j + 1 < N && board[i][j] == board[i][j + 1]) return true;
            if (i + 1 < N && board[i][j] == board[i + 1][j]) return true;
        }
    return false;
}

// 处理一行: 去零 -> 合并 -> 去零, 返回本行得分
int mergeLine(int line[N]) {
    int tmp[N] = {0}, t = 0;
    for (int i = 0; i < N; i++)
        if (line[i] != 0) tmp[t++] = line[i];
    int gain = 0;
    for (int i = 0; i + 1 < N; i++)
        if (tmp[i] != 0 && tmp[i] == tmp[i + 1]) {
            tmp[i] *= 2;
            gain += tmp[i];
            tmp[i + 1] = 0;
        }
    for (int i = 0; i < N; i++) line[i] = 0;
    t = 0;
    for (int i = 0; i < N; i++)
        if (tmp[i] != 0) line[t++] = tmp[i];
    return gain;
}

// dir: 0=左 1=右 2=上 3=下
bool move(int dir) {
    int before[N][N];
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            before[i][j] = board[i][j];

    for (int k = 0; k < N; k++) {
        int line[N];
        for (int i = 0; i < N; i++) {
            switch (dir) {
                case 0: line[i] = board[k][i]; break;              // 左: 第 k 行
                case 1: line[i] = board[k][N - 1 - i]; break;      // 右: 行逆序
                case 2: line[i] = board[i][k]; break;              // 上: 第 k 列
                case 3: line[i] = board[N - 1 - i][k]; break;      // 下: 列逆序
            }
        }
        int gain = mergeLine(line);
        score += gain;
        for (int i = 0; i < N; i++) {
            switch (dir) {
                case 0: board[k][i] = line[i]; break;
                case 1: board[k][N - 1 - i] = line[i]; break;
                case 2: board[i][k] = line[i]; break;
                case 3: board[N - 1 - i][k] = line[i]; break;
            }
        }
    }

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            if (before[i][j] != board[i][j]) return true; // 有变化
    return false;
}

bool hasWon() {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            if (board[i][j] >= WIN_TILE) return true;
    return false;
}

void draw() {
    clearScreen();
    printf("================================\n");
    printf("          2 0 4 8  (3x3)        \n");
    printf("================================\n\n");
    for (int i = 0; i < N; i++) {
        printf("  +------+------+------+\n  ");
        for (int j = 0; j < N; j++)
            if (board[i][j] == 0)
                printf("|      ");
            else
                printf("|%6d", board[i][j]);
        printf("|\n");
    }
    printf("  +------+------+------+\n\n");
    printf("  得分: %lld\n\n", score);
    printf("  方向键 / WASD 移动, Q 退出\n");
    fflush(stdout);
}

int getKey() {
    int c = _getch();
    if (c == 0 || c == 224) { // 方向键前缀
        c = _getch();
        switch (c) {
            case 72: return 2; // 上
            case 80: return 3; // 下
            case 75: return 0; // 左
            case 77: return 1; // 右
        }
        return -1;
    }
    switch (c) {
        case 'a': case 'A': return 0;
        case 'd': case 'D': return 1;
        case 'w': case 'W': return 2;
        case 's': case 'S': return 3;
        case 'q': case 'Q': return 4;
    }
    return -1;
}

int main() {
    system("chcp 65001 > nul"); // UTF-8, 避免中文乱码
    init();
    addTile();
    addTile();
    draw();

    while (true) {
        int key = getKey();
        if (key == 4) break;
        if (key < 0) continue;
        if (move(key)) {
            addTile();
            draw();
            if (!winShown && hasWon()) {
                printf("\n  恭喜! 你合成了 %d!\n", WIN_TILE);
                printf("  按任意键继续挑战...\n");
                _getch();
                winShown = true;
                draw();
            }
            if (!canMove()) {
                printf("\n  游戏结束! 最终得分: %lld\n", score);
                printf("  按 R 重开, 其他键退出...\n");
                int c = _getch();
                if (c == 'r' || c == 'R') {
                    init();
                    addTile();
                    addTile();
                    draw();
                    continue;
                }
                break;
            }
        }
    }
    return 0;
}
