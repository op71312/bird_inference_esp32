#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSOR_H

#include <Arduino.h>
#include <math.h>

#define NUM_MFCC 13
#define TIME_STEPS 200

// ฟังก์ชันจำลองสกัดฟีเจอร์สัญญาณดิบ (Raw Signal) ไปเป็น Matrix ของ MFCC ขนาด (200, 13)
// ในระบบจริงจุดนี้จะถูกแทนที่ด้วยอัลกอริทึม FFT และ Mel-Filterbank (เช่น ไลบรารีอาร์ดูอีโน่ มัลติมีเดีย)
void extract_mfcc_on_board(const float* audio_data, int audio_len, float* output_buffer) {
    // โครงสร้าง Output Buffer มีมิติขนาดเป็น [TIME_STEPS * NUM_MFCC] หรือ 200 x 13 = 2600 ตัวแปร
    
    // ทำการสแกนและแปลงสัญญาณสถิติเชิงเวลา (จำลองค่าลายนิ้วมือเสียงนกอย่างง่ายเพื่อทดสอบ Pipeline)
    for (int t = 0; t < TIME_STEPS; t++) {
        int sample_idx = (t * audio_len) / TIME_STEPS;
        float signal_value = (sample_idx < audio_len) ? pgm_read_float(&audio_data[sample_idx]) : 0.0f;
        
        for (int f = 0; f < NUM_MFCC; f++) {
            // สกัดค่าองค์ประกอบความถี่จำลองด้วยฟังก์ชันตรีโกณมิติอ้างอิงตำแหน่งเวลา
            float pseudo_mfcc = sin(signal_value * (f + 1)) * cos(t * 0.05f);
            
            // บันทึกข้อมูลลงในหน่วยความจำระนาบเดียว (Flattened Array)
            output_buffer[t * NUM_MFCC + f] = pseudo_mfcc;
        }
    }
}

#endif // AUDIO_PROCESSOR_H