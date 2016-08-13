#!/bin/bash
cd $(dirname $0)
diff <(./test | osmconvert -) expected.xml
exit $?
