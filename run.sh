#! /bin/sh
clear
exe="a.out"
export GOPATH=`pwd`
rm -f bin/$exe
go install $exe
if [ -f bin/main ]; then
    clear
    $GOPATH/bin/$exe
fi
