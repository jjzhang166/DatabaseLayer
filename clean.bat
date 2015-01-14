REM Delete all temporary files 
@echo off

erase /A H *.suo
erase *.ncb
erase *.opt
erase *.ilk
erase *.sdf
erase *.ncb
erase *.opt
erase *.positions

rmdir /S /Q ipch
rmdir /S /Q Debug
rmdir /S /Q Release
rmdir /S /Q debug_shared
rmdir /S /Q release_shared
rmdir /S /Q debug_static_md
rmdir /S /Q release_static_md

del /q /s bin\*.*

cd lib
for /f "" %%i in ('dir /b /a-d *.*^|findstr /i /v "libmysql"') do (del /f /q %%i 2>nul)
cd ..

erase prj_win32\*.plg
rmdir /S /Q prj_win32\debug_shared
rmdir /S /Q prj_win32\debug_static_md
rmdir /S /Q prj_win32\release_shared
rmdir /S /Q prj_win32\release_static_md

erase Demos\CppSQLite3Demo\*.plg
rmdir /S /Q Demos\CppSQLite3Demo\debug_shared
rmdir /S /Q Demos\CppSQLite3Demo\debug_static_md
rmdir /S /Q Demos\CppSQLite3Demo\release_shared
rmdir /S /Q Demos\CppSQLite3Demo\release_static_md

erase Demos\CppMySQLDemo\*.plg
rmdir /S /Q Demos\CppMySQLDemo\debug_shared
rmdir /S /Q Demos\CppMySQLDemo\debug_static_md
rmdir /S /Q Demos\CppMySQLDemo\release_shared
rmdir /S /Q Demos\CppMySQLDemo\release_static_md

rmdir /S /Q Demos\CppSQLite3DemoMT\debug_shared
rmdir /S /Q Demos\CppSQLite3DemoMT\debug_static_md
rmdir /S /Q Demos\CppSQLite3DemoMT\release_shared
rmdir /S /Q Demos\CppSQLite3DemoMT\release_static_md

rmdir /S /Q Demos\CppMySQLDemoMT\debug_shared
rmdir /S /Q Demos\CppMySQLDemoMT\debug_static_md
rmdir /S /Q Demos\CppMySQLDemoMT\release_shared
rmdir /S /Q Demos\CppMySQLDemoMT\release_static_md

rmdir /S /Q Tests\obj\



