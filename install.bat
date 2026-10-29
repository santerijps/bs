@echo off
gcc -o %~dp0\build -O3 -s -march=native %~dp0\build.c
