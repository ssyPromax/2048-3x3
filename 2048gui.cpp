// 2048 (3x3) - Windows 图形界面版
// 操作: 方向键 或 W/A/S/D 移动, R 重开, Esc 退出
#include <windows.h>
#include <cstdio>
#include <ctime>

const int N = 3;                 // 3x3 棋盘
const int WIN_TILE = 2048;       // 获胜方块
const int CELL = 120;            // 格子边长
const int GAP = 12;              // 格子间距
const int MARGIN = 16;           // 棋盘外边距
const int TOPBAR = 70;           // 顶部得分栏高度
const int BOARD = MARGIN * 2 + N * CELL + (N - 1) * GAP;
const int WIN_W = BOARD;
const int WIN_H = TOPBAR + BOARD + MARGIN;

int board[N][N];
long long score = 0;
bool winShown = false;
bool gameOver = false;
HWND g_hwnd;

void init() {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            board[i][j] = 0;
    score = 0;
    winShown = false;
    gameOver = false;
}

void addTile() {
    int ei[N * N], ej[N * N], cnt = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            if (board[i][j] == 0) { ei[cnt] = i; ej[cnt] = j; cnt++; }
    if (!cnt) return;
    int k = rand() % cnt;
    board[ei[k]][ej[k]] = (rand() % 10 == 0) ? 4 : 2;
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

int mergeLine(int line[N]) {
    int tmp[N] = {0}, t = 0;
    for (int i = 0; i < N; i++)
        if (line[i]) tmp[t++] = line[i];
    int gain = 0;
    for (int i = 0; i + 1 < N; i++)
        if (tmp[i] && tmp[i] == tmp[i + 1]) {
            tmp[i] *= 2;
            gain += tmp[i];
            tmp[i + 1] = 0;
        }
    for (int i = 0; i < N; i++) line[i] = 0;
    t = 0;
    for (int i = 0; i < N; i++)
        if (tmp[i]) line[t++] = tmp[i];
    return gain;
}

bool doMove(int dir) { // 0=左 1=右 2=上 3=下
    int before[N][N];
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            before[i][j] = board[i][j];
    for (int k = 0; k < N; k++) {
        int line[N];
        for (int i = 0; i < N; i++) {
            switch (dir) {
                case 0: line[i] = board[k][i]; break;
                case 1: line[i] = board[k][N - 1 - i]; break;
                case 2: line[i] = board[i][k]; break;
                case 3: line[i] = board[N - 1 - i][k]; break;
            }
        }
        score += mergeLine(line);
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
            if (before[i][j] != board[i][j]) return true;
    return false;
}

bool hasWon() {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            if (board[i][j] >= WIN_TILE) return true;
    return false;
}

COLORREF tileColor(int v) {
    switch (v) {
        case 2:    return RGB(0xEE, 0xE4, 0xDA);
        case 4:    return RGB(0xED, 0xE0, 0xC8);
        case 8:    return RGB(0xF2, 0xB1, 0x79);
        case 16:   return RGB(0xF5, 0x95, 0x63);
        case 32:   return RGB(0xF6, 0x7C, 0x5F);
        case 64:   return RGB(0xF6, 0x5E, 0x3B);
        case 128:  return RGB(0xED, 0xCF, 0x72);
        case 256:  return RGB(0xED, 0xCC, 0x61);
        case 512:  return RGB(0xED, 0xC8, 0x50);
        case 1024: return RGB(0xED, 0xC5, 0x3F);
        case 2048: return RGB(0xED, 0xC2, 0x2E);
        default:   return RGB(0x3C, 0x3A, 0x32);
    }
}

void drawTile(HDC hdc, int x, int y, int v) {
    RECT rc = {x, y, x + CELL, y + CELL};
    HBRUSH br = CreateSolidBrush(v ? tileColor(v) : RGB(0xCD, 0xC1, 0xB4));
    FillRect(hdc, &rc, br);
    DeleteObject(br);
    if (!v) return;
    bool dark = (v <= 4);
    SetTextColor(hdc, dark ? RGB(0x77, 0x6E, 0x65) : RGB(0xFF, 0xFF, 0xFF));
    SetBkMode(hdc, TRANSPARENT);
    int len = 0, t = v;
    while (t) { len++; t /= 10; }
    int pt = (len <= 2) ? 44 : (len == 3) ? 36 : 28;
    HFONT font = CreateFont(pt, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial"));
    HFONT old = (HFONT)SelectObject(hdc, font);
    wchar_t wbuf[16];
    swprintf(wbuf, 16, L"%d", v);
    DrawTextW(hdc, wbuf, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, old);
    DeleteObject(font);
}

void drawBoard(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    // 双缓冲, 避免闪烁
    HDC mem = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, WIN_W, WIN_H);
    HBITMAP oldBmp = (HBITMAP)SelectObject(mem, bmp);

    // 背景
    RECT bg = {0, 0, WIN_W, WIN_H};
    HBRUSH bgBr = CreateSolidBrush(RGB(0xFA, 0xF8, 0xEF));
    FillRect(mem, &bg, bgBr);
    DeleteObject(bgBr);

    // 棋盘底板
    RECT panel = {MARGIN, TOPBAR, MARGIN + N * CELL + (N - 1) * GAP,
                  TOPBAR + N * CELL + (N - 1) * GAP};
    HBRUSH panelBr = CreateSolidBrush(RGB(0xBB, 0xAD, 0xA0));
    FillRect(mem, &panel, panelBr);
    DeleteObject(panelBr);

    // 格子
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            int x = MARGIN + j * (CELL + GAP);
            int y = TOPBAR + i * (CELL + GAP);
            drawTile(mem, x, y, board[i][j]);
        }

    // 标题与得分
    SetBkMode(mem, TRANSPARENT);
    SetTextColor(mem, RGB(0x77, 0x6E, 0x65));
    HFONT tf = CreateFont(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial"));
    HFONT sf = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial"));
    HFONT oldF = (HFONT)SelectObject(mem, tf);
    RECT trc = {MARGIN, 14, WIN_W - MARGIN, 52};
    DrawTextW(mem, L"2048 (3x3)", -1, &trc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    wchar_t sbuf[64];
    swprintf(sbuf, 64, L"得分: %lld", score);
    SelectObject(mem, sf);
    RECT src = {MARGIN, 50, WIN_W - MARGIN, 74};
    DrawTextW(mem, sbuf, -1, &src, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    HFONT hf = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Microsoft YaHei"));
    SelectObject(mem, hf);
    RECT hrc = {MARGIN, WIN_H - 26, WIN_W - MARGIN, WIN_H - 6};
    DrawTextW(mem, L"方向键/WASD 移动   R 重开   Esc 退出", -1, &hrc,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SelectObject(mem, oldF);
    DeleteObject(tf);
    DeleteObject(sf);
    DeleteObject(hf);

    BitBlt(hdc, 0, 0, WIN_W, WIN_H, mem, 0, 0, SRCCOPY);
    SelectObject(mem, oldBmp);
    DeleteObject(bmp);
    DeleteDC(mem);
    EndPaint(hwnd, &ps);
}

void afterMove() {
    addTile();
    InvalidateRect(g_hwnd, NULL, FALSE);
    if (!winShown && hasWon()) {
        winShown = true;
        MessageBoxW(g_hwnd, L"恭喜! 你合成了 2048! 继续挑战更高分吧~",
                    L"胜利", MB_ICONINFORMATION | MB_OK);
    }
    if (!canMove()) {
        gameOver = true;
        wchar_t buf[128];
        swprintf(buf, 128, L"游戏结束! 最终得分: %lld\n\n按 R 重新开始, 或按 Esc 退出。", score);
        MessageBoxW(g_hwnd, buf, L"游戏结束", MB_ICONINFORMATION | MB_OK);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_PAINT:
            drawBoard(hwnd);
            return 0;
        case WM_KEYDOWN:
            if (gameOver) {
                if (wp == 'R') { init(); addTile(); addTile(); InvalidateRect(hwnd, NULL, FALSE); }
                else if (wp == VK_ESCAPE) DestroyWindow(hwnd);
                return 0;
            }
            switch (wp) {
                case VK_LEFT: case 'A': if (doMove(0)) afterMove(); return 0;
                case VK_RIGHT: case 'D': if (doMove(1)) afterMove(); return 0;
                case VK_UP: case 'W': if (doMove(2)) afterMove(); return 0;
                case VK_DOWN: case 'S': if (doMove(3)) afterMove(); return 0;
                case 'R': init(); addTile(); addTile(); InvalidateRect(hwnd, NULL, FALSE); return 0;
                case VK_ESCAPE: DestroyWindow(hwnd); return 0;
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    srand((unsigned)time(NULL));

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"Game2048";
    RegisterClassW(&wc);

    RECT rc = {0, 0, WIN_W, WIN_H};
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
    g_hwnd = CreateWindowW(L"Game2048", L"2048 (3x3)",
                           WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                           CW_USEDEFAULT, CW_USEDEFAULT,
                           rc.right - rc.left, rc.bottom - rc.top,
                           NULL, NULL, hInst, NULL);
    if (!g_hwnd) return 1;

    init();
    addTile();
    addTile();

    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}
