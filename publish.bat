@echo off
REM One-click publish: commits your changes, pushes them, and tags a release so GitHub builds ReactionTrainer.exe.
cd /d "%~dp0"
echo === Reaction Trainer: publish to GitHub ===
echo.
git --version >nul 2>&1
if errorlevel 1 (
  echo Git is not installed. Install it from https://git-scm.com then run this again.
  pause
  exit /b 1
)

if not exist .git (
  echo First time here: setting up the repository...
  git init
  git branch -M main
  git remote add origin https://github.com/AimerMarcel/ReactionTrainer.git
)

REM identity used for these commits (only for this folder, not your whole PC)
git config user.name "AimerMarcel"
git config user.email "340798858+AimerMarcel@users.noreply.github.com"

set /p VER=Version for this release (example v1.1.0): 
set /p MSG=Short description of what changed: 
if "%MSG%"=="" set MSG=Update

git add .
git commit -m "%MSG%"
git push -u origin main
if errorlevel 1 (
  echo.
  echo Push failed. If a browser or login window appeared, sign in to GitHub and run this file again.
  pause
  exit /b 1
)

git tag %VER%
git push origin %VER%
if errorlevel 1 (
  echo.
  echo The tag %VER% could not be pushed. It probably already exists, so run this again with a new version number.
  pause
  exit /b 1
)

echo.
echo Done! GitHub is now building your exe. It takes a few minutes:
echo   Progress:  https://github.com/AimerMarcel/ReactionTrainer/actions
echo   Download:  https://github.com/AimerMarcel/ReactionTrainer/releases/latest
pause
