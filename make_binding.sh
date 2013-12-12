#!/bin/sh
#export QTDIR=~/Qt5.0.0-rc1/5.0.0-rc1/clang_64/
#export QTDIR=~/QtSDK/Desktop/Qt/4.8.1/gcc/
#export QTDIR=~/develop/qt4-src/qt4-lib/
export QTDIR=~/Qt/5.2.0/clang_64/
#export QTDIR=/opt/local/
export QTTOOLDIR=$QTDIR
export PATH=$QTTOOLDIR/bin:$PATH
export OUTPUT=plugins

cd ./third_party/qtscriptgenerator/generator/

if expr `sw_vers -productVersion` : "10.8.*" > /dev/null; then
    echo "Mac OS 10.8"
    qmake -spec macx-clang
    #qmake -spec unsupported/macx-clang
    #qmake
else
    echo "Mac OS 10.7"
    qmake
fi
make -j 4

#./generator --include-paths={$QTDIR}/gcc/include
#cd ../qtbindings
#if expr `sw_vers -productVersion` : "10.8.*" > /dev/null; then
#    echo "Mac OS 10.8"
#    qmake -spec macx-clang
#    #qmake -spec unsupported/macx-clang
#    #qmake
#else
#    echo "Mac OS 10.7"
#    qmake
#fi
#make -j 4
#
#cd ../../..
#
#if [ -d ${OUTPUT} ]; then
#    rm -rf ${OUTPUT}
#fi
#
#cp -R third_party/qtscriptgenerator/plugins ./${OUTPUT}