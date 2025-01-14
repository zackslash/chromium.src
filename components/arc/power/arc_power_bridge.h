// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ARC_POWER_ARC_POWER_BRIDGE_H_
#define COMPONENTS_ARC_POWER_ARC_POWER_BRIDGE_H_

#include <map>

#include "base/macros.h"
#include "components/arc/arc_bridge_service.h"
#include "components/arc/arc_service.h"
#include "mojo/public/cpp/bindings/binding.h"

namespace arc {

// ARC Power Client sets power management policy based on requests from
// ARC instances.
class ArcPowerBridge : public ArcService,
                       public ArcBridgeService::Observer,
                       public PowerHost {
 public:
  explicit ArcPowerBridge(ArcBridgeService* bridge_service);
  ~ArcPowerBridge() override;

  // ArcBridgeService::Observer overrides.
  void OnStateChanged(ArcBridgeService::State state) override;
  void OnPowerInstanceReady() override;

  // PowerHost overrides.
  void OnAcquireDisplayWakeLock(DisplayWakeLockType type) override;
  void OnReleaseDisplayWakeLock(DisplayWakeLockType type) override;

  void IsDisplayOn(const IsDisplayOnCallback& callback) override;

 private:
  void ReleaseAllDisplayWakeLocks();

  mojo::Binding<PowerHost> binding_;

  // Stores a mapping of type -> wake lock ID for all wake locks
  // held by ARC.
  std::multimap<DisplayWakeLockType, int> wake_locks_;

  DISALLOW_COPY_AND_ASSIGN(ArcPowerBridge);
};

}  // namespace arc

#endif  // COMPONENTS_ARC_POWER_ARC_POWER_BRIDGE_H_
