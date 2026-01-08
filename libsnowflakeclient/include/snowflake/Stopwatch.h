#ifndef STOPWATCH_H
#define STOPWATCH_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Stopwatch
{
  bool isStarted;

  long startTime;

  long elapsedTime;
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
