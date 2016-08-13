#!/bin/bash
cd $(dirname $0)
./test | osmconvert - | diff - expected.xml
exit $?
