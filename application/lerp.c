#define DEBUG_THIS_FILE DEBUG_LERP_FILE

#include <stdint.h>
#include <stdbool.h>

#include "utils.h"
#include "main.h"

#include "lerp.h"


extern float lerp(float start, float end, float progress) {
	if(progress < 0.0f) { progress = 0.0f; }
	else if(progress > 1.0f) { progress = 1.0f; }
	return (start * (1.0f - progress)) + (end * progress);
}

