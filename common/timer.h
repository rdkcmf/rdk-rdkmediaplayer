/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
#ifndef _PLAYER_TIMER_H_
#define _PLAYER_TIMER_H_

class TimerBaseImpl;

class TimerBase {
public:
  TimerBase();
  virtual ~TimerBase();

  void startOneShot(double interval);
  void stop();
  bool isActive() const;

protected:
  virtual void fired() = 0;

private:
  TimerBaseImpl *m_impl;
  friend class TimerBaseImpl;
};

template <typename TimerFiredClass>
class Timer : public TimerBase {
public:
  typedef void (TimerFiredClass::*TimerFiredFunction)();

  Timer(TimerFiredClass* o, TimerFiredFunction f)
    : m_object(o), m_function(f) { }

private:
  virtual void fired() { (m_object->*m_function)(); }

  TimerFiredClass* m_object;
  TimerFiredFunction m_function;
};

#endif  // _PLAYER_TIMER_H_
