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
#include "timer.h"
#include <glib.h>

class TimerBaseImpl {
public:
  explicit TimerBaseImpl(TimerBase* pub)
    : m_pub(pub)
    , m_tag(0) {
  }

  virtual ~TimerBaseImpl() {
    if (m_tag)
      g_source_remove(m_tag);
    m_tag = 0;
  }

  void start(double nextFireInterval) {
    guint delay = (guint)(nextFireInterval * 1000);
    if (m_tag)
      g_source_remove(m_tag);
    m_tag = g_timeout_add(delay, [](gpointer data) -> gboolean {
      TimerBaseImpl &self = *static_cast<TimerBaseImpl*>(data);
      self.m_tag = 0;
      self.m_pub->fired();
      return G_SOURCE_REMOVE;
    }, this);
  }

  bool isActive() const {
    return m_tag != 0;
  }

private:
  TimerBase* m_pub;
  guint m_tag;
};

TimerBase::TimerBase() : m_impl(nullptr) {
}

TimerBase::~TimerBase() {
  stop();
}

void TimerBase::startOneShot(double interval) {
  if (!m_impl)
    m_impl = new TimerBaseImpl(this);
  m_impl->start(interval);
}

void TimerBase::stop() {
  if (m_impl) {
    delete m_impl;
    m_impl = nullptr;
  }
}

bool TimerBase::isActive() const {
  return m_impl && m_impl->isActive();
}
