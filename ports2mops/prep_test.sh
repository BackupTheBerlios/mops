#!/bin/sh
# $Id: prep_test.sh,v 1.3 2006/12/15 12:36:00 adiakin Exp $
rm test/out/vim-port.xml
if [ -e "convert.pl" ]; then
    echo "coping script...."
    cp convert.pl test/;
else
    echo "no script found"
    exit;
fi

if [ -e "template.xml" ]; then
   echo "copy template..."
   cp template.xml test/;
else
    echo "no template found"
    rm test/convert.pl
    echo "exit..."
    exit;
fi

echo "go to test/ and run ./convert.pl now!"
(cd test/ && ./convert.pl)
#less test/out/vim-port.xml
