#!/bin/bash 2>nul || goto :windows
# run `sh MAKE.bat` to compile Spectral

# linux + osx -----------------------------------------------------------------
cd `dirname $0`

# update to latest ------------------------------------------------------------
git reset --hard HEAD~1
git pull

#echo $*

if [ "$(uname)" != "Darwin" ]; then

# setup (ArchLinux) ----------------------------------------------------------
[ ! -f ".setup" ] && sudo pacman -Sy && sudo pacman -Sy --noconfirm gcc && echo>.setup
# setup (Debian, Ubuntu, etc)
[ ! -f ".setup" ] && sudo apt-get -y update && sudo apt-get -y install gcc libx11-dev gcc libgl1-mesa-dev libasound2-dev mesa-common-dev  && echo>.setup

# compile --------------------------------------------------------------------
gcc src/app.c -I src -o ./Spectral.linux -O3 -DNDEBUG=3 -Wno-unused-result -Wno-unused-value -Wno-format -Wno-multichar -Wno-pointer-sign -Wno-string-plus-int -Wno-empty-body -lm -lX11 -lGL -lasound -lpthread $* || exit
upx -9 Spectral.linux
src/res/embed.linux Spectral.linux @SpectralEmBeDdEd
src/res/embed.linux Spectral.linux src/res/zxdb/Spectral.db.gz
src/res/embed.linux Spectral.linux @SpectralEmBeDdEd

fi

if [ "$(uname)" = "Darwin" ]; then

# compile --------------------------------------------------------------------
export SDKROOT=$(xcrun --show-sdk-path)
gcc -ObjC src/app.c -I src -o ./Spectral.osx -O3 -DNDEBUG=3 -Wno-unused-result -Wno-unused-value -Wno-format -Wno-multichar -Wno-pointer-sign -Wno-string-plus-int -Wno-empty-body -framework cocoa -framework iokit -framework CoreFoundation -framework CoreAudio -framework AudioToolbox -framework OpenGL -lm $* || exit
src/res/embed.osx Spectral.osx @SpectralEmBeDdEd
src/res/embed.osx Spectral.osx src/res/zxdb/Spectral.db.gz
src/res/embed.osx Spectral.osx @SpectralEmBeDdEd

# embed icon and make .app
test -d Spectral.app && rm -rf Spectral.app
sh src/res/img/appify.sh --script ./Spectral.osx --name "Spectral" --icons src/res/img/noto_1f47b_1k.icns

# make .dmg from .app
test -f Spectral.dmg && rm Spectral.dmg
sh src/res/img/create-dmg/create-dmg \
    --volname "Spectral" \
    --window-pos 200 120 \
    --window-size 800 400 \
    --icon Spectral 200 190 \
    --app-drop-link 600 185 \
    --background src/res/img/dmg_background0.png \
    --no-internet-enable Spectral.dmg Spectral.app

fi

exit

:windows

@echo off

for /f "tokens=1,* delims= " %%a in ("%*") do set ALL_FROM_2ND=%%b

if "%1"=="-h" (
    echo make [dev^|opt^|rel] [compiler-flags]
    exit /b
)

if "%1"=="test" (
    call make opt -DPRINTER -DTESTS %ALL_FROM_2ND% || goto error
    pause

    rem Z80------------------------------------------

    rem spectral "tests\z80\rak\z80test-1.0\z80ccf.tap"
    rem spectral "tests\z80\rak\z80test-1.0\z80doc.tap"
    rem spectral "tests\z80\rak\z80test-1.0\z80docflags.tap"
    rem spectral "tests\z80\rak\z80test-1.0\z80flags.tap"
    rem spectral "tests\z80\rak\z80test-1.0\z80full.tap"
    rem spectral "tests\z80\rak\z80test-1.0\z80memptr.tap"
    rem spectral "tests\z80\rak\z80test-1.2a\z80ccf.tap"
    rem spectral "tests\z80\rak\z80test-1.2a\z80ccfscr.tap"
    rem spectral "tests\z80\rak\z80test-1.2a\z80doc.tap"
    rem spectral "tests\z80\rak\z80test-1.2a\z80docflags.tap"
    rem spectral "tests\z80\rak\z80test-1.2a\z80flags.tap"
    rem spectral "tests\z80\rak\z80test-1.2a\z80full.tap"
    rem spectral "tests\z80\rak\z80test-1.2a\z80memptr.tap"
    rem spectral "tests\z80\woodster\z80tests.tap"
    rem spectral "tests\z80\zexall\zexall.tap"
    rem spectral "tests\z80\zexall\zexall2.tap"
    rem spectral "tests\z80\zexall\zexbit.tap"
    rem spectral "tests\z80\zexall\zexdoc.tap"
    rem spectral "tests\z80\zexall\zexfix.tap"

    rem spectral "tests\z80\redcode\Z80 XCF Flavor.tap"

    rem https://github.com/redcode/Z80/wiki/Z80-Block-Flags-Test
    rem spectral "tests\z80\z80bltst.tap"

    rem https://github.com/redcode/Z80/wiki/Z80-INT-Skip
    spectral "tests\z80\int_skip.tap"

    exit /b
)

if "%1"=="tidy" (
    del *.obj
    del *.exe
    del *.pdb
    del *.ilk
    del *.zip
    del src\res\zxdb\*.db
    del src\res\zxdb\*.exe
    del src\res\zxdb\*.sqlite
    exit /b
)

if "%1"=="dev" (
    call make nil /Zi %ALL_FROM_2ND% || goto error
    src\res\embed Spectral.exe @SpectralEmBeDdEd
    src\res\embed Spectral.exe src\res\zxdb\Spectral.db.gz
    src\res\embed Spectral.exe @SpectralEmBeDdEd

    tasklist /fi "ImageName eq remedybg.exe" 2>NUL | find /I "exe">NUL || (where /q remedybg.exe && start remedybg -q -g Spectral.exe)

    exit /b
)

if "%1"=="opt" (
    rem do not use /O1 or /O2 below. ayumi drums will be broken in AfterBurner.dsk otherwise
    call make nil /Ox /MT /DNDEBUG /GL /GF /arch:AVX2 %ALL_FROM_2ND% || goto error
    where /q upx.exe && upx Spectral.exe
    src\res\embed Spectral.exe @SpectralEmBeDdEd
    copy /y Spectral.exe SpectralNoZXDB.exe
    src\res\embed Spectral.exe src\res\zxdb\Spectral.db.gz
    src\res\embed Spectral.exe @SpectralEmBeDdEd
    exit /b
)

if "%1"=="rel" (
    call make opt -Dmain=WinMain -DNDEBUG=3 %ALL_FROM_2ND% || goto error

    del *.ilk 1> nul 2> nul
    del *.pdb 1> nul 2> nul
    del *.obj 1>nul 2>nul
    del *.ilk 1>nul 2>nul
    del *.pdb 1>nul 2>nul

    exit /b 1
)

where /q cl.exe || call "%VS170COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" >nul 2>nul
where /q cl.exe || call "%VS160COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" >nul 2>nul
where /q cl.exe || call "%VS150COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" >nul 2>nul
where /q cl.exe || call "%VS140COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" >nul 2>nul
where /q cl.exe || call "%VS120COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" >nul 2>nul
where /q cl.exe || call "%ProgramFiles%/microsoft visual studio/2022/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" >nul 2>nul
where /q cl.exe || call "%ProgramFiles(x86)%/microsoft visual studio/2019/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" >nul 2>nul
where /q cl.exe || call "%ProgramFiles(x86)%/microsoft visual studio/2017/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" >nul 2>nul

@echo on
cl src\app.c src\sys_window.cc -I src /FeSpectral.exe %ALL_FROM_2ND% || goto error

@echo off
where /Q ResourceHacker.exe && ResourceHacker.exe -open Spectral.exe -save Spectral.exe -action addskip -res src\res\img\noto_1f47b.ico -mask ICONGROUP,MAINICON,0

exit /b 0

:error
@copy , ,, >nul
