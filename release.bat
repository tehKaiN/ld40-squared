@echo off
mkdir game
copy ld40 game
xcopy data game\data /e /i /h
mkdir game\s
echo ld40 > game\s\startup-sequence
exe2adf ld40 -l "ld40-squared" -a "ld40-squared.adf" -d game
rmdir game /s /q