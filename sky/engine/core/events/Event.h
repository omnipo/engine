/*
 * Copyright (C) 2001 Peter Kelly (pmk@post.com)
 * Copyright (C) 2001 Tobias Anton (anton@stud.fbi.fh-darmstadt.de)
 * Copyright (C) 2006 Samuel Weinig (sam.weinig@gmail.com)
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef SKY_ENGINE_CORE_EVENTS_EVENT_H_
#define SKY_ENGINE_CORE_EVENTS_EVENT_H_

#include "sky/engine/tonic/dart_wrappable.h"
#include "sky/engine/platform/heap/Handle.h"
#include "sky/engine/wtf/RefCounted.h"
#include "sky/engine/wtf/text/AtomicString.h"

namespace blink {

class EventDispatcher;
class ExecutionContext;

struct EventInit {
    bool bubbles = false;
    bool cancelable = false;

private:
    STACK_ALLOCATED();
};

class Event : public RefCounted<Event>, public DartWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    enum PhaseType {
        NONE                = 0,
        CAPTURING_PHASE     = 1,
        AT_TARGET           = 2,
        BUBBLING_PHASE      = 3
    };

    static PassRefPtr<Event> create()
    {
        return adoptRef(new Event);
    }

    // A factory for a simple event. The event doesn't bubble, and isn't
    // cancelable.
    // http://www.whatwg.org/specs/web-apps/current-work/multipage/webappapis.html#fire-a-simple-event
    static PassRefPtr<Event> create(const AtomicString& type, bool bubbles = false, bool cancelable = false)
    {
        return adoptRef(new Event(type, bubbles, cancelable));
    }
    static PassRefPtr<Event> createCancelable(const AtomicString& type)
    {
        return adoptRef(new Event(type, false, true));
    }
    static PassRefPtr<Event> createBubble(const AtomicString& type)
    {
        return adoptRef(new Event(type, true, false));
    }
    static PassRefPtr<Event> createCancelableBubble(const AtomicString& type)
    {
        return adoptRef(new Event(type, true, true));
    }

    static PassRefPtr<Event> create(const AtomicString& type, const EventInit& initializer)
    {
        return adoptRef(new Event(type, initializer));
    }

    virtual ~Event();

    void initEvent(const AtomicString& type, bool canBubble, bool cancelable);

    const AtomicString& type() const { return m_type; }
    void setType(const AtomicString& type) { m_type = type; }

    unsigned short eventPhase() const { return m_eventPhase; }
    void setEventPhase(unsigned short eventPhase) { m_eventPhase = eventPhase; }

    bool bubbles() const { return m_canBubble; }
    bool cancelable() const { return m_cancelable; }
    double timeStamp() const { return m_timeStamp; }

    void stopPropagation() { m_propagationStopped = true; }
    void stopImmediatePropagation() { m_immediatePropagationStopped = true; }

    bool legacyReturnValue() const;
    void setLegacyReturnValue(bool returnValue);

    virtual const AtomicString& interfaceName() const;

    // These events are general classes of events.
    virtual bool isKeyboardEvent() const;

    // Drag events are a subset of mouse events.
    virtual bool isDragEvent() const;

    // These events lack a DOM interface.
    virtual bool isClipboardEvent() const;

    bool propagationStopped() const { return m_propagationStopped || m_immediatePropagationStopped; }
    bool immediatePropagationStopped() const { return m_immediatePropagationStopped; }

    bool defaultPrevented() const { return m_defaultPrevented; }
    virtual void preventDefault()
    {
        if (m_cancelable)
            m_defaultPrevented = true;
    }
    void setDefaultPrevented(bool defaultPrevented) { m_defaultPrevented = defaultPrevented; }

    bool defaultHandled() const { return m_defaultHandled; }
    void setDefaultHandled() { m_defaultHandled = true; }

    bool cancelBubble() const { return m_cancelBubble; }
    void setCancelBubble(bool cancel) { m_cancelBubble = cancel; }

    Event* underlyingEvent() const { return m_underlyingEvent.get(); }
    void setUnderlyingEvent(PassRefPtr<Event>);

    bool isBeingDispatched() const { return eventPhase(); }

protected:
    Event();
    Event(const AtomicString& type, bool canBubble, bool cancelable);
    Event(const AtomicString& type, const EventInit&);

    double m_timeStamp;

private:
    AtomicString m_type;
    bool m_canBubble;
    bool m_cancelable;

    bool m_propagationStopped;
    bool m_immediatePropagationStopped;
    bool m_defaultPrevented;
    bool m_defaultHandled;
    bool m_cancelBubble;

    unsigned short m_eventPhase;
    RefPtr<Event> m_underlyingEvent;
};

#define DEFINE_EVENT_TYPE_CASTS(typeName) \
    DEFINE_TYPE_CASTS(typeName, Event, event, event->is##typeName(), event.is##typeName())

} // namespace blink

#endif  // SKY_ENGINE_CORE_EVENTS_EVENT_H_
