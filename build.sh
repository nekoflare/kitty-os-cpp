#!/usr/bin/env bash

cd libs/klibc++/
./build.sh
cd ../../

cd apps/init/
./build.sh
cd ../../

cp apps/init/bin/init.exe files/init.exe

make run-uefi -j12