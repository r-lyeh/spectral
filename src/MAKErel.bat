call makeopt -Dmain=WinMain -DFLAGS=FLAGS_REL %*
del ..\*.ilk 1> nul 2> nul
del ..\*.pdb 1> nul 2> nul

where /q upx.exe && upx ..\spectral.exe
