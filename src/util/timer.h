#ifndef _UTIL_TIMER_H_
#define _UTIL_TIMER_H_

/*----------------------------------------------------------------------------*/

class TimerPriv;

/*----------------------------------------------------------------------------*/

class Timer {
public:
  Timer();
  ~Timer();
  void stop();
  float getSec() const;
private:
  TimerPriv* m;
};


#endif /* _UTIL_TIMER_H_ */
