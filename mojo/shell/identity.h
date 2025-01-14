// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SHELL_IDENTITY_H_
#define MOJO_SHELL_IDENTITY_H_

#include "mojo/shell/capability_filter.h"
#include "url/gurl.h"

namespace mojo {
namespace shell {

// Represents the identity of an application.
// |url| is the URL of the application.
// |qualifier| is a string that allows to tie a specific instance of an
// application to another. A typical use case of qualifier is to control process
// grouping for a given application URL. For example, the core services are
// grouped into "Core"/"Files"/"Network"/etc. using qualifier; content handler's
// qualifier is derived from the origin of the content.
class Identity {
 public:
  Identity();
  // Assumes user = mojom::Connector::kUserRoot.
  // Used in tests or for shell-initiated connections.
  explicit Identity(const GURL& in_url);
  Identity(const GURL& in_url,
           const std::string& in_qualifier,
           uint32_t user_id);
  ~Identity();

  bool operator<(const Identity& other) const;
  bool is_null() const { return url_.is_empty(); }
  bool operator==(const Identity& other) const;

  const GURL& url() const { return url_; }
  uint32_t user_id() const { return user_id_; }
  void set_user_id(uint32_t user_id) { user_id_ = user_id; }
  const std::string& qualifier() const { return qualifier_; }
  void SetFilter(const CapabilityFilter& filter);
  const CapabilityFilter& filter() const { return filter_; }

 private:
  GURL url_;
  std::string qualifier_;

  uint32_t user_id_;

  // TODO(beng): CapabilityFilter is not currently included in equivalence
  //             checks for Identity since we're not currently clear on the
  //             policy for instance disambiguation. Need to figure this out.
  //             This field is supplied because it is logically part of the
  //             instance identity of an application.
  CapabilityFilter filter_;
};

// Creates an identity for the Shell, used when the Shell connects to
// applications.
Identity CreateShellIdentity();

}  // namespace shell
}  // namespace mojo

#endif  // MOJO_SHELL_IDENTITY_H_
