#!/bin/bash
NAME=$( basename $1 )
mkdir $2
ls -l
export LD_LIBRARY_PATH=./
ldd $1 | grep '=>' | cut -d ' ' -f 3
cp -v $( ldd $1 | grep '=>' | cut -d ' ' -f 3 | grep -v  -e libm.so -e libc.so -e libdl.so.2 -e ld-linux-x86-64.so.2 -e libpthread.so.0 -e librt.so) $2/
echo '#!/bin/bash' >> $2/$NAME.sh
echo 'dir=$( dirname "$0" )' >> $2/$NAME.sh
echo "LD_LIBRARY_PATH=\$dir \$dir/$NAME \$@" >> $2/$NAME.sh
chmod +x $2/$NAME.sh
cp $1 $2/
