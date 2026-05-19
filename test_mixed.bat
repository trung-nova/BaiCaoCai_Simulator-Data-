@echo off
setlocal enabledelayedexpansion

echo [TEST] Running Mixed Setup: 3 of each archetype ^| 15 Players ^| 5 Rounds
set TOTAL=15
set F=3
set M=3
set N=3
set O=3
set S=3
set R=5

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

echo [TEST] Mixed Setup Complete. Check 'data' folder.
pause
