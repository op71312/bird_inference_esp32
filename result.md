## 📊 สรุปผลการทดสอบโมเดล Edge AI บน ESP32

จากการทดสอบด้วยไฟล์เสียงจริงจำนวน 10 ไฟล์ (ข้อมูลนก 5 สายพันธุ์ และข้อมูลเสียงแปลกปลอม 5 ไฟล์) ได้ผลลัพธ์ดังนี้:

| ลำดับที่ | เฉลยที่ถูกต้อง (Ground Truth) | AI ทำนายว่า | ความมั่นใจ (Confidence) | สถานะ |
| --- | --- | --- | --- | --- |
| 1 | Great Tinamou | Great Tinamou | 78.91% | ✅ ถูกต้อง |
| 2 | Solitary Tinamou | Solitary Tinamou | 83.27% | ✅ ถูกต้อง |
| 3 | White-throated Tinamou | Great Tinamou | 63.07% | ❌ ผิดพลาด |
| 4 | Grey Tinamou | Unknown | 39.73% | ⚠️ ปฏิเสธ |
| 5 | Small-billed Tinamou | Great Tinamou | 81.87% | ❌ ผิดพลาด |
| 6 | Unknown | Solitary Tinamou | 90.16% | ❌ ทายผิด (False Positive) |
| 7 | Unknown | Small-billed Tinamou | 96.85% | ❌ ทายผิด (False Positive) |
| 8 | Unknown | Solitary Tinamou | 85.57% | ❌ ทายผิด (False Positive) |
| 9 | Unknown | Great Tinamou | 94.48% | ❌ ทายผิด (False Positive) |
| 10 | Unknown | Great Tinamou | 92.43% | ❌ ทายผิด (False Positive) |

**หมายเหตุ:** เวลาประมวลผลเฉลี่ย (Inference Latency) อยู่ที่ประมาณ **1,765 ms** ต่อไฟล์

-