#ifndef TIMER_H_
#define TIMER_H_

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


#endif TIMER_H_
