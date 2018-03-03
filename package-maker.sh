#!/bin/bash
NAME=$( basename $1 )
mkdir $2
cp $( ldd $1 | grep '=>' | cut -d ' ' -f 3 ) $2/
echo '#!/bin/bash' >> $2/$NAME.sh
echo 'dir=$( dirname "$0" )' >> $2/$NAME.sh
echo "LD_LIBRARY_PATH=\$dir \$dir/$NAME \$@" >> $2/$NAME.sh
chmod +x $2/$NAME.sh
cp $1 $2/
