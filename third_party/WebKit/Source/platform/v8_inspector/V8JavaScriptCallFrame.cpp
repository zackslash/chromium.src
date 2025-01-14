// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/v8_inspector/V8JavaScriptCallFrame.h"

#include "platform/v8_inspector/InspectorWrapper.h"
#include "platform/v8_inspector/JavaScriptCallFrame.h"
#include "platform/v8_inspector/V8StringUtil.h"
#include "wtf/RefPtr.h"
#include "wtf/StdLibExtras.h"
#include <algorithm>

namespace blink {

namespace {

void callerAttributeGetterCallback(v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    JavaScriptCallFrame* impl = V8JavaScriptCallFrame::unwrap(info.GetIsolate()->GetCurrentContext(), info.Holder());
    JavaScriptCallFrame* caller = impl->caller();
    if (!caller)
        return;
    v8::Isolate* isolate = info.GetIsolate();
    v8::Local<v8::FunctionTemplate> wrapperTemplate = impl->wrapperTemplate(isolate);
    if (wrapperTemplate.IsEmpty())
        return;
    info.GetReturnValue().Set(V8JavaScriptCallFrame::wrap(impl->client(), wrapperTemplate, isolate->GetCurrentContext(), caller));
}

void sourceIDAttributeGetterCallback(v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    JavaScriptCallFrame* impl = V8JavaScriptCallFrame::unwrap(info.GetIsolate()->GetCurrentContext(), info.Holder());
    info.GetReturnValue().Set(impl->sourceID());
}

void lineAttributeGetterCallback(v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    JavaScriptCallFrame* impl = V8JavaScriptCallFrame::unwrap(info.GetIsolate()->GetCurrentContext(), info.Holder());
    info.GetReturnValue().Set(impl->line());
}

void columnAttributeGetterCallback(v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    JavaScriptCallFrame* impl = V8JavaScriptCallFrame::unwrap(info.GetIsolate()->GetCurrentContext(), info.Holder());
    info.GetReturnValue().Set(impl->column());
}

void scopeChainAttributeGetterCallback(v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    JavaScriptCallFrame* impl = V8JavaScriptCallFrame::unwrap(info.GetIsolate()->GetCurrentContext(), info.Holder());
    info.GetReturnValue().Set(impl->scopeChain());
}

void thisObjectAttributeGetterCallback(v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    JavaScriptCallFrame* impl = V8JavaScriptCallFrame::unwrap(info.GetIsolate()->GetCurrentContext(), info.Holder());
    info.GetReturnValue().Set(impl->thisObject());
}

void stepInPositionsAttributeGetterCallback(v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    JavaScriptCallFrame* impl = V8JavaScriptCallFrame::unwrap(info.GetIsolate()->GetCurrentContext(), info.Holder());
    info.GetReturnValue().Set(toV8String(info.GetIsolate(), impl->stepInPositions()));
}

void functionNameAttributeGetterCallback(v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    JavaScriptCallFrame* impl = V8JavaScriptCallFrame::unwrap(info.GetIsolate()->GetCurrentContext(), info.Holder());
    info.GetReturnValue().Set(toV8String(info.GetIsolate(), impl->functionName()));
}

void functionLineAttributeGetterCallback(v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    JavaScriptCallFrame* impl = V8JavaScriptCallFrame::unwrap(info.GetIsolate()->GetCurrentContext(), info.Holder());
    info.GetReturnValue().Set(impl->functionLine());
}

void functionColumnAttributeGetterCallback(v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    JavaScriptCallFrame* impl = V8JavaScriptCallFrame::unwrap(info.GetIsolate()->GetCurrentContext(), info.Holder());
    info.GetReturnValue().Set(impl->functionColumn());
}

void isAtReturnAttributeGetterCallback(v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    JavaScriptCallFrame* impl = V8JavaScriptCallFrame::unwrap(info.GetIsolate()->GetCurrentContext(), info.Holder());
    info.GetReturnValue().Set(impl->isAtReturn());
}

void returnValueAttributeGetterCallback(v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    JavaScriptCallFrame* impl = V8JavaScriptCallFrame::unwrap(info.GetIsolate()->GetCurrentContext(), info.Holder());
    info.GetReturnValue().Set(impl->returnValue());
}

void evaluateWithExceptionDetailsMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    JavaScriptCallFrame* impl = V8JavaScriptCallFrame::unwrap(info.GetIsolate()->GetCurrentContext(), info.Holder());
    info.GetReturnValue().Set(impl->evaluateWithExceptionDetails(info[0], info[1]));
}

void restartMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    JavaScriptCallFrame* impl = V8JavaScriptCallFrame::unwrap(info.GetIsolate()->GetCurrentContext(), info.Holder());
    v8::MaybeLocal<v8::Value> result = impl->restart();
    v8::Local<v8::Value> value;
    if (result.ToLocal(&value))
        info.GetReturnValue().Set(value);
}

void setVariableValueMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    JavaScriptCallFrame* impl = V8JavaScriptCallFrame::unwrap(info.GetIsolate()->GetCurrentContext(), info.Holder());
    v8::Maybe<int32_t> maybeScopeIndex = info[0]->Int32Value(info.GetIsolate()->GetCurrentContext());
    if (maybeScopeIndex.IsNothing())
        return;
    int scopeIndex = maybeScopeIndex.FromJust();
    v8::MaybeLocal<v8::Value> result = impl->setVariableValue(scopeIndex, info[1], info[2]);
    v8::Local<v8::Value> value;
    if (result.ToLocal(&value))
        info.GetReturnValue().Set(value);

}

void scopeTypeMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    JavaScriptCallFrame* impl = V8JavaScriptCallFrame::unwrap(info.GetIsolate()->GetCurrentContext(), info.Holder());
    v8::Maybe<int32_t> maybeScopeIndex = info[0]->Int32Value(info.GetIsolate()->GetCurrentContext());
    if (maybeScopeIndex.IsNothing())
        return;
    int scopeIndex = maybeScopeIndex.FromJust();
    info.GetReturnValue().Set(impl->scopeType(scopeIndex));
}

void scopeNameMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    JavaScriptCallFrame* impl = V8JavaScriptCallFrame::unwrap(info.GetIsolate()->GetCurrentContext(), info.Holder());
    v8::Maybe<int32_t> maybeScopeIndex = info[0]->Int32Value(info.GetIsolate()->GetCurrentContext());
    if (maybeScopeIndex.IsNothing())
        return;
    int scopeIndex = maybeScopeIndex.FromJust();
    info.GetReturnValue().Set(impl->scopeName(scopeIndex));
}

char hiddenPropertyName[] = "v8inspector::JavaScriptCallFrame";
char className[] = "V8JavaScriptCallFrame";
using JavaScriptCallFrameWrapper = InspectorWrapper<JavaScriptCallFrame, hiddenPropertyName, className>;

const JavaScriptCallFrameWrapper::V8AttributeConfiguration V8JavaScriptCallFrameAttributes[] = {
    {"scopeChain", scopeChainAttributeGetterCallback},
    {"thisObject", thisObjectAttributeGetterCallback},
    {"returnValue", returnValueAttributeGetterCallback},

    {"caller", callerAttributeGetterCallback},
    {"sourceID", sourceIDAttributeGetterCallback},
    {"line", lineAttributeGetterCallback},
    {"column", columnAttributeGetterCallback},
    {"stepInPositions", stepInPositionsAttributeGetterCallback},
    {"functionName", functionNameAttributeGetterCallback},
    {"functionLine", functionLineAttributeGetterCallback},
    {"functionColumn", functionColumnAttributeGetterCallback},
    {"isAtReturn", isAtReturnAttributeGetterCallback},
};

const JavaScriptCallFrameWrapper::V8MethodConfiguration V8JavaScriptCallFrameMethods[] = {
    {"evaluateWithExceptionDetails", evaluateWithExceptionDetailsMethodCallback},
    {"restart", restartMethodCallback},
    {"setVariableValue", setVariableValueMethodCallback},
    {"scopeType", scopeTypeMethodCallback},
    {"scopeName", scopeNameMethodCallback}
};

} // namespace

v8::Local<v8::FunctionTemplate> V8JavaScriptCallFrame::createWrapperTemplate(v8::Isolate* isolate)
{
    Vector<InspectorWrapperBase::V8MethodConfiguration> methods(WTF_ARRAY_LENGTH(V8JavaScriptCallFrameMethods));
    std::copy(V8JavaScriptCallFrameMethods, V8JavaScriptCallFrameMethods + WTF_ARRAY_LENGTH(V8JavaScriptCallFrameMethods), methods.begin());
    Vector<InspectorWrapperBase::V8AttributeConfiguration> attributes(WTF_ARRAY_LENGTH(V8JavaScriptCallFrameAttributes));
    std::copy(V8JavaScriptCallFrameAttributes, V8JavaScriptCallFrameAttributes + WTF_ARRAY_LENGTH(V8JavaScriptCallFrameAttributes), attributes.begin());
    return JavaScriptCallFrameWrapper::createWrapperTemplate(isolate, methods, attributes);
}

v8::Local<v8::Object> V8JavaScriptCallFrame::wrap(V8DebuggerClient* client, v8::Local<v8::FunctionTemplate> constructorTemplate, v8::Local<v8::Context> context, PassRefPtr<JavaScriptCallFrame> frame)
{
    // Store template for .caller callback
    frame->setWrapperTemplate(constructorTemplate, context->GetIsolate());
    return JavaScriptCallFrameWrapper::wrap(client, constructorTemplate, context, frame);
}

JavaScriptCallFrame* V8JavaScriptCallFrame::unwrap(v8::Local<v8::Context> context, v8::Local<v8::Object> object)
{
    return JavaScriptCallFrameWrapper::unwrap(context, object);
}

} // namespace blink
