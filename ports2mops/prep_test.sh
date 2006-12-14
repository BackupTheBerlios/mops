#!/bin/sh
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