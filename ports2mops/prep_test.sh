#!/bin/sh
# $Id: prep_test.sh,v 1.2 2006/12/14 10:30:12 adiakin Exp $
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
