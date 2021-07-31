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

msbuild.exe lwmovie.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64
msbuild.exe lwmovie.sln /t:Rebuild /p:Configuration=Release /p:Platform=Win32
msbuild.exe lwfe.sln /t:Rebuild /p:Configuration=Release "/p:Platform=Any CPU"

copy /Y Release\lwmovie.lib lwmovie-release\lib\lwmovie.lib
copy /Y x64\Release\lwmovie64.lib lwmovie-release\lib\lwmovie64.lib

copy /Y lwmovie\lwmovie.h lwmovie-release\include\lwmovie\lwmovie.h
copy /Y lwmovie\lwmovie_cake.h lwmovie-release\include\lwmovie\lwmovie_cake.h
copy /Y lwmovie\lwmovie_cake_cppshims.hpp lwmovie-release\include\lwmovie\lwmovie_cake_cppshims.hpp
copy /Y lwmovie\lwmovie_cpp_shims.hpp lwmovie-release\include\lwmovie\lwmovie_cpp_shims.hpp
copy /Y lwmovie\lwmovie_api.h lwmovie-release\include\lwmovie\lwmovie_api.h
copy /Y lwmovie\lwmovie_external_types.h lwmovie-release\include\lwmovie\lwmovie_external_types.h
copy /Y common\lwmovie_config.h lwmovie-release\include\common\lwmovie_config.h
copy /Y common\lwmovie_attribs.h lwmovie-release\include\common\lwmovie_attribs.h
copy /Y common\lwmovie_coretypes.h lwmovie-release\include\common\lwmovie_coretypes.h

copy /Y Release\lwmovie.dll lwmovie-release\bin\lwmovie.dll
copy /Y x64\Release\lwmovie64.dll lwmovie-release\bin\lwmovie64.dll
copy /Y x64\Release\lwenctools.dll lwmovie-release\bin\lwenctools.dll
copy /Y x64\Release\lwplay.exe lwmovie-release\bin\lwplay.exe
copy /Y x64\Release\lwrerange.exe lwmovie-release\bin\lwrerange.exe
copy /Y x64\Release\lwmux.exe lwmovie-release\bin\lwmux.exe
copy /Y x64\Release\lwfe.exe "lwmovie-release\bin\lwmovie Encoder.exe"
copy /Y x64\Release\lwenccmd.exe "lwmovie-release\bin\lwenccmd.exe"
copy /Y x64\Release\lwroqenc.exe "lwmovie-release\bin\lwroqenc.exe"
copy /Y x64\Release\lwthenc.exe "lwmovie-release\bin\lwthenc.exe"
copy /Y x64\Release\SDL2.dll lwmovie-release\bin\SDL2.dll

copy /Y doc\CommandLineOptions.txt lwmovie-release\doc\
copy /Y doc\Cake_Documentation.pdf lwmovie-release\doc\
copy /Y doc\architecture.svg lwmovie-release\doc\
copy /Y doc\LICENSE.txt lwmovie-release\doc\
copy /Y doc\LICENSE-lwroqenc.txt lwmovie-release\doc\
copy /Y doc\README-libjpeg.txt lwmovie-release\doc\
copy /Y doc\Encoding.txt lwmovie-release\doc\
copy /Y doc\Decoding.txt lwmovie-release\doc\
copy /Y doc\gpudecoding_d3d11.txt lwmovie-release\doc\

xcopy /Y /E x64\Release\ffmpeg lwmovie-release\bin\ffmpeg
xcopy /Y /E external-sources lwmovie-release\external-sources

del deploy.7z
del deploy.7z.tmp
"C:\Program Files\7-zip\7z.exe" a -mx=9 deploy.7z lwmovie-release
rmdir /S /Q lwmovie-release
pause

