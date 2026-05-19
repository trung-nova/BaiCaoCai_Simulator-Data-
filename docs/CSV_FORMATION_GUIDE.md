# HƯỚNG DẪN DỮ LIỆU CSV (CSV DATA FORMATION GUIDE - CHI TIẾT)

Tài liệu này là bản đặc tả kỹ thuật đầy đủ nhất về mọi trường dữ liệu, thuật toán và logic hình thành dữ liệu trong các tệp CSV của hệ thống mô phỏng.

---

## 1. Cấu trúc tệp `*_swap_decisions.csv`

Đây là file quan trọng nhất, lưu trữ "nhật ký tư duy" của AI qua từng lượt đổi bài.

### 1.1. Các tham số định danh (Identifiers)
- **SessionPrefix**: Tên file CSV nay được gắn tiền tố (ví dụ `1.1_`, `3.0_`) kèm theo Timestamp chi tiết đến mili-giây (milliseconds) để tránh hiện tượng ghi đè file khi chạy tự động hóa hàng loạt bằng Batch Script.
- **Batch**: Mã định danh của đợt chạy (chỉ có ở Mode 3). Mỗi Batch đại diện cho một tổ hợp Archetype và Hạt giống khác nhau.
- **RoundID**: Số thứ tự ván bài trong một Batch.
- **PlayerName**: Tên định danh của người chơi (ví dụ: `AI_1`, `AI_2`).
- **SwapTurn**: Lượt đổi bài hiện tại (1, 2, hoặc 3 trong ván).

### 1.2. Các tham số trạng thái nội tại (Internal States)
- **Satisfaction ($S$)**: 
    - **Ý nghĩa**: Mức độ hạnh phúc với bài hiện tại.
    - **Thuật toán**: Logistic Sigmoid.
    - **Cận dưới/Cận trên**: 0.0 (tuyệt vọng) đến 1.0 (mãn nguyện).
- **Desire ($D$)**:
    - **Ý nghĩa**: Áp lực tâm lý buộc phải thay đổi.
    - **Hình thành**: Tích lũy theo thời gian. Nếu $S$ thấp kéo dài, $D$ sẽ tăng vọt, khiến AI đưa ra những quyết định liều lĩnh ở lượt 3.
- **Probability ($P_{final}$)**:
    - **Ý nghĩa**: Xác suất thực thi hành động Đổi bài.
    - **Cơ chế**: Là "máy xay" trộn lẫn giữa sự thông minh (Skill) và sự bốc đồng (Emotion).

### 1.3. Các tham số sự kiện (Event Data)
- **Decision**: Kết quả cuối cùng (0: Skip, 1: Trade, 2: Stay).
- **ScoreBefore**: Điểm bài ngay trước lượt đổi.
- **ScoreAfter**: Điểm bài sau khi hoán đổi (bằng `ScoreBefore` nếu không đổi).
- **CardOut / CardIn**: Tên lá bài được đẩy ra và nhận về. Hình thành từ việc chuyển đổi Enum `Rank` và `Suit` thành chuỗi văn bản (ví dụ: `Hearts 7`).

---

## 2. Chi tiết Thuật toán tính toán Tham số

Hệ thống không sử dụng số ngẫu nhiên đơn thuần mà sử dụng các mô hình toán học sau:

### 2.1. Thuật toán Dịch chuyển Điểm kỳ vọng (Midpoint Shift)
Điểm $x_0$ trong hàm Sigmoid không cố định mà biến thiên theo trạng thái tài chính:
- **Công thức**: $x_0 = (5.0 - 2 \cdot C) + (\gamma - 2.0)$
- **Logic**: 
    - Nếu AI đang thắng (`Confidence` $C$ tăng), $x_0$ giảm $\rightarrow$ AI dễ hài lòng với bài điểm thấp hơn (hiện tượng "thắng nên đánh chắc").
    - Nếu AI là người cẩn trọng (`Risk Factor` $\gamma$ cao), $x_0$ tăng $\rightarrow$ Đòi hỏi bài phải cực cao mới thấy hài lòng.

### 2.2. Thuật toán Tính toán Chỉ số Aggression (Sự hung hăng)
Quyết định tốc độ tăng trưởng của `Desire`:
- **Công thức**: $Aggression = 0.5 + (0.5 \cdot Confidence)$
- **Ý nghĩa**: Người tự tin cao sẽ có `Desire` tăng nhanh hơn sau mỗi lượt thất bại.

### 2.3. Thuật toán Quyết định Đổi bài (Dual-Process blending)
- **Lý trí ($P_{rational}$)**: Nếu Skill cao, $P_{rational} = 1.0 - S$.
- **Tâm lý ($P_{psych}$)**: $P_{psych} = 0.5(1-S) + 0.5D + (C \cdot S \cdot 0.3)$.
- **Tổng hòa**: $P_{final} = (Skill \cdot P_{rational}) + ((1 - Skill) \cdot P_{psych})$.
- **Kết quả**: Skill càng thấp, AI càng bị cảm xúc ($P_{psych}$) chi phối mạnh.

---

## 3. Tệp `*_bankroll_history.csv` (Biến động tài sản)

- **Round**: Số thứ tự ván.
- **Player_Balance**: Số dư tài khoản sau khi ván đấu kết thúc.
- **Thuật toán hình thành**: 
    1. Trừ tiền đặt cược (Ante) vào đầu ván.
    2. Nếu thắng: Cộng lại Ante + Phần thắng từ Nhà Cái.
    3. Nếu thua: Mất số tiền đã cược.

---

## 4. Tệp `*_ai_configs.csv` (Hồ sơ Nhân vật)

Lưu trữ các tham số "Gen" cố định:
- **Skill**: Được sinh ra bằng phân phối Gaussian xung quanh Mean của Archetype.
- **k**: Độ dốc Sigmoid (Shark: 2.5, Maniac: 0.8).
- **Gamma**: Hệ số rủi ro (Nit: 3.0, Shark: 1.5).

---

## 5. Logic hình thành "Lá bài đổi" (Card Trading Algorithm)

AI không đổi bài ngẫu nhiên, nó tuân theo thuật toán tối ưu hóa:
1. **Ưu tiên 1**: Đẩy các lá "Tây" (J, Q, K) ra nếu bài chưa đạt Ba Tiên, vì các lá này tính là 10 điểm, dễ làm bài bị "bù".
2. **Ưu tiên 2**: Đẩy lá có giá trị Rank thấp nhất ra để hy vọng nhận về lá cao hơn.
3. **Kết quả**: Được ghi vào cột `CardOut`.
