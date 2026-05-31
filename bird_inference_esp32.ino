// ==============================================================================
// 🐦 ESP32 Edge-AI Bird Sound Classifier (Native TensorFlow Lite)
// ==============================================================================

// 1. กระตุ้นให้ Arduino IDE ดึงโฟลเดอร์ไลบรารีเข้ามาร่วมคอมไพล์
#include <TensorFlowLite_ESP32.h>

// 2. เรียกใช้ไฟล์ของระบบ TensorFlow Lite (Native)
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

// 3. นำเข้าไฟล์องค์ประกอบของเรา
#include "bird_edge_1d_cnn.h"
#include "esp32_mfcc_samples.h"
#include "audio_processor.h"

// ------------------------------------------------------------------------------
// การตั้งค่าหน่วยความจำและตัวแปรระบบ
// ------------------------------------------------------------------------------
// กำหนดขนาดหน่วยความจำ (90KB สำหรับ Float32 หรือปรับเหลือ 32KB ถ้าใช้โมเดล INT8)
const int kTensorArenaSize = 90 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

const tflite::Model* tflite_model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
tflite::ErrorReporter* error_reporter = nullptr;
TfLiteTensor* input_tensor = nullptr;
TfLiteTensor* output_tensor = nullptr;

// ------------------------------------------------------------------------------
// ข้อมูลคลาสและเฉลยสำหรับการทดสอบ
// ------------------------------------------------------------------------------
const char* BIRD_CLASSES[] = {
    "Great Tinamou", 
    "Solitary Tinamou", 
    "White-throated Tinamou", 
    "Grey Tinamou", 
    "Small-billed Tinamou"
};

// เฉลยที่ถูกต้องสำหรับไฟล์ทดสอบทั้ง 10 ไฟล์ (ต้องตรงกับที่สกัดมาจาก Colab)
const char* GROUND_TRUTH[] = {
    "Great Tinamou",           // ไฟล์ 1
    "Solitary Tinamou",        // ไฟล์ 2
    "White-throated Tinamou",  // ไฟล์ 3
    "Grey Tinamou",            // ไฟล์ 4
    "Small-billed Tinamou",    // ไฟล์ 5
    "Unknown (เสียงแปลกปลอม)",  // ไฟล์ 6
    "Unknown (เสียงแปลกปลอม)",  // ไฟล์ 7
    "Unknown (เสียงแปลกปลอม)",  // ไฟล์ 8
    "Unknown (เสียงแปลกปลอม)",  // ไฟล์ 9
    "Unknown (เสียงแปลกปลอม)"   // ไฟล์ 10
};

// ==============================================================================
// ฟังก์ชัน SETUP
// ==============================================================================
void setup() {
    Serial.begin(115200);
    while (!Serial);
    
    Serial.println("\n==================================================");
    Serial.println("🏆 ESP32 Edge-AI Bird Sound (Native TensorFlow)");
    Serial.println("==================================================");

    // 1. ตั้งค่าตัวรายงานข้อผิดพลาด
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    // 2. โหลดไฟล์โมเดล Hex Array
    tflite_model = tflite::GetModel(bird_edge_1d_cnn_tflite);
    if (tflite_model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter, "❌ Error: เวอร์ชันของโมเดลไม่ตรงกับไลบรารี!");
        while (1);
    }

    // 3. โหลดชุดคำสั่งคณิตศาสตร์ทั้งหมดเข้าบอร์ด
    static tflite::AllOpsResolver resolver;

    // 4. สร้าง Interpreter สำหรับประมวลผล
    static tflite::MicroInterpreter static_interpreter(
        tflite_model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    // 5. จัดสรรหน่วยความจำ
    if (interpreter->AllocateTensors() != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "❌ Error: จัดสรรหน่วยความจำแรม (Arena) ล้มเหลว!");
        while (1);
    }

    // 6. ชี้เป้าหมายไปที่ช่องรับข้อมูลและช่องส่งข้อมูลออก
    input_tensor = interpreter->input(0);
    output_tensor = interpreter->output(0);

    Serial.println("[SUCCESS] Native TFLite Initialization Complete.");
    Serial.print("[INFO] จำนวนคลังไฟล์เสียงทดสอบในระบบ: ");
    Serial.println(TOTAL_AUDIO_SAMPLES);
    Serial.println("--------------------------------------------------\n");
}

// ==============================================================================
// ฟังก์ชัน LOOP (ทำงานวนซ้ำ)
// ==============================================================================
void loop() {
    // วนลูปตามจำนวนไฟล์เสียง (จำกัดสูงสุด 10 ไฟล์เพื่อไม่ให้ Array เฉลย Error)
    int num_tests = (TOTAL_AUDIO_SAMPLES > 10) ? 10 : TOTAL_AUDIO_SAMPLES;

    for (int i = 0; i < num_tests; i++) {
        Serial.print("🎬 [TEST SAMPLE ");
        Serial.print(i + 1);
        Serial.println("]");

        int current_audio_len = ALL_AUDIO_LEN[i];
        const float* current_audio_ptr = ALL_AUDIO_DATA[i];

        // 1. โหลด MFCC ของจริงเข้า Tensor รับข้อมูลโดยตรง
        for (int k = 0; k < 2600; k++) {
                    input_tensor->data.f[k] = current_audio_ptr[k];
                }

        // 2. จับเวลาและสั่งโมเดลประมวลผล (Invoke)
        unsigned long start_time = micros();
        
        if (interpreter->Invoke() != kTfLiteOk) {
            Serial.println("❌ Error: การทำนายผลล้มเหลว!");
            continue;
        }
        
        unsigned long end_time = micros();

        // 3. ค้นหาคลาสที่มีคะแนนสูงสุดจาก Output Tensor
        int best_class_idx = 0;
        float max_confidence = -1.0f;

        for (int c = 0; c < 5; c++) {
            // ดึงค่าความมั่นใจ (Float32) ออกมา
            float confidence = output_tensor->data.f[c];
            if (confidence > max_confidence) {
                max_confidence = confidence;
                best_class_idx = c;
            }
        }

        // 4. แสดงผลลัพธ์และเปรียบเทียบกับเฉลย
        Serial.print(" 📝 เฉลยที่ถูกต้อง: ");
        Serial.println(GROUND_TRUTH[i]); 

        Serial.print(" 🎯 AI ทำนายว่า:   ");
        
        // ถ้าระดับความมั่นใจต่ำกว่า 60% (0.60) ให้ถือว่าเป็นเสียงที่ไม่รู้จัก
        if (max_confidence < 0.60f) {
            Serial.print("Unknown (เสียงแปลกปลอม)");
        } else {
            Serial.print(BIRD_CLASSES[best_class_idx]);
        }
        
        Serial.print("  (มั่นใจ: ");
        Serial.print(max_confidence * 100.0f, 2);
        Serial.println("%)");

        Serial.print(" ⚡ เวลาประมวลผล: ");
        Serial.print((end_time - start_time) / 1000.0f, 2);
        Serial.println(" ms");
        Serial.println("--------------------------------------------------");

        delay(3000); // หน่วงเวลา 3 วินาที เพื่อให้อ่านผลทัน
    }

    Serial.println("🏁 ทดสอบครบเซ็ตแล้ว ระบบจะเริ่มต้นรอบใหม่ใน 5 วินาที...\n");
    delay(5000);
}