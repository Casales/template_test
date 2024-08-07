@echo off
if "%~3"=="" (
    echo Usage: %0 ^<repo_url^> ^<target_folder^> ^<tag^>
    exit /b 255
)

set original_path=%cd%
set repo_url=%1
set target_folder=%2
set tag=%3

:: Clone the repository
:: Check if the target folder exists
if not exist %target_folder% (
	:: echo Cloning %repo_url% into %target_folder%
	git clone %repo_url% %target_folder%
	:: Check if the clone was successful
	if %errorlevel% neq 0 (
		echo Error: Failed to clone %repo_url%.
		exit /b 1
	)
)

:: switch to target folder for checkout (should always succeed but if not return error)
cd %target_folder% || exit /b 255

:: Checkout the specified tag
:: echo Checking out %tag%(of %repo_url% in %target_folder%)
git fetch
git checkout %tag%

:: Check if checkout was successfull
if %errorlevel% neq 0 (
	echo Error: Failed to checkout tag %tag%.
	cd %original_path%
	exit /b 255
)

set needToPull=false
if "%tag%" == "main" (set needToPull=true)
if %tag% == "main" (set needToPull=true)
if %needToPull% == true (
	echo pulling changes from origin (main branch)
	git pull
	if %errorlevel% neq 0 (
		echo Warning: Failed to pull from remote.
	)
)

cd %original_path%

:: Exit the script
exit /b 0

