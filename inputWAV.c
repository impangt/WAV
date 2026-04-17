#include <stdio.h>
#include <windows.h>
#include <conio.h>

// 单音按键 (A/S/D/F)
#define FREQ_A    349
#define FREQ_S    392
#define FREQ_D    440
#define FREQ_F    494

// 和弦按键 (Z/X/C/V)
#define CHORD_Z   261, 329, 392
#define CHORD_X   293, 349, 440
#define CHORD_C   329, 392, 494
#define CHORD_V   349, 440, 523

static int tone_duration = 150;

// 播放单个音调
void play_tone(DWORD freq) {
    Beep(freq, tone_duration);
}

// 播放和弦
void play_chord(DWORD f1, DWORD f2, DWORD f3) {
    Beep(f1, tone_duration);
    Beep(f2, tone_duration);
    Beep(f3, tone_duration);
}

// 同一行刷新显示（不换行）
void show_msg(const char* msg) {
    printf("\r%-50s", msg);  // \r 回到行首，刷新显示
}

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    printf("=============================================\n");
    printf(" A/S/D/F=单音  |  Z/X/C/V=和弦 | +/-=调时长\n");
    printf(" 当前时长：%d ms | 按 Q 退出\n", tone_duration);
    printf("=============================================\n");

    show_msg("请按键...");

    while (1) {
        if (_kbhit()) {
            char c = _getch();

            switch (c) {
                case 'a': case 'A':
                    show_msg("→ 按下 A");
                    play_tone(FREQ_A);
                    break;
                case 's': case 'S':
                    show_msg("→ 按下 S");
                    play_tone(FREQ_S);
                    break;
                case 'd': case 'D':
                    show_msg("→ 按下 D");
                    play_tone(FREQ_D);
                    break;
                case 'f': case 'F':
                    show_msg("→ 按下 F");
                    play_tone(FREQ_F);
                    break;

                case 'z': case 'Z':
                    show_msg("→ 按下 Z (和弦)");
                    play_chord(CHORD_Z);
                    break;
                case 'x': case 'X':
                    show_msg("→ 按下 X (和弦)");
                    play_chord(CHORD_X);
                    break;
                case 'c': case 'C':
                    show_msg("→ 按下 C (和弦)");
                    play_chord(CHORD_C);
                    break;
                case 'v': case 'V':
                    show_msg("→ 按下 V (和弦)");
                    play_chord(CHORD_V);
                    break;

                case '+':
                    tone_duration += 50;
                    if (tone_duration > 2000) tone_duration = 2000;
                    char buf[100];
                    sprintf(buf, "⏫ 时长：%d ms", tone_duration);
                    show_msg(buf);
                    break;
                case '-':
                    tone_duration -= 50;
                    if (tone_duration < 50) tone_duration = 50;
                    sprintf(buf, "⏬ 时长：%d ms", tone_duration);
                    show_msg(buf);
                    break;

                case 'q': case 'Q':
                    show_msg("👋 已退出");
                    printf("\n");
                    return 0;

                default:
                    break;
            }
        }
    }
    return 0;
}