// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/mediasession/HTMLMediaElementMediaSession.h"

#include "core/dom/ExceptionCode.h"

namespace blink {

MediaSession* HTMLMediaElementMediaSession::session(HTMLMediaElement& mediaElement)
{
    if (HTMLMediaElementMediaSession* supplement = fromIfExists(mediaElement))
        return supplement->m_session.get();
    return nullptr;
}

void HTMLMediaElementMediaSession::setSession(HTMLMediaElement& mediaElement, MediaSession* session, ExceptionState& exceptionState)
{
    HTMLMediaElement::NetworkState networkState = mediaElement.networkState();
    if (networkState == HTMLMediaElement::NETWORK_IDLE || networkState == HTMLMediaElement::NETWORK_LOADING) {
        exceptionState.throwDOMException(InvalidStateError, "networkState must be NETWORK_EMPTY or NETWORK_NO_SOURCE.");
        return;
    }

    from(mediaElement).m_session = session;
}

const char* HTMLMediaElementMediaSession::supplementName()
{
    return "HTMLMediaElementMediaSession";
}

HTMLMediaElementMediaSession& HTMLMediaElementMediaSession::from(HTMLMediaElement& element)
{
    HTMLMediaElementMediaSession* supplement = fromIfExists(element);
    if (!supplement) {
        supplement = new HTMLMediaElementMediaSession();
        provideTo(element, supplementName(), adoptPtrWillBeNoop(supplement));
    }
    return *supplement;
}

HTMLMediaElementMediaSession* HTMLMediaElementMediaSession::fromIfExists(HTMLMediaElement& element)
{
    return static_cast<HTMLMediaElementMediaSession*>(WillBeHeapSupplement<HTMLMediaElement>::from(element, supplementName()));
}

DEFINE_TRACE(HTMLMediaElementMediaSession)
{
    visitor->trace(m_session);
    WillBeHeapSupplement<HTMLMediaElement>::trace(visitor);
}

} // namespace blink
