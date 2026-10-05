// eh.h shim — SEH does not exist on Android; crash reporting is handled by
// the Android platform (tombstones) and our diagnostics layer.
#pragma once
#include <crt/fundamental.h>
#define EXCEPTION_EXECUTE_HANDLER 1
#define EXCEPTION_CONTINUE_SEARCH 0
#define EXCEPTION_CONTINUE_EXECUTION (-1)
