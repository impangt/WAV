#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdint>

// ===================== 配置参数 =====================
const int SAMPLE_RATE = 44100;    // 采样率
const double BPM = 120.0;         // 速度
const double BASE_FREQ = 261.63;   // 中音1 = C4 = 261.63Hz
// ====================================================

// 12平均律频率表（中音 1-7）
const double notes[] = {
    0.0,     // 0 休止符
    1.0,     // 1
    9.0/8,   // 2
    5.0/4,   // 3
    4.0/3,   // 4
    3.0/2,   // 5
    5.0/3,   // 6
    15.0/8   // 7
};

// 生成单个音符的波形
std::vector<int16_t> generateTone(double freq, double duration) {
    std::vector<int16_t> samples;
    int totalSamples = static_cast<int>(SAMPLE_RATE * duration);

    if (freq <= 0) {
        samples.resize(totalSamples, 0);
        return samples;
    }

    for (int i = 0; i < totalSamples; ++i) {
        double t = static_cast<double>(i) / SAMPLE_RATE;
        double val = sin(2 * M_PI * freq * t);
        val *= 0.5; // 音量控制
        samples.push_back(static_cast<int16_t>(val * 32767));
    }
    return samples;
}

// 解析简谱字符
double parseNote(char c, int octave) {
    if (c >= '1' && c <= '7') {
        int n = c - '0';
        return BASE_FREQ * notes[n] * pow(2.0, octave - 4);
    }
    return 0.0;
}

// 解析整个乐谱
std::vector<int16_t> parseScore(const std::string& score) {
    std::vector<int16_t> audio;
    double beatDuration = 60.0 / BPM;
    int octave = 4;  // 修正：默认中音组
    size_t i = 0;

    while (i < score.size()) {
        char c = score[i];

        if (c == '(') { octave--; i++; continue; }
        if (c == ')') { octave++; i++; continue; }
        if (c == '\'') { octave++; i++; continue; }
        if (c == '|' || c == ' ' || c == '\n' || c == '\r') { i++; continue; }

        double dur = 1.0;
        if (i+1 < score.size() && score[i+1] == '-') {
            dur = 2.0;
            i++;
        }

        double freq = parseNote(c, octave);
        auto wave = generateTone(freq, dur * beatDuration);
        audio.insert(audio.end(), wave.begin(), wave.end());

        i++;
    }
    return audio;
}

// 写入 WAV 文件头
void writeWavHeader(std::ofstream& file, int dataSize) {
    char header[44] = {0};

    // ChunkID
    header[0] = 'R'; header[1] = 'I'; header[2] = 'F'; header[3] = 'F';
    int fileSize = 36 + dataSize;
    *(uint32_t*)(header + 4) = fileSize;

    // Format
    header[8] = 'W'; header[9] = 'A'; header[10] = 'V'; header[11] = 'E';
    header[12] = 'f'; header[13] = 'm'; header[14] = 't'; header[15] = ' ';

    *(uint32_t*)(header + 16) = 16;
    *(uint16_t*)(header + 20) = 1;
    *(uint16_t*)(header + 22) = 1;
    *(uint32_t*)(header + 24) = SAMPLE_RATE;
    *(uint32_t*)(header + 28) = SAMPLE_RATE * 2;
    *(uint16_t*)(header + 32) = 2;
    *(uint16_t*)(header + 34) = 16;

    header[36] = 'd'; header[37] = 'a'; header[38] = 't'; header[39] = 'a';
    *(uint32_t*)(header + 40) = dataSize;

    file.write(header, 44);
}

// 保存 WAV 文件
void saveWav(const std::string& filename, const std::vector<int16_t>& audio) {
    std::ofstream file(filename, std::ios::binary);
    int dataSize = audio.size() * sizeof(int16_t);
    writeWavHeader(file, dataSize);
    file.write(reinterpret_cast<const char*>(audio.data()), dataSize);
    file.close();
    std::cout << "✅ WAV 文件已生成：" << filename << std::endl;
}

int main() {
    // ===================== 你的简谱写在这里！ =====================
    std::string score = R"(
        1 1 | 5 5 | 6 6 | 5 - |
        4 4 | 3 3 | 2 2 | 1 - |
        5 5 | 4 4 | 3 3 | 2 - |
        1 1 | 5 5 | 6 6 | 5 - |
        4 4 | 3 3 | 2 2 | 1 - ||
    )";
    // ============================================================

    std::cout << "正在解析简谱并生成音频..." << std::endl;
    auto audioData = parseScore(score);
    saveWav("output.wav", audioData);

    return 0;
}