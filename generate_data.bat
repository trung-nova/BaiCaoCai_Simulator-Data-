@echo off
setlocal enabledelayedexpansion
set "EXE=game.exe"

:: --- DANH SÁCH CÁC SETUP (Định dạng: "Tổng Fish Maniac Nit Normal Shark Rounds") ---
set "setups="
:: Bàn đồng nhất
set setups=%setups% "17 17 0 0 0 0 100000" "17 0 17 0 0 0 100000" "17 0 0 17 0 0 100000" "17 0 0 0 17 0 100000" "17 0 0 0 0 17 100000"
:: Các cặp Shark vs Others
set setups=%setups% "17 0 16 0 0 1 100000" "17 0 1 0 0 16 100000" "17 0 0 16 0 1 100000" "17 0 0 1 0 16 100000" "17 0 0 0 16 1 100000" "17 0 0 0 1 16 100000" "17 16 0 0 0 1 100000" "17 1 0 0 0 16 100000"
:: Các cặp Maniac vs Others
set setups=%setups% "17 0 1 16 0 0 100000" "17 0 16 1 0 0 100000" "17 0 1 0 16 0 100000" "17 0 16 0 1 0 100000" "17 1 16 0 0 0 100000" "17 16 1 0 0 0 100000"
:: Các cặp còn lại
set setups=%setups% "17 0 0 1 16 0 100000" "17 0 0 16 1 0 100000" "17 1 0 16 0 0 100000" "17 16 0 1 0 0 100000" "17 1 0 0 16 0 100000" "17 16 0 0 1 0 100000"
:: Ván hỗn hợp (Mixed)
set setups=%setups% "15 3 3 3 3 3 200000"

echo ===================================================
echo   BAI CAO CAI - AUTOMATED DATA GENERATOR (ROBUST)
echo ===================================================
echo [System] Starting simulation batches...

for %%S in (%setups%) do (
    for /f "tokens=1-7" %%A in (%%S) do (
        echo [Run] Setup: F:%%B M:%%C N:%%D O:%%E S:%%F ^| Rounds: %%G
        :: Chạy cả Submode 1 (Persistent) và Submode 2 (Reset)
        for %%M in (1 2) do (
            echo   - Mode 1.%%M starting...
            (echo 1 & echo -1 & echo %%M & echo %%A & echo %%B & echo %%C & echo %%D & echo %%E & echo %%F & echo y & echo %%G & echo 2 & echo 0) | !EXE! > nul
            timeout /t 1 > nul
        )
    )
)

echo ===================================================
echo   ALL SIMULATIONS COMPLETE
echo ===================================================
pause
