@echo on
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86
mkdir opt32 && cd opt32
python ..\configure.py --enable-optimize
ambuild
cd ..
opt32\dist\testrunner.exe
