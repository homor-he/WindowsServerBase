set targetIncludeDir=D:\VS2019\Proj\DestroyBrickServerKit\WindowsServerBase\include\
set targetLibDir=D:\VS2019\Proj\DestroyBrickServerKit\WindowsServerBase\lib\

copy .\*.h %targetIncludeDir%

echo copy .h files finished

copy .\Release\*.lib %targetLibDir%
copy .\Release\*.pdb %targetLibDir%

echo copy lib and pdb files finished

pause