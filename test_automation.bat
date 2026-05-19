@echo off
setlocal enabledelayedexpansion

:: Testing with a single setup and 5 rounds
set TOTAL=17
set F=0
set M=0
set N=0
set O=0
set S=17
set R=5

echo [TEST] Running Setup: F:%F% M:%M% N:%N% O:%O% S:%S% ^| Rounds: %R%

:: Mode 1.1 (Persistent)
echo [TEST] [Mode 1.1] Starting...
(
echo 1
echo -1
echo 1
echo %TOTAL%
echo %F%
echo %M%
echo %N%
echo %O%
echo %S%
echo y
echo %R%
echo 2
echo 0
) | game.exe
timeout /t 1 > nul

echo [TEST] Complete. Check the 'data' folder for new files.
pause
