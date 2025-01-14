// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/logging.h"
#include "base/mac/bundle_locations.h"
#include "base/macros.h"
#include "base/path_service.h"
#include "content/shell/app/blink_test_platform_support.h"

#include <AppKit/AppKit.h>
#include <Foundation/Foundation.h>

namespace content {

namespace {

void SetDefaultsToLayoutTestValues(void) {
  // So we can match the WebKit layout tests, we want to force a bunch of
  // preferences that control appearance to match.
  // (We want to do this as early as possible in application startup so
  // the settings are in before any higher layers could cache values.)

  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
  // Do not set text-rendering prefs (AppleFontSmoothing,
  // AppleAntiAliasingThreshold) here: Skia picks the right settings for this
  // in layout test mode, see FontSkia.cpp in WebKit and
  // SkFontHost_mac_coretext.cpp in skia.
  const NSInteger kBlueTintedAppearance = 1;
  [defaults setInteger:kBlueTintedAppearance
                forKey:@"AppleAquaColorVariant"];
  [defaults setObject:@"0.709800 0.835300 1.000000"
               forKey:@"AppleHighlightColor"];
  [defaults setObject:@"0.500000 0.500000 0.500000"
               forKey:@"AppleOtherHighlightColor"];
  [defaults setObject:[NSArray arrayWithObject:@"en"]
               forKey:@"AppleLanguages"];
  [defaults setBool:NO
             forKey:@"AppleScrollAnimationEnabled"];
  [defaults setBool:NO
             forKey:@"NSScrollAnimationEnabled"];
  [defaults setObject:@"Always"
               forKey:@"AppleShowScrollBars"];
}

}  // namespace

bool CheckLayoutSystemDeps() {
  return true;
}

bool BlinkTestPlatformInitialize() {

  SetDefaultsToLayoutTestValues();

  // Load font files in the resource folder.
  static const char* const fontFileNames[] = {
      "AHEM____.TTF", "ChromiumAATTest.ttf"
  };

  // mainBundle is Content Shell Helper.app.  Go two levels up to find
  // Content Shell.app. Due to DumpRenderTree injecting the font files into
  // its direct dependents, it's not easily possible to put the ttf files into
  // the helper's resource directory instead of the outer bundle's resource
  // directory.
  NSString* bundle = [base::mac::FrameworkBundle() bundlePath];
  bundle = [bundle stringByAppendingPathComponent:@"../.."];
  NSURL* resources_directory = [[NSBundle bundleWithPath:bundle] resourceURL];

  NSMutableArray* font_urls = [NSMutableArray array];
  for (unsigned i = 0; i < arraysize(fontFileNames); ++i) {
    NSURL* font_url = [resources_directory
        URLByAppendingPathComponent:[NSString
            stringWithUTF8String:fontFileNames[i]]];
    [font_urls addObject:[font_url absoluteURL]];
  }

  CFArrayRef errors = 0;
  if (!CTFontManagerRegisterFontsForURLs((CFArrayRef)font_urls,
                                         kCTFontManagerScopeProcess,
                                         &errors)) {
    DLOG(FATAL) << "Fail to activate fonts.";
    CFRelease(errors);
  }

  // Add <app bundle's parent dir>/plugins to the plugin path so we can load
  // test npapi plugins (soon to be removed).
  const char kExtraPluginDir[] = "extra-plugin-dir";
  base::FilePath plugins_dir;
  PathService::Get(base::DIR_EXE, &plugins_dir);
  plugins_dir = plugins_dir.AppendASCII("../../../plugins");
  base::CommandLine& command_line = *base::CommandLine::ForCurrentProcess();
  command_line.AppendSwitchPath(kExtraPluginDir, plugins_dir);

  return true;
}

}  // namespace content
