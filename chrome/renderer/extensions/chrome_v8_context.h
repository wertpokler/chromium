// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_RENDERER_EXTENSIONS_CHROME_V8_CONTEXT_H_
#define CHROME_RENDERER_EXTENSIONS_CHROME_V8_CONTEXT_H_
#pragma once

#include <string>

#include "base/basictypes.h"
#include "v8/include/v8.h"

namespace base {
class ListValue;
}

namespace WebKit {
class WebFrame;
}

class RenderView;

// Chrome's wrapper for a v8 context.
class ChromeV8Context {
 public:
  ChromeV8Context(v8::Handle<v8::Context> context,
                  WebKit::WebFrame* frame,
                  const std::string& extension_id);
  ~ChromeV8Context();

  v8::Handle<v8::Context> v8_context() const {
    return v8_context_;
  }

  const std::string& extension_id() const {
    return extension_id_;
  }

  WebKit::WebFrame* web_frame() const {
    return web_frame_;
  }
  void clear_web_frame() {
    web_frame_ = NULL;
  }

  // Returns the RenderView associated with this context. Can return NULL if the
  // context is in the process of being destroyed.
  RenderView* GetRenderView() const;

  // Fires the onload and onunload events on the chromeHidden object.
  // TODO(aa): Does these make more sense with EventBindings?
  void DispatchOnLoadEvent(bool is_extension_process,
                           bool is_incognito_process) const;
  void DispatchOnUnloadEvent() const;

  // Call the named method of the chromeHidden object in this context.
  // The function can be a sub-property like "Port.dispatchOnMessage". Returns
  // the result of the function call. If an exception is thrown an empty Handle
  // will be returned.
  v8::Handle<v8::Value> CallChromeHiddenMethod(
      const std::string& function_name,
      int argc,
      v8::Handle<v8::Value>* argv) const;

 private:
  // The v8 context the bindings are accessible to. We keep a strong reference
  // to it for simplicity. In the case of content scripts, this is necessary
  // because we want all scripts from the same extension for the same frame to
  // run in the same context, so we can't have the contexts being GC'd if
  // nothing is happening. In the case of page contexts, this isn't necessary
  // since the DOM keeps the context alive, but it makes things simpler to not
  // distinguish the two cases.
  v8::Persistent<v8::Context> v8_context_;

  // The WebFrame associated with this context. This can be NULL because this
  // object can outlive is destroyed asynchronously.
  WebKit::WebFrame* web_frame_;

  // The extension ID this context is associated with.
  // TODO(aa): Could we get away with removing this?
  std::string extension_id_;

  DISALLOW_COPY_AND_ASSIGN(ChromeV8Context);
};

#endif  // CHROME_RENDERER_EXTENSIONS_CHROME_V8_CONTEXT_H_
