/**************************************************
 * Copyright (c) 2025 Wenbo Li
 * University of Science and Technology of China
 *
 * This file is part of the SafeMRC project.
 * Distributed under the MIT License.
 **************************************************/

#include "safe_mrc_device/rs485bus/bus_diagnostics.hpp"

#include <algorithm>

namespace safe_mrc {

void BusDiagnostics::register_device(uint8_t id) { ensure_device(id); }

void BusDiagnostics::record_tx_attempt(uint8_t id) {
  auto& stats = ensure_device(id);
  ++stats.tx_attempts;
}

void BusDiagnostics::record_tx_success(uint8_t id) {
  auto& stats = ensure_device(id);
  ++stats.tx_successes;
}

void BusDiagnostics::record_rx_attempt(uint8_t id) {
  auto& stats = ensure_device(id);
  ++stats.rx_attempts;
}

void BusDiagnostics::record_rx_success(uint8_t id) {
  auto& stats = ensure_device(id);
  ++stats.rx_successes;
}

std::vector<BusDiagnostics::DeviceReport> BusDiagnostics::collect_reports()
    const {
  std::lock_guard<std::mutex> lock(mutex_);
  std::vector<DeviceReport> reports;
  reports.reserve(stats_.size());
  for (const auto& [id, stats] : stats_) {
    auto tx_attempts = stats.tx_attempts;
    auto rx_attempts = stats.rx_attempts;
    double tx_rate = tx_attempts > 0
                         ? static_cast<double>(stats.tx_successes) /
                               static_cast<double>(tx_attempts)
                         : 0.0;
    double rx_rate = rx_attempts > 0
                         ? static_cast<double>(stats.rx_successes) /
                               static_cast<double>(rx_attempts)
                         : 0.0;
    reports.push_back(DeviceReport{id,        tx_attempts, stats.tx_successes,
                                  rx_attempts, stats.rx_successes, tx_rate,
                                  rx_rate});
  }
  std::sort(reports.begin(), reports.end(),
            [](const DeviceReport& lhs, const DeviceReport& rhs) {
              return lhs.id < rhs.id;
            });
  return reports;
}

BusDiagnostics::DeviceStats& BusDiagnostics::ensure_device(uint8_t id) const {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = stats_.find(id);
  if (it == stats_.end()) {
    auto [inserted_it, _] = stats_.emplace(id, DeviceStats{id});
    return inserted_it->second;
  }
  return it->second;
}

}  // namespace safe_mrc

