if [ $# -eq 1 ]; then 
    dd if=/dev/random of=test.bin bs="$1" count=1 iflag=fullblock
else
    dd if=/dev/random of=test.bin bs=1000 count=1 iflag=fullblock
fi