#pragma once

/*
 * performance counters
 */
 
typedef struct {
  int frameCount;
  float frameTime;
  float interval;
  float lastTime;
} Counters;

void updateCounters(Counters *ctrs, float t);
void initCounters(Counters *ctrs);

