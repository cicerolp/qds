//
// Created by cicerolp on 11/2/17.
//

#pragma once

#include "AVLTreeInterface.h"

class AVLGroupTree {
 public:

 private:
  // AVLGroupTree
  double _centroid;
  int32_t _count;
  std::vector<double> _data;

  std::vector<double> _centroids;
  std::vector<int32_t> _counts;
  std::vector<double> _datas;
  std::vector<int32_t> _aggregatedCounts;
};

