#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

typedef int16_t i16;
typedef uint16_t u16;
typedef uint32_t u32;

typedef float f32;

void write_le_16(FILE *f, u16 n){
    fwrite(&n, sizeof(u16), 1, f);
}

void write_le_32(FILE *f, u32 n){
    fwrite(&n, sizeof(u32), 1, f);
}

#define WRITE_STR_LIT(f, s) fwrite((s), 1, sizeof(s)-1,f)

#define FREQ 44100
// #define DURATION 3

int main(void)
{
    FILE* f = fopen("try.wav", "wb");

    struct{
        f32 freq;
        f32 dur;
    } notes[] = {
       {392, 60.0f/76},
       {440, 60.0f/76},
       {294, 60.0f/114},
       {440, 60.0f/76},
       {494, 60.0f/76},
    };

    u32 num_notes = sizeof(notes) / sizeof(notes[0]);

    f32 duration = 0.0f;
    for(u32 i=0; i< num_notes; i++)
    {
        duration += notes[i].dur;
    }

    u32 num_sample = (u32)(duration * FREQ);
    u32 file_size = num_sample * sizeof(u16) + 44;

    WRITE_STR_LIT(f, "RIFF");//RIFF 的ID
    write_le_32(f,file_size - 8);//文件总大小
    WRITE_STR_LIT(f,"WAVE");//说明装了音频wave文件
    WRITE_STR_LIT(f, "fmt ");//参数区证明
    write_le_32(f,16);//fmt区块本身的大小
    write_le_16(f,1);//编码格式PCM
    write_le_16(f,1);//通道数
    write_le_32(f, FREQ);//采样率（一般是44100Hz
    write_le_32(f, FREQ * sizeof(u16));//采样率*声道*位深/8
    write_le_16(f, sizeof(u16));//声道*位深/8
    write_le_16(f, sizeof(u16) * 8);//每一位采样的比特率（16位）

    WRITE_STR_LIT(f, "data");//往后全是原始采样点
    //标记后面音频数据总长度（字节数）
    write_le_32(f, num_sample * sizeof(u16));

    u32 cur_note =  0;
    f32 cur_note_start = 0.0f;
    for(u32 i=0; i< num_sample ; i++)
    {//信号发生器
        f32 t = (f32)i / FREQ;
        f32 y = 0.0f;

        if(cur_note < num_notes)
        {
            y = 0.25f * sinf(t * notes[cur_note].freq * 2.0f * 3.1415926535f);

            if(t > cur_note_start + notes[cur_note].dur)
            {
                cur_note++;
                cur_note_start = t;
            }
        }
        
        i16 sample = (i16)(y * INT16_MAX);

        write_le_16(f, sample);
    }

    fclose(f);

    return 0;
}