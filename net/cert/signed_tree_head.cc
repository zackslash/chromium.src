// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/cert/signed_tree_head.h"

#include <string.h>

#include "base/strings/string_number_conversions.h"

namespace net {
namespace ct {

SignedTreeHead::SignedTreeHead() {}

SignedTreeHead::SignedTreeHead(Version version,
                               const base::Time& timestamp,
                               uint64_t tree_size,
                               const char sha256_root_hash[kSthRootHashLength],
                               const DigitallySigned& signature,
                               const std::string& log_id)
    : version(version),
      timestamp(timestamp),
      tree_size(tree_size),
      signature(signature),
      log_id(log_id) {
  memcpy(this->sha256_root_hash, sha256_root_hash, kSthRootHashLength);
}

SignedTreeHead::~SignedTreeHead() {}

std::ostream& operator<<(std::ostream& stream, const SignedTreeHead& sth) {
  return stream << "{\n"
                << "\t\"version\": " << sth.version << ",\n"
                << "\t\"timestamp\": " << sth.timestamp << ",\n"
                << "\t\"tree_size\": " << sth.tree_size << ",\n"
                << "\t\"sha256_root_hash\": \""
                << base::HexEncode(sth.sha256_root_hash, kSthRootHashLength)
                << "\",\n\t\"log_id\": \""
                << base::HexEncode(sth.log_id.data(), sth.log_id.size())
                << "\"\n"
                << "}";
}

}  // namespace ct
}  // namespace net
