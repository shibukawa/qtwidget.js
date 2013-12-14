set QTDIR=C:\Qt\5.2.0\mingw48_32
cd third_party\qtscriptgenerator\generator
qmake
mingw32-make
release/generator
cd ../qtbindings
qmake
mingw32-make
cd ../../..
mkdir plugins
mkdir jsx
xcopy /e third_party\qtscriptgenerator\plugins .\plugins
xcopy /e third_party\qtscriptgenerator\jsx .\jsx
