#ifndef STOPWATCH_H
#define STOPWATCH_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Stopwatch
{
#ifdef __cplusplus
  /* Default member initializers protect C++ callers from uninitialized use
   * (SNOW-4007740); rationale and regression coverage live in
   * tests/test_unit_stopwatch.cpp. C callers must still init at the call site. */
  bool isStarted = false;

  long startTime = 0;

  long elapsedTime = 0;
#else
  bool isStarted;

  long startTime;

  long elapsedTime;
#endif
} Stopwatch;

  void stopwatch_start(Stopwatch* s);
  
  void stopwatch_stop(Stopwatch* s);
  
  void stopwatch_reset(Stopwatch* s);
  
  void stopwatch_restart(Stopwatch* s);
  
  long stopwatch_elapsedMillis(Stopwatch* s);

  bool stopwatch_isStarted(Stopwatch* s);

#ifdef __cplusplus
}
#endif

#endif //STOPWATCH_H
