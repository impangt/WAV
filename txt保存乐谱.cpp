#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <regex>

using namespace std;

// ===================== 核心配置 =====================
const int SAMPLE_RATE = 44100;    // 采样率
const double BPM = 120.0;         // 速度：1拍=0.5秒
const double BASE_FREQ = 261.63;  // 中音1(C4)
const string SCORE_FILE = "music_score.txt";

// 中音1-7 频率倍率
const double notes[] = {
    0.0,    // 0 休止符
    1.0,    // 1
    9.0/8,  // 2
    5.0/4,  // 3
    4.0/3,  // 4
    3.0/2,  // 5
    5.0/3,  // 6
    15.0/8  // 7
};

// 生成单个音符波形
vector<int16_t> generateTone(double freq, double duration) {
    vector<int16_t> samples;
    int total = static_cast<int>(SAMPLE_RATE * duration);
    if (freq <= 0) {
        samples.resize(total, 0);
        return samples;
    }
    for (int i = 0; i < total; ++i) {
        double t = (double)i / SAMPLE_RATE;
        double v = sin(2 * M_PI * freq * t) * 0.5;
        samples.push_back((int16_t)(v * 32767));
    }
    return samples;
}

// ===================== 【补上缺失的函数】音符频率计算 =====================
double getNoteFreq(char c, int octave) {
    if (c >= '1' && c <= '7') {
        int n = c - '0';
        return BASE_FREQ * notes[n] * pow(2.0, octave - 4);
    }
    return 0.0;
}

// ===================== 【安全定稿】精准判断：不吞括号 + 不死循环 =====================
vector<int16_t> parseScore(const string& score) {
    vector<int16_t> audio;
    double beat = 60.0 / BPM;
    size_t i = 0;
    size_t len = score.size();

    while (i < len) {
        // 跳过空格、小节线、换行等分隔符
        while (i < len && (score[i] == ' ' || score[i] == '|' || score[i] == '\n' || score[i] == '\r')) {
            i++;
        }
        if (i >= len) break;

        // 每个音符独立重置八度，杜绝污染
        int octave = 4;
        char note = 0;
        bool hasNote = false;

        // 读取前置八度符号 ( ) '
        while (i < len) {
            char c = score[i];
            if (c == '(') { octave--; i++; }
            else if (c == ')') { octave++; i++; }
            else if (c == '\'') { octave++; i++; }
            else break;
        }

        // 读取核心音符 1-7
        if (i < len && score[i] >= '1' && score[i] <= '7') {
            note = score[i];
            hasNote = true;
            i++;
        }

        // 读取后置高音符号 '
        while (i < len && score[i] == '\'') {
            octave++;
            i++;
        }

        // 读取延长音 -（支持空格）
        double dur = 1.0;
        while (i < len && score[i] == ' ') i++;
        if (i < len && score[i] == '-') {
            dur = 2.0;
            i++;
        }

        // ===================== 【核心精准修复】唯一修改点 =====================
        if (hasNote) {
            // 有效音符：生成音频
            double freq = getNoteFreq(note, octave);
            auto wave = generateTone(freq, dur * beat);
            audio.insert(audio.end(), wave.begin(), wave.end());
        } else {
            // 【关键判断】仅跳过 真正无效的字符，合法符号( )' 绝不跳过！
            char currentChar = score[i];
            if (!(currentChar == '(' || currentChar == ')' || currentChar == '\'')) {
                i++; // 只有无效字符才跳过
            }
            // 如果是 ( ) ' → 不做任何操作，下一轮循环正常处理
        }
    }
    return audio;
}

// ===================== WAV 文件头（每行注释） =====================
void writeWavHeader(ofstream& file, int dataSize) {
    char header[44] = {0};

    // RIFF 文件标识
    header[0] = 'R'; header[1] = 'I'; header[2] = 'F'; header[3] = 'F';
    // 文件总长度 = 36 + 音频数据长度
    int fileSize = 36 + dataSize;
    *(uint32_t*)(header + 4) = fileSize;
    // WAVE 格式标识
    header[8] = 'W'; header[9] = 'A'; header[10] = 'V'; header[11] = 'E';
    // fmt 子块标识
    header[12] = 'f'; header[13] = 'm'; header[14] = 't'; header[15] = ' ';
    // fmt 子块长度（固定16）
    *(uint32_t*)(header + 16) = 16;
    // 音频格式：PCM = 1
    *(uint16_t*)(header + 20) = 1;
    // 声道数：单声道
    *(uint16_t*)(header + 22) = 1;
    // 采样率
    *(uint32_t*)(header + 24) = SAMPLE_RATE;
    // 字节率 = 采样率 * 位深度/8 * 声道数
    *(uint32_t*)(header + 28) = SAMPLE_RATE * 2;
    // 块对齐
    *(uint16_t*)(header + 32) = 2;
    // 位深度：16bit
    *(uint16_t*)(header + 34) = 16;
    // data 子块标识
    header[36] = 'd'; header[37] = 'a'; header[38] = 't'; header[39] = 'a';
    // 音频数据长度
    *(uint32_t*)(header + 40) = dataSize;

    file.write(header, 44);
}

// 保存 WAV 音频文件
void saveWav(const string& filename, const vector<int16_t>& audio) {
    ofstream file(filename, ios::binary);
    int dataSize = audio.size() * sizeof(int16_t);
    writeWavHeader(file, dataSize);
    file.write((const char*)audio.data(), dataSize);
    file.close();
    cout << "✅ WAV 文件已生成：" << filename << endl;
}

// 从 TXT 读取乐谱
string readScoreFromTxt(const string& filename) {
    ifstream file(filename);
    string score;
    if (file) {
        getline(file, score);
        file.close();
        cout << "✅ 从 music_score.txt 读取乐谱成功" << endl;
    } else {
        cerr << "❌ 无法打开文件" << endl;
    }
    return score;
}

// 将乐谱写入 TXT（强制单行）
bool writeScoreToTxt(const string& filename, const string& score) {
    ofstream file(filename);
    if (!file) return false;
    string cleanScore = score;
    cleanScore.erase(remove(cleanScore.begin(), cleanScore.end(), '\n'), cleanScore.end());
    cleanScore.erase(remove(cleanScore.begin(), cleanScore.end(), '\r'), cleanScore.end());
    file << cleanScore;
    file.close();
    cout << "✅ 乐谱已保存到 music_score.txt" << endl;
    return true;
}

// ===================== 乐谱语法校验 =====================
bool validateScore(const string& score, string& errorMsg) {
    errorMsg.clear();
    if (score.empty()) {
        errorMsg = "乐谱不能为空！";
        return false;
    }
    // 合法字符校验
    regex validChars(R"([^1234567\|\-\(\)' ])");
    if (regex_search(score, validChars)) {
        errorMsg = "包含非法字符！仅允许：1234567 | - ( ) ' 空格";
        return false;
    }
    // 括号配对校验
    int bracketCount = 0;
    for (char c : score) {
        if (c == '(') bracketCount++;
        if (c == ')') bracketCount--;
        if (bracketCount < 0) {
            errorMsg = "右括号不能单独使用！格式：(1)";
            return false;
        }
    }
    if (bracketCount != 0) {
        errorMsg = "括号未闭合！格式：(1)";
        return false;
    }
    // 延长音校验
    for (size_t j = 0; j < score.size(); j++) {
        if (score[j] == '-') {
            int pos = (int)j - 1;
            while (pos > 0 && score[pos] == ' ') pos--;
            if (pos < 0 || !(score[pos] >= '1' && score[pos] <= '7' || score[pos] == '\'' || score[pos] == ')')) {
                errorMsg = "延长音 - 必须跟在音符后！";
                return false;
            }
        }
    }
    return true;
}

// 用户交互式输入乐谱
string getUserInputScore() {
    string score;
    cout << "\n=====================================\n";
    cout << "🎵 简谱输入规则（无BUG版）\n";
    cout << "中音：1 2 3      高音：1' 2' 3'\n";
    cout << "低音：(1) (2)    延长：5- 5'- (5)- 5 -\n";
    cout << "示例：1' 1' (5) (5)- 2 2' 3 3'- \n";
    cout << "=====================================\n";
    cout << "请输入乐谱：";
    getline(cin, score);
    // 清理首尾空格
    score.erase(score.begin(), find_if(score.begin(), score.end(), [](unsigned char ch) {
        return !isspace(ch);
    }));
    score.erase(find_if(score.rbegin(), score.rend(), [](unsigned char ch) {
        return !isspace(ch);
    }).base(), score.end());
    return score;
}

// ===================== 主函数 =====================
int main() {
    string userScore, errorMsg;
    bool isValid = false;

    // 循环输入直到合法
    while (!isValid) {
        userScore = getUserInputScore();
        isValid = validateScore(userScore, errorMsg);
        if (!isValid) {
            cerr << "\n❌ 错误：" << errorMsg << "，请重新输入！\n" << endl;
        }
    }

    // 写入→读取→生成音频
    writeScoreToTxt(SCORE_FILE, userScore);
    string scoreFromFile = readScoreFromTxt(SCORE_FILE);
    if (!scoreFromFile.empty()) {
        cout << "\n🎵 正在生成音频..." << endl;
        vector<int16_t> audioData = parseScore(scoreFromFile);
        saveWav("output.wav", audioData);
    }

    cout << "\n🎉 全部完成！播放 output.wav 试听\n";
    return 0;
}