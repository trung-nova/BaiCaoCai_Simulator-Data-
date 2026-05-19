# HƯỚNG DẪN QUY TRÌNH MÔ PHỎNG CHI TIẾT (SIMULATION WORKFLOW GUIDE)

Tài liệu này tổng hợp toàn bộ quy trình từ thiết lập, cấu hình đến vận hành và phân tích dữ liệu cho hệ thống mô phỏng Bài Cào Cái v3.0.

---

## 1. Chuẩn bị Hệ thống (System Setup)
Trước khi bắt đầu, hãy đảm bảo máy tính của bạn đã cài đặt:
- **Compiler**: `g++` hỗ trợ chuẩn C++17 (thường đi kèm với MinGW-w64 trên Windows).
- **SQLite3**: Thư viện đã được đính kèm sẵn trong mã nguồn (`sqlite3.c`, `sqlite3.h`).
- **Terminal**: Command Prompt (CMD) hoặc PowerShell trên Windows.

---

## 2. Cấu hình Tham số (Configuration)
Trước khi chạy mô phỏng, bạn có thể tinh chỉnh hành vi của AI trong file [**config.ini**](file:///d:/T%C3%A0i%20li%E1%BB%87u%20h%E1%BB%8Dc%20t%E1%BA%ADp/OOP/config.ini):

- **[GLOBAL]**:
    - `seed`: Thiết lập số cụ thể để tái lập kết quả (deterministic) hoặc `-1` để chạy ngẫu nhiên.
    - `enable_tilt`: `true/false` để bật/tắt trạng thái tâm lý bộc phát (TILT) của AI.
- **[SHARK / MANIAC / NIT]**:
    - `k`, `gamma`: Tham số hàm Sigmoid điều khiển độ nhạy bén và rủi ro.
    - `greed_threshold`: Ngưỡng tham vọng đổi bài.
    - `min_skill / max_skill`: Khoảng kỹ năng của từng nhóm nhân vật.

---

## 3. Quy trình Vận hành (Execution Workflow)

### Bước 1: Biên dịch (Compile)
Mở Terminal tại thư mục gốc và chạy file batch:
```powershell
.\build_all.bat
```
Lệnh này sẽ tạo ra file `game_sql.exe` (phiên bản có hỗ trợ SQLite) và `game.exe`.

### Bước 2: Khởi chạy và Chọn chế độ
Chạy file `game_sql.exe`. Hệ thống sẽ cung cấp 4 chế độ chính:

1.  **Standard Simulation**: Chế độ nghiên cứu chính. Cho phép chạy hàng triệu ván đấu liên tục để thu thập dữ liệu thống kê.
2.  **Interactive**: Chế độ chơi thử nghiệm. Bạn có thể nhập tên và trực tiếp đấu với các AI để kiểm chứng hành vi.
3.  **Random (Mode 3)**: Chế độ Big Data. Hệ thống tự động tạo các "Batch" với số lượng người chơi và kỹ năng ngẫu nhiên để quét diện rộng.
4.  **Log Mode**: Chế độ gỡ lỗi. Hiển thị chi tiết từng bước tính toán xác suất của AI trong ván đấu.

### Bước 3: Thiết lập thông số chạy
Sau khi chọn chế độ (ví dụ Mode 1), bạn sẽ nhập:
- **Tổng số người chơi (Total Players)**: Số lượng người tham gia ván bài (từ 2 đến 17).
- **Số lượng cụ thể từng nhóm (Archetype Counts)**: Lần lượt nhập số lượng người chơi thuộc nhóm Fish, Maniac, Nit, Other, Shark. Tổng số lượng nhập vào phải bằng "Tổng số người chơi" ở trên.
- **Tính năng Auto Export**: Chọn (y/n) để tự động xuất dữ liệu sau khi kết thúc.
- **Số ván đấu (Rounds)**: Nhập số lượng ván muốn mô phỏng liên tục.

### Bước 4: Tự động hóa với Batch Script (Mới)
Hệ thống cung cấp các file script `.bat` để chạy mô phỏng tự động theo kịch bản:
- `generate_data.bat`: Chạy tự động hàng loạt các kịch bản với đủ loại đội hình (đồng nhất, cặp đối kháng, hỗn hợp) liên tục.
- `test_automation.bat`: Chạy kiểm tra nhanh 1 kịch bản 5 ván.
- `test_mixed.bat`: Chạy cấu hình hỗn hợp các Archetype để kiểm tra.

---

## 4. Dữ liệu Đầu ra (Output Data)
Sau khi kết thúc (hoặc trong quá trình chạy nếu bật `Auto Export`), dữ liệu sẽ được lưu tại thư mục `data/`:

- **simulation_results.db**: Cơ sở dữ liệu SQLite chứa toàn bộ lịch sử ván đấu và hành vi đổi bài chi tiết.
- **summary_report.txt**: Bản tóm tắt kết quả cuối cùng (Win rate, ROI, Bankruptcy rate).
- **tilt_logs.csv**: Ghi lại các thời điểm AI rơi vào trạng thái "vỡ trận" (Tilt).
- **ai_configs.csv**: Lưu lại thông số kỹ năng/tâm lý của từng AI trong phiên chạy đó.

---

## 5. Kiểm tra và Xác minh (Verification)
Để đảm bảo logic mô phỏng chính xác (đặc biệt là sau khi chỉnh sửa mã nguồn):
1.  Chạy `.\build_tests.bat`.
2.  Thực hiện `test_logic.exe` để kiểm tra các unit test về tính điểm và luật chơi.
3.  Quan sát `system_test_suite.py` để xác minh việc xuất file dữ liệu có lỗi hay không.

---
> [!TIP]
> Để có kết quả nghiên cứu tốt nhất, hãy chạy ít nhất **100,000 ván đấu** trong chế độ **Standard Simulation** với `enable_tilt=true` để thấy rõ sự khác biệt về ROI giữa các nhóm AI.
