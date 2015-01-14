@echo off
echo "CppSQLite3 build begin"

if defined VS100COMNTOOLS (call "%VS100COMNTOOLS%\vsvars32.bat")

devenv CppSQLite3.vcxproj /Rebuild "release_shared" 
devenv CppSQLite3.vcxproj /Rebuild "release_static_md" 

echo "CppSQLite3 build over"


rmdir /s /q .\release_shared
rmdir /s /q .\release_static_md



