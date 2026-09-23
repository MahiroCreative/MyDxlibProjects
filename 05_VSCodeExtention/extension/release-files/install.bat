@echo off
chcp 65001 >nul
rem 上の chcp で文字コードを UTF-8 にしてから日本語を書く(この行より前に日本語を置かないこと)。
rem DxLib 開発環境(VSCode 拡張機能)のインストーラー。ダブルクリックで実行する。
rem
rem なぜ VSIX をダブルクリックしないのか:
rem   Visual Studio が入っている PC では .vsix が Visual Studio に関連付けられており、
rem   ダブルクリックすると Visual Studio のインストーラーが起動して失敗する。
rem   このファイルは、VSCode の code コマンドで確実にインストールする。
rem
setlocal
title DxLib 開発環境 インストール

echo ============================================================
echo  DxLib 開発環境(VSCode 拡張機能)のインストール
echo ============================================================
echo.

rem --- 1. このフォルダの VSIX を探す(通常は 1 つだけ。複数あれば名前が最後のもの) ---
set "VSIX="
for %%v in ("%~dp0dxlib-devenv-*.vsix") do set "VSIX=%%~fv"
if not defined VSIX goto :no_vsix

rem --- 2. VSCode の code コマンドを探す(PATH → 標準のインストール先の順) ---
set "CODE="
for /f "delims=" %%c in ('where code.cmd 2^>nul') do if not defined CODE set "CODE=%%c"
if not defined CODE if exist "%LOCALAPPDATA%\Programs\Microsoft VS Code\bin\code.cmd" set "CODE=%LOCALAPPDATA%\Programs\Microsoft VS Code\bin\code.cmd"
if not defined CODE if exist "%ProgramFiles%\Microsoft VS Code\bin\code.cmd" set "CODE=%ProgramFiles%\Microsoft VS Code\bin\code.cmd"
if not defined CODE if exist "%ProgramFiles(x86)%\Microsoft VS Code\bin\code.cmd" set "CODE=%ProgramFiles(x86)%\Microsoft VS Code\bin\code.cmd"
if not defined CODE goto :no_code

echo VSCode   : %CODE%
echo 拡張機能 : %VSIX%
echo.
echo インストールしています。しばらくお待ちください...
echo (C/C++ 拡張などの、一緒に必要な拡張機能もダウンロードします)
echo.

rem --- 3. インストール(--force: 同じバージョンが入っていても入れ直す) ---
call "%CODE%" --install-extension "%VSIX%" --force
if errorlevel 1 goto :failed

echo.
echo ------------------------------------------------------------
echo  インストールが完了しました。
echo.
echo  次にやること:
echo    1. VSCode を開いている場合は、いったん閉じて開き直す
echo    2. 左端のバーに DxLib のアイコンが出れば成功
echo    3. アイコンを押して、パネルの案内に従う
echo ------------------------------------------------------------
set "RC=0"
goto :end

:no_vsix
echo [エラー] dxlib-devenv-*.vsix が見つかりません。
echo.
echo  install.bat と同じフォルダに、拡張機能のファイル(.vsix)を置いてください。
echo  探した場所: %~dp0
set "RC=1"
goto :end

:no_code
echo [エラー] Visual Studio Code が見つかりません。
echo.
echo  先に VSCode をインストールしてください。
echo    https://code.visualstudio.com/
echo  インストールしたら、もう一度この install.bat をダブルクリックしてください。
set "RC=1"
goto :end

:failed
echo.
echo [エラー] インストールに失敗しました。
echo.
echo  次を確認してください。
echo    - インターネットに接続されているか(C/C++ 拡張などをダウンロードします)
echo    - 上に出ているメッセージ
echo  解決しない場合は、この画面の内容を講師に見せてください。
set "RC=1"
goto :end

:end
echo.
if not defined DXLIB_INSTALL_NOPAUSE pause
exit /b %RC%
