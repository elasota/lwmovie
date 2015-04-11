call "D:\Low Speed Installs\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
"D:\Source Code\cov-analysis-win64-7.6.0\bin\cov-build.exe" --dir cov-int msbuild.exe lwmovie.sln /t:Rebuild /p:Configuration=Release
"C:\Program Files\7-zip\7z.exe" a lwmovie-cov.tar cov-int
"C:\Program Files\7-zip\7z.exe" a -mx=9 lwmovie-cov.tar.gz lwmovie-cov.tar
del /Q lwmovie-cov.tar
pause
