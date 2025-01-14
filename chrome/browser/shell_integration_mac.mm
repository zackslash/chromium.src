// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/shell_integration.h"

#include "base/mac/bundle_locations.h"
#include "base/mac/foundation_util.h"
#include "base/strings/sys_string_conversions.h"
#include "chrome/common/channel_info.h"
#include "components/version_info/version_info.h"
#import "third_party/mozilla/NSWorkspace+Utils.h"

namespace shell_integration {

namespace {

// Returns true if |identifier| is the bundle id of the default browser.
bool IsIdentifierDefaultBrowser(NSString* identifier) {
  NSString* default_browser =
      [[NSWorkspace sharedWorkspace] defaultBrowserIdentifier];
  if (!default_browser)
    return false;

  // We need to ensure we do the comparison case-insensitive as LS doesn't
  // persist the case of our bundle id.
  NSComparisonResult result =
      [default_browser caseInsensitiveCompare:identifier];
  return result == NSOrderedSame;
}

// Returns true if |identifier| is the bundle id of the default client
// application for the given protocol.
bool IsIdentifierDefaultProtocolClient(NSString* identifier,
                                       NSString* protocol) {
  CFStringRef default_client_cf =
      LSCopyDefaultHandlerForURLScheme(base::mac::NSToCFCast(protocol));
  NSString* default_client = static_cast<NSString*>(
      base::mac::CFTypeRefToNSObjectAutorelease(default_client_cf));
  if (!default_client)
    return false;

  // We need to ensure we do the comparison case-insensitive as LS doesn't
  // persist the case of our bundle id.
  NSComparisonResult result =
      [default_client caseInsensitiveCompare:identifier];
  return result == NSOrderedSame;
}

}  // namespace

// Sets Chromium as default browser to be used by the operating system. This
// applies only for the current user. Returns false if this cannot be done, or
// if the operation fails.
bool SetAsDefaultBrowser() {
  if (CanSetAsDefaultBrowser() != SET_DEFAULT_UNATTENDED)
    return false;

  // We really do want the outer bundle here, not the main bundle since setting
  // a shortcut to Chrome as the default browser doesn't make sense.
  NSString* identifier = [base::mac::OuterBundle() bundleIdentifier];
  if (!identifier)
    return false;

  [[NSWorkspace sharedWorkspace] setDefaultBrowserWithIdentifier:identifier];
  return true;
}

// Sets Chromium as the default application to be used by the operating system
// for the given protocol. This applies only for the current user. Returns false
// if this cannot be done, or if the operation fails.
bool SetAsDefaultProtocolClient(const std::string& protocol) {
  if (protocol.empty())
    return false;

  if (CanSetAsDefaultProtocolClient() != SET_DEFAULT_UNATTENDED)
    return false;

  // We really do want the main bundle here since it makes sense to set an
  // app shortcut as a default protocol handler.
  NSString* identifier = [base::mac::MainBundle() bundleIdentifier];
  if (!identifier)
    return false;

  NSString* protocol_ns = [NSString stringWithUTF8String:protocol.c_str()];
  OSStatus return_code =
      LSSetDefaultHandlerForURLScheme(base::mac::NSToCFCast(protocol_ns),
                                      base::mac::NSToCFCast(identifier));
  return return_code == noErr;
}

DefaultWebClientSetPermission CanSetAsDefaultBrowser() {
  if (chrome::GetChannel() != version_info::Channel::CANARY) {
    return SET_DEFAULT_UNATTENDED;
  }

  return SET_DEFAULT_NOT_ALLOWED;
}

base::string16 GetApplicationNameForProtocol(const GURL& url) {
  NSURL* ns_url = [NSURL URLWithString:
      base::SysUTF8ToNSString(url.possibly_invalid_spec())];
  CFURLRef openingApp = NULL;
  OSStatus status = LSGetApplicationForURL((CFURLRef)ns_url,
                                           kLSRolesAll,
                                           NULL,
                                           &openingApp);
  if (status != noErr) {
    // likely kLSApplicationNotFoundErr
    return base::string16();
  }
  NSString* appPath = [(NSURL*)openingApp path];
  CFRelease(openingApp);  // NOT A BUG; LSGetApplicationForURL retains for us
  NSString* appDisplayName =
      [[NSFileManager defaultManager] displayNameAtPath:appPath];
  return base::SysNSStringToUTF16(appDisplayName);
}

// Attempt to determine if this instance of Chrome is the default browser and
// return the appropriate state. (Defined as being the handler for HTTP/HTTPS
// protocols; we don't want to report "no" here if the user has simply chosen
// to open HTML files in a text editor and FTP links with an FTP client.)
DefaultWebClientState GetDefaultBrowser() {
  // We really do want the outer bundle here, since this we want to know the
  // status of the main Chrome bundle and not a shortcut.
  NSString* my_identifier = [base::mac::OuterBundle() bundleIdentifier];
  if (!my_identifier)
    return UNKNOWN_DEFAULT;

  return IsIdentifierDefaultBrowser(my_identifier) ? IS_DEFAULT : NOT_DEFAULT;
}

// Returns true if Firefox is the default browser for the current user.
bool IsFirefoxDefaultBrowser() {
  return IsIdentifierDefaultBrowser(@"org.mozilla.firefox");
}

// Attempt to determine if this instance of Chrome is the default client
// application for the given protocol and return the appropriate state.
DefaultWebClientState IsDefaultProtocolClient(const std::string& protocol) {
  if (protocol.empty())
    return UNKNOWN_DEFAULT;

  // We really do want the main bundle here since it makes sense to set an
  // app shortcut as a default protocol handler.
  NSString* my_identifier = [base::mac::MainBundle() bundleIdentifier];
  if (!my_identifier)
    return UNKNOWN_DEFAULT;

  NSString* protocol_ns = [NSString stringWithUTF8String:protocol.c_str()];
  return IsIdentifierDefaultProtocolClient(my_identifier, protocol_ns) ?
      IS_DEFAULT : NOT_DEFAULT;
}

}  // namespace shell_integration