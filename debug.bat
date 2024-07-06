gdb -ex "target remote 127.0.0.1:1234" -ex "set architecture i386:x86-64" -ex "file ./kernel/bin/kernel" -ex "break start"
pause