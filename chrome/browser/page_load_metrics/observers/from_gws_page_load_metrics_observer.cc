// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/page_load_metrics/observers/from_gws_page_load_metrics_observer.h"

#include "base/metrics/histogram.h"
#include "base/strings/string_util.h"
#include "components/page_load_metrics/browser/page_load_metrics_util.h"
#include "components/page_load_metrics/common/page_load_timing.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"

namespace {

bool IsFromGoogle(const GURL& url) {
  const char kGoogleSearchHostnamePrefix[] = "www.";
  std::string domain = net::registry_controlled_domains::GetDomainAndRegistry(
      url, net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);
  if (!base::StartsWith(domain, "google.", base::CompareCase::SENSITIVE) ||
      !base::StartsWith(url.host_piece(), kGoogleSearchHostnamePrefix,
                        base::CompareCase::SENSITIVE) ||
      url.host_piece().length() !=
          domain.length() + strlen(kGoogleSearchHostnamePrefix)) {
    return false;
  }
  return true;
}

// Returns true if the provided URL is a referrer string that came from
// a Google Web Search results page. This is a little non-deterministic
// because desktop and mobile websearch differ and sometimes just provide
// http://www.google.com/ as the referrer. See
// http://googlewebmastercentral.blogspot.in/2012/03/upcoming-changes-in-googles-http.html
// In the case of /url we can be sure that it came from websearch but we will
// be generous and allow for cases where a non-Google URL was provided a bare
// Google URL as a referrer. The domain validation matches the code used by the
// prerenderer for similar purposes.
// TODO(csharrison): Remove the fuzzy logic when the referrer is reliable.
bool IsFromGoogleSearchResult(const GURL& url, const GURL& referrer) {
  if (!IsFromGoogle(referrer))
    return false;
  if (base::StartsWith(referrer.path(), "/url", base::CompareCase::SENSITIVE))
    return true;
  bool is_possible_search_referrer =
      referrer.path().empty() || referrer.path() == "/" ||
      base::StartsWith(referrer.path(), "/search",
                       base::CompareCase::SENSITIVE) ||
      base::StartsWith(referrer.path(), "/webhp", base::CompareCase::SENSITIVE);
  return is_possible_search_referrer && !IsFromGoogle(url);
}

}  // namespace

FromGWSPageLoadMetricsObserver::FromGWSPageLoadMetricsObserver()
    : navigation_from_gws_(false) {}

void FromGWSPageLoadMetricsObserver::OnCommit(
    content::NavigationHandle* navigation_handle) {
  SetCommittedURLAndReferrer(navigation_handle->GetURL(),
                             navigation_handle->GetReferrer());
}

void FromGWSPageLoadMetricsObserver::OnComplete(
    const page_load_metrics::PageLoadTiming& timing,
    const page_load_metrics::PageLoadExtraInfo& extra_info) {
  using page_load_metrics::WasStartedInForegroundEventInForeground;

  if (!navigation_from_gws_)
    return;

  if (WasStartedInForegroundEventInForeground(
          timing.dom_content_loaded_event_start, extra_info)) {
    PAGE_LOAD_HISTOGRAM(
        "PageLoad.Clients.FromGWS.Timing2."
        "NavigationToDOMContentLoadedEventFired",
        timing.dom_content_loaded_event_start);
  }
  if (WasStartedInForegroundEventInForeground(timing.load_event_start,
                                              extra_info)) {
    PAGE_LOAD_HISTOGRAM(
        "PageLoad.Clients.FromGWS.Timing2.NavigationToLoadEventFired",
        timing.load_event_start);
  }
  if (WasStartedInForegroundEventInForeground(timing.first_layout,
                                              extra_info)) {
    PAGE_LOAD_HISTOGRAM(
        "PageLoad.Clients.FromGWS.Timing2.NavigationToFirstLayout",
        timing.first_layout);
  }
  if (WasStartedInForegroundEventInForeground(timing.first_text_paint,
                                              extra_info)) {
    PAGE_LOAD_HISTOGRAM(
        "PageLoad.Clients.FromGWS.Timing2.NavigationToFirstTextPaint",
        timing.first_text_paint);
  }
  if (WasStartedInForegroundEventInForeground(timing.first_image_paint,
                                              extra_info)) {
    PAGE_LOAD_HISTOGRAM(
        "PageLoad.Clients.FromGWS.Timing2.NavigationToFirstImagePaint",
        timing.first_image_paint);
  }
  if (WasStartedInForegroundEventInForeground(timing.first_paint, extra_info)) {
    PAGE_LOAD_HISTOGRAM(
        "PageLoad.Clients.FromGWS.Timing2.NavigationToFirstPaint",
        timing.first_paint);
  }
  if (WasStartedInForegroundEventInForeground(timing.first_contentful_paint,
                                              extra_info)) {
    PAGE_LOAD_HISTOGRAM(
        "PageLoad.Clients.FromGWS.Timing2.NavigationToFirstContentfulPaint",
        timing.first_contentful_paint);
  }
}

void FromGWSPageLoadMetricsObserver::SetCommittedURLAndReferrer(
    const GURL& url,
    const content::Referrer& referrer) {
  navigation_from_gws_ = IsFromGoogleSearchResult(url, referrer.url);
}
