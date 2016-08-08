#!/bin/bash
diff <(./test | osmconvert -) expected.xml
exit $?
