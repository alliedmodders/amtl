@echo on
SET PYTHON=c:\python33
SET PATH=%PYTHON%;%PYTHON%\Scripts;%PATH%
git submodule update --init
git clone https://github.com/alliedmodders/ambuild ambuild
pip install ./ambuild
chdir /D "%APPVEYOR_BUILD_FOLDER%"
