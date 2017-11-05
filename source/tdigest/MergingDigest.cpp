//
// Created by cicerolp on 11/4/17.
//

#include "stdafx.h"
#include "MergingDigest.h"

// TODO tdigest get mean from data
void MergingDigest::add(NDS &nds, uint32_t front, uint32_t back) {
  std::vector<double> inMean;
  inMean.reserve(back - front);

  for (auto p = front; p < back; ++p) {
    inMean.emplace_back(rand() % 1001);
  }

  std::vector<double> inWeight(back - front, 1);

  add(inMean, inWeight);
}

void MergingDigest::add(std::vector<double> inMean, std::vector<double> inWeight) {
  inMean.insert(inMean.end(), _mean.begin(), _mean.end());
  inWeight.insert(inWeight.end(), _weight.begin(), _weight.end());

  int32_t incomingCount = inMean.size();

  auto inOrder = sort_indexes(inMean);

  double totalWeight = std::accumulate(inWeight.begin(), inWeight.end(), 0.0);

  double normalizer = TDIGEST_COMPRESSION / (M_PI * totalWeight);

  // points to the first unused centroid
  _mean.clear();
  _weight.clear();

  //_lastUsedCell = 0;
  _mean.emplace_back(inMean[inOrder[0]]);
  _weight.emplace_back(inWeight[inOrder[0]]);

  double wSoFar = 0;

  double k1 = 0;

  // weight will contain all zeros
  double wLimit;
  wLimit = totalWeight * integratedQ(k1 + 1);

  for (int i = 1; i < incomingCount; i++) {
    int ix = inOrder[i];
    double proposedWeight = _weight.back() + inWeight[ix];
    //double proposedWeight = _weight[_lastUsedCell] + inWeight[ix];

    double projectedW = wSoFar + proposedWeight;

    bool addThis = false;
    if (useWeightLimit) {
      double z = proposedWeight * normalizer;
      double q0 = wSoFar / totalWeight;
      double q2 = (wSoFar + proposedWeight) / totalWeight;
      addThis = z * z <= q0 * (1 - q0) && z * z <= q2 * (1 - q2);
    } else {
      addThis = projectedW <= wLimit;
    }

    if (addThis) {
      // next point will fit
      // so merge into existing centroid
      _weight.back() += inWeight[ix];
      _mean.back() = _mean.back()
          + (inMean[ix] - _mean.back()) * inWeight[ix] / _weight.back();

      //_weight[_lastUsedCell] += inWeight[ix];
      //_mean[_lastUsedCell] = _mean[_lastUsedCell]
      //+ (inMean[ix] - _mean[_lastUsedCell]) * inWeight[ix] / _weight[_lastUsedCell];

      inWeight[ix] = 0;

    } else {
      // didn't fit ... move to next output, copy out first centroid
      //wSoFar += _weight[_lastUsedCell];
      wSoFar += _weight.back();

      if (!useWeightLimit) {
        k1 = integratedLocation(wSoFar / totalWeight);
        wLimit = totalWeight * integratedQ(k1 + 1);
      }

      _mean.emplace_back(inMean[ix]);
      _weight.emplace_back(inWeight[ix]);
      //_lastUsedCell++;
      //_mean[_lastUsedCell] = inMean[ix];
      //_weight[_lastUsedCell] = inWeight[ix];
      inWeight[ix] = 0;
    }
  }
  // points to next empty cell
  //_lastUsedCell++;

  if (totalWeight > 0) {
    _min = std::min(_min, _mean[0]);
    _max = std::max(_max, _mean.back());
    //_max = std::max(_max, _mean[_lastUsedCell - 1]);
  }
}

void MergingDigest::merge(const MergingDigest &other) {
  //other.compress();

  add(other._mean, other._weight);
}

double MergingDigest::quantile(double q) {
  //if (_lastUsedCell == 0 && _weight[_lastUsedCell] == 0) {
  if (_mean.size() == 0) {
    // no centroids means no data, no way to get a quantile
    return std::numeric_limits<double>::quiet_NaN();

    //} else if (_lastUsedCell == 0) {
  } else if (_mean.size() == 1) {
    // with one data point, all quantiles lead to Rome
    return _mean[0];
  }

  // we know that there are at least two centroids now
  //int32_t n = _lastUsedCell;
  int32_t n = _mean.size();

  double totalWeight = std::accumulate(_weight.begin(), _weight.end(), 0.0);

  // if values were stored in a sorted array, index would be the offset we are interested in
  const double index = q * totalWeight;

  // at the boundaries, we return min or max
  if (index < _weight[0] / 2) {
    return _min + 2 * index / _weight[0] * (_mean[0] - _min);
  }

  // in between we interpolate between centroids
  double weightSoFar = _weight[0] / 2;

  for (auto i = 0; i < n - 1; ++i) {
    double dw = (_weight[i] + _weight[i + 1]) / 2;

    if (weightSoFar + dw > index) {
      // centroids i and i+1 bracket our current point
      double z1 = index - weightSoFar;
      double z2 = weightSoFar + dw - index;
      return weightedAverage(_mean[i], z2, _mean[i + 1], z1);
    }

    weightSoFar += dw;
  }

  // weightSoFar = totalWeight - weight[n-1]/2 (very nearly)
  // so we interpolate out to max value ever seen
  double z1 = index - totalWeight - _weight[n - 1] / 2.0;
  double z2 = _weight[n - 1] / 2 - z1;

  return weightedAverage(_mean[n - 1], z1, _max, z2);
}

double MergingDigest::asinApproximation(double x) {
  if (usePieceWiseApproximation) {
    if (x < 0) {
      return -asinApproximation(-x);
    } else {
      // this approximation works by breaking that range from 0 to 1 into 5 regions
      // for all but the region nearest 1, rational polynomial models get us a very
      // good approximation of asin and by interpolating as we move from region to
      // region, we can guarantee continuity and we happen to get monotonicity as well.
      // for the values near 1, we just use Math.asin as our region "approximation".

      // cutoffs for models. Note that the ranges overlap. In the overlap we do
      // linear interpolation to guarantee the overall result is "nice"
      double c0High = 0.1;
      double c1High = 0.55;
      double c2Low = 0.5;
      double c2High = 0.8;
      double c3Low = 0.75;
      double c3High = 0.9;
      double c4Low = 0.87;
      if (x > c3High) {
        return std::asin(x);
      } else {
        // the models
        double m0[] = {0.2955302411, 1.2221903614, 0.1488583743, 0.2422015816, -0.3688700895, 0.0733398445};
        double m1[] = {-0.0430991920, 0.9594035750, -0.0362312299, 0.1204623351, 0.0457029620, -0.0026025285};
        double
            m2[] = {-0.034873933724, 1.054796752703, -0.194127063385, 0.283963735636, 0.023800124916, -0.000872727381};
        double m3[] = {-0.37588391875, 2.61991859025, -2.48835406886, 1.48605387425, 0.00857627492, -0.00015802871};

        // the parameters for all of the models
        double vars[] = {1, x, x * x, x * x * x, 1 / (1 - x), 1 / (1 - x) / (1 - x)};

        // raw grist for interpolation coefficients
        double x0 = bound((c0High - x) / c0High);
        double x1 = bound((c1High - x) / (c1High - c2Low));
        double x2 = bound((c2High - x) / (c2High - c3Low));
        double x3 = bound((c3High - x) / (c3High - c4Low));

        // interpolation coefficients
        //noinspection UnnecessaryLocalVariable
        double mix0 = x0;
        double mix1 = (1 - x0) * x1;
        double mix2 = (1 - x1) * x2;
        double mix3 = (1 - x2) * x3;
        double mix4 = 1 - x3;

        // now mix all the results together, avoiding extra evaluations
        double r = 0;
        if (mix0 > 0) {
          r += mix0 * eval(m0, vars);
        }
        if (mix1 > 0) {
          r += mix1 * eval(m1, vars);
        }
        if (mix2 > 0) {
          r += mix2 * eval(m2, vars);
        }
        if (mix3 > 0) {
          r += mix3 * eval(m3, vars);
        }
        if (mix4 > 0) {
          // model 4 is just the real deal
          r += mix4 * std::asin(x);
        }
        return r;
      }
    }
  } else {
    return std::asin(x);
  }
}



