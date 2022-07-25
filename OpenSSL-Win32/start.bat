@echo off
@set PATH=%PATH%;%~dp0bin

title Win32 OpenSSL Command Prompt
echo Win32 OpenSSL Command Prompt
echo.
openssl version -a
echo.

%SystemDrive%
cd %UserProfile%

cmd.exe /K
