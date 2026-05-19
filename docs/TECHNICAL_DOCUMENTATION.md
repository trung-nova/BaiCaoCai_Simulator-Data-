# TÀI LIỆU KỸ THUẬT TỔNG QUAN (TECHNICAL MASTER GUIDE)

Đây là tài liệu hướng dẫn cách biên dịch, vận hành và khai thác dữ liệu từ hệ thống mô phỏng Bài Cào Cái (Phiên bản Chuẩn hóa CSV).

## 1. Cấu trúc Tài liệu (Modular Docs)
Để tiện cho việc tra cứu, tài liệu được chia làm 4 phần:
1. [Luật chơi (Game Rules)](./GAME_RULES.md): Cách tính điểm và phân định thắng thua.
2. [Luồng vận hành (Game Flow)](./GAME_FLOW.md): Quy trình State Machine và các bước chia bài/đổi bài.
3. [Phân loại AI (Player Archetypes)](./PLAYER_ARCHETYPES.md): Giải thích các tham số toán học (Sigmoid, Tilt) và cơ chế Archetype động.
4. **Hướng dẫn Kỹ thuật (Tài liệu này)**: Build app và cấu trúc dữ liệu CSV.

---

## 2. Hướng dẫn Build & Chạy App

### 2.1. Yêu cầu hệ thống
- **Compiler**: GCC (MinGW cho Windows) hỗ trợ chuẩn C++17.
- **Thư viện**: Chỉ sử dụng thư viện chuẩn (Standard Library), không phụ thuộc bên ngoài.

### 2.2. Cách Build (Windows)
Sử dụng file batch để tự động hóa:
1. Chạy lệnh: `.\build_all.bat`
2. Hệ thống sẽ tạo ra file `game.exe`.

### 2.3. Cách Vận hành
Chạy file `game.exe`. Dữ liệu sẽ được sinh ra trong thư mục `data/`.
- **Cấu hình**: Chỉnh sửa `config.ini` để thay đổi hoặc thêm mới các loại cá tính AI (Archetypes).

---

## 3. Cấu trúc Dữ liệu Export (CSV)

Hệ thống tập trung xuất dữ liệu ra định dạng CSV để đảm bảo tính minh bạch và dễ dàng phân tích bằng Python (Pandas) hoặc Excel.

### 3.1. Danh mục các file báo cáo
Mỗi phiên chạy sẽ sinh ra bộ 4 file với định dạng tên `[Mode]_[Timestamp_ms]_[TênFile].csv` (ví dụ `1.1_20260519_121008_123_ai_configs.csv`):

1.  **`_ai_configs.csv`**: Lưu trữ hồ sơ tâm lý của từng AI (Skill, Confidence, Archetype).
2.  **`_rounds_summary.csv`**: Chi tiết kết quả từng ván đấu (Nhà cái, Tổng tiền, Số người thắng, Điểm số).
3.  **`_swap_decisions.csv`**: Ghi lại mọi quyết định đổi bài, bao gồm chỉ số Thỏa mãn (S) và Thèm muốn (D) tại thời điểm đó.
4.  **`_bankroll_history.csv`**: Biến động tài sản của tất cả người chơi qua từng ván.

---

## 4. Gợi ý Analysis Recipe (Python)

Dữ liệu CSV có thể được nạp vào Pandas cực kỳ đơn giản:

```python
import pandas as pd
import glob

# Lấy file swap gần nhất
latest_swap = glob.glob('data/*_swap_decisions.csv')[-1]
df_swaps = pd.read_csv(latest_swap)

# Tính xác suất đổi bài trung bình của nhóm SHARK
shark_trades = df_swaps[df_swaps['PlayerName'].str.contains('SHARK')]
avg_prob = shark_trades['Probability'].mean()

print(f"Average Decision Probability for Sharks: {avg_prob:.2f}")
```

---

## 5. Đảm bảo Chất lượng (Quality Assurance & Testing)

### 5.1. Hệ thống Kiểm thử (Automated Testing)
1. **Unit Testing**: `tests/test_logic.cpp` kiểm tra logic Bài Cào và "Ba Tiên".
2. **Integration Test**: `tests/system_test_suite.py` giả lập 20 ván chơi và kiểm tra tính toàn vẹn của các file CSV xuất ra.

### 5.2. Seeding phân cấp
Hệ thống sử dụng Master Seed để tạo ra Local Seeds cho từng AI, đảm bảo mọi ván đấu và hành vi tâm lý đều có thể **tái lập 100%** nếu dùng chung một Seed ban đầu.
