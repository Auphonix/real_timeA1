#include "counters.h"

void
initCounters(Counters *ctrs)
{
  ctrs->frameCount = 0;
  ctrs->frameTime = 0.0;
  ctrs->lastTime = 0.0;
  ctrs->interval = 1000.0; // 1 s
}

void
updateCounters(Counters *ctrs, float t)
{
  float dt = (t - ctrs->lastTime);
  if (dt > ctrs->interval) {
    ctrs->frameTime = dt / ctrs->frameCount;
    ctrs->lastTime = t;
    ctrs->frameCount = 0;
  }
}


