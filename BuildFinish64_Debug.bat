set targetIncludeDir=D:\VS2019\Proj\DestroyBrickServerKit\WindowsServerBase\include\
set targetLibDir=D:\VS2019\Proj\DestroyBrickServerKit\WindowsServerBase\lib64\

copy .\*.h %targetIncludeDir%

echo copy .h files finished

copy .\x64\Debug\*.lib %targetLibDir%
copy .\x64\Debug\*.pdb %targetLibDir%

echo copy lib and pdb files finished

pause