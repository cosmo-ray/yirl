#!/bin/bash
cppcheck -I include/core/ --enable=warning --enable=style --enable=performance --enable=portability  CMakeFiles/feature_tests.c tests/lifecycle.c tests/main.c tests/setunset.c core/entity/debug_core.c core/entity/entity.c core/util/debug.c
rats CMakeFiles/feature_tests.c tests/lifecycle.c tests/main.c tests/setunset.c core/entity/debug_core.c core/entity/entity.c core/util/debug.c
