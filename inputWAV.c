#include <stdio.h>
#include <windows.h>
#include <conio.h>

// 4 个按键对应的频率（音调）
#define FREQ_A    349
#define FREQ_S    392
#define FREQ_D    440
#define FREQ_F    494

// 发声时长（毫秒）
#define TONE_DURATION 150

int main() {
    // ========== 关键：强制控制台使用 UTF-8 编码 ==========
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    // =====================================================

    printf("=========================================\n");
    printf("  按键 A / S / D / F 播放不同声音\n");
    printf("  按 Q 退出\n");
    printf("=========================================\n");

    while (1) {
        if (_kbhit()) {
            char c = _getch();

            switch (c) {
                case 'a': case 'A':
                    printf("→ A 发声\n");
                    Beep(FREQ_A, TONE_DURATION);
                    break;

                case 's': case 'S':
                    printf("→ S 发声\n");
                    Beep(FREQ_S, TONE_DURATION);
                    break;

                case 'd': case 'D':
                    printf("→ D 发声\n");
                    Beep(FREQ_D, TONE_DURATION);
                    break;

                case 'f': case 'F':
                    printf("→ F 发声\n");
                    Beep(FREQ_F, TONE_DURATION);
                    break;

                case 'q': case 'Q':
                    printf("退出程序\n");
                    return 0;

                default:
                    break;
            }
        }
    }
    return 0;
}