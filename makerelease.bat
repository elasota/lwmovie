rmdir /S /Q lwmovie-release
mkdir lwmovie-release
mkdir lwmovie-release\external-sources
mkdir lwmovie-release\bin
mkdir lwmovie-release\bin\ffmpeg
mkdir lwmovie-release\lib
mkdir lwmovie-release\include
mkdir lwmovie-release\include\lwmovie
mkdir lwmovie-release\include\common
mkdir lwmovie-release\doc
call "D:\Low Speed Installs\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
msbuild.exe lwmovie.sln /t:Rebuild /p:Configuration=Release
msbuild.exe lwfe.sln /t:Rebuild /p:Configuration=Release

copy /Y Release\lwmovie.lib lwmovie-release\lib\lwmovie.lib

copy /Y lwmovie\lwmovie.h lwmovie-release\include\lwmovie\lwmovie.h
copy /Y lwmovie\lwmovie_cake.h lwmovie-release\include\lwmovie\lwmovie_cake.h
copy /Y lwmovie\lwmovie_cake_cppshims.h lwmovie-release\include\lwmovie\lwmovie_cake_cppshims.h
copy /Y lwmovie\lwmovie_cpp_shims.hpp lwmovie-release\include\lwmovie\lwmovie_cpp_shims.hpp
copy /Y lwmovie\lwmovie_api.h lwmovie-release\include\lwmovie\lwmovie_api.h
copy /Y lwmovie\lwmovie_external_types.h lwmovie-release\include\lwmovie\lwmovie_external_types.h
copy /Y lwmovie\lwmovie_common.h lwmovie-release\include\lwmovie\lwmovie_common.h
copy /Y lwmovie\lwmovie_attribs.h lwmovie-release\include\lwmovie\lwmovie_attribs.h

copy /Y Release\lwmovie.dll lwmovie-release\bin\lwmovie.dll
copy /Y Release\lwenctools.dll lwmovie-release\bin\lwenctools.dll
copy /Y Release\lwplay.exe lwmovie-release\bin\lwplay.exe
copy /Y Release\lwrerange.exe lwmovie-release\bin\lwrerange.exe
copy /Y Release\lwmux.exe lwmovie-release\bin\lwmux.exe
copy /Y Release\lwfe.exe "lwmovie-release\bin\lwmovie Encoder.exe"
copy /Y Release\SDL2.dll lwmovie-release\bin\SDL2.dll

copy /Y CommandLineOptions.txt lwmovie-release\doc\
copy /Y Cake_Documentation.pdf lwmovie-release\doc\
copy /Y architecture.pdf lwmovie-release\doc\
copy /Y LICENSE.txt lwmovie-release\doc\
copy /Y README-libjpeg.txt lwmovie-release\doc\
copy /Y Using_lwmovie.txt lwmovie-release\doc\

xcopy /Y /E Release\ffmpeg lwmovie-release\bin\ffmpeg
xcopy /Y /E external-sources lwmovie-release\external-sources

del deploy.7z
"C:\Program Files\7-zip\7z.exe" a -mx=9 deploy.7z lwmovie-release
rmdir /S /Q lwmovie-release
pause

