@echo off
echo "CppMySQ build begin"

if defined VS100COMNTOOLS (call "%VS100COMNTOOLS%\vsvars32.bat")

devenv CppMySQL.vcxproj /Rebuild "release_shared" 
devenv CppMySQL.vcxproj /Rebuild "release_static_md" 

echo "CppMySQL build over"

rmdir /s /q .\release_shared
rmdir /s /q .\release_static_md


