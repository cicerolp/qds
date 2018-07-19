//
// Created by cicerolp on 5/23/18.
//

#include "PDigest.h"

#pragma once

namespace ks {

static float distance(AgrrPDigest *lhs, AgrrPDigest *rhs) {
  // Kolmogorov–Smirnov test
  auto c1 = lhs->get_centroids();
  auto c2 = rhs->get_centroids();

  auto c1_end = c1.begin() + lhs->get_size();
  auto c2_end = c2.begin() + rhs->get_size();

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // method 1
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  /*for (auto iter = c1.begin(); iter < c1_end; ++iter) {
    distance = std::max(distance, std::fabs(lhs->inverse(*iter) - rhs->inverse(*iter)));
    if (distance == 1.f) break;
  }

  for (auto iter = c2.begin(); iter < c2_end; ++iter) {
    distance = std::max(distance, std::fabs(lhs->inverse(*iter) - rhs->inverse(*iter)));
    if (distance == 1.f) break;
  }*/

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // method 2
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // first c1 centroid that is not less than c2.front
  auto c1_c2 = std::lower_bound(c1.begin(), c1_end, c2.front(), [](const auto &lhs, const auto &rhs) {
    return lhs.mean < rhs.mean;
  });

  // first c2 centroid that is not less than c1.front
  auto c2_c1 = std::lower_bound(c2.begin(), c2_end, c1.front(), [](const auto &lhs, const auto &rhs) {
    return lhs.mean < rhs.mean;
  });

  auto distance = 0.f;

  if (c1_c2 == c1_end || c2_c1 == c2_end) {
    distance = 1.f;
  } else {
    distance = std::max(distance, std::fabs(lhs->inverse((*c1_c2).mean) - rhs->inverse((*c1_c2).mean)));
    distance = std::max(distance, std::fabs(lhs->inverse((*c2_c1).mean) - rhs->inverse((*c2_c1).mean)));

    for (auto iter = c1_c2; iter < c1_end; ++iter) {
      if (distance == 1.f) break;
      distance = std::max(distance, std::fabs(lhs->inverse((*iter).mean) - rhs->inverse((*iter).mean)));
    }

    for (auto iter = c2_c1; iter < c2_end; ++iter) {
      if (distance == 1.f) break;
      distance = std::max(distance, std::fabs(lhs->inverse((*iter).mean) - rhs->inverse((*iter).mean)));
    }
  }

  return distance;
}

static float distance_w(AgrrPDigest *lhs, AgrrPDigest *rhs, float weight) {
  auto distance = ks::distance(lhs, rhs);
  return distance * 2.f;
}

} // namespace ks

namespace polar {

static float distance(AgrrPDigest *lhs, AgrrPDigest *rhs) {
  static const float radius = 0.5f;

  auto theta_c1 = lhs->get_denser_sector();
  auto x1 = radius * std::cos(theta_c1);
  auto y1 = radius * std::sin(theta_c1);

  auto theta_c2 = rhs->get_denser_sector();
  auto x2 = radius * std::cos(theta_c2);
  auto y2 = radius * std::sin(theta_c2);

  auto distance = std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));

  return distance;
}

} // namespace sector