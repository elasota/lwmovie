rmdir /S /Q deploy
mkdir deploy
mkdir deploy\external-sources
mkdir deploy\bin
mkdir deploy\bin\ffmpeg
mkdir deploy\lib
mkdir deploy\include
mkdir deploy\include\lwmovie
mkdir deploy\include\common
call "D:\Low Speed Installs\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
msbuild.exe lwmovie.sln /t:Rebuild /p:Configuration=Release
msbuild.exe lwfe.sln /t:Rebuild /p:Configuration=Release

copy /Y Release\lwmovie.lib deploy\lib\lwmovie.lib

copy /Y lwmovie\lwmovie.h deploy\include\lwmovie\lwmovie.h
copy /Y lwmovie\lwmovie_cake.h deploy\include\lwmovie\lwmovie_cake.h
copy /Y lwmovie\lwmovie_cake_cppshims.h deploy\include\lwmovie\lwmovie_cake_cppshims.h
copy /Y lwmovie\lwmovie_cpp_shims.hpp deploy\include\lwmovie\lwmovie_cpp_shims.hpp
copy /Y lwmovie\lwmovie_api.h deploy\include\lwmovie\lwmovie_api.h
copy /Y lwmovie\lwmovie_external_types.h deploy\include\lwmovie\lwmovie_external_types.h
copy /Y lwmovie\lwmovie_common.h deploy\include\lwmovie\lwmovie_common.h
copy /Y lwmovie\lwmovie_attribs.h deploy\include\lwmovie\lwmovie_attribs.h

copy /Y Release\lwmovie.dll deploy\bin\lwmovie.dll
copy /Y Release\lwenctools.dll deploy\bin\lwenctools.dll
copy /Y Release\lwplay.exe deploy\bin\lwplay.exe
copy /Y Release\lwrerange.exe deploy\bin\lwrerange.exe
copy /Y Release\lwmux.exe deploy\bin\lwmux.exe
copy /Y Release\lwfe.exe deploy\bin\lwfe.exe
copy /Y Release\SDL2.dll deploy\bin\SDL2.dll
xcopy /Y /E Release\ffmpeg deploy\bin\ffmpeg
xcopy /Y /E external-sources deploy\external-sources

del deploy.7z
"C:\Program Files\7-zip\7z.exe" a -mx=9 deploy.7z deploy
rmdir /S /Q deploy
pause

