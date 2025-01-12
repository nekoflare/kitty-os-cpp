for file in $(ls | grep -v loongarch64 | grep -v code | grep -v clear_cfg); do
    # Get the size of the file
    filesize=$(stat --format=%s "$file")
    
    # Use a block size of 1M (1 Megabyte)
    dd if=/dev/zero of="$file" bs=1M count=$((filesize / 1308672 )) conv=notrunc
    
    echo "Cleared contents of $file (size: $filesize bytes)"
done

