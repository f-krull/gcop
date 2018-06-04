#ifndef SRC_CORRELATION_H_
#define SRC_CORRELATION_H_

#include <stdint.h>
#include <algorithm>
#include <math.h>
#include <vector>
#include <assert.h>

static double avg(const std::vector<double> &x) {
  return std::accumulate(x.begin(), x.end(), 0.0f) / x.size();
}

/*----------------------------------------------------------------------------*/

static double stddev(const std::vector<double> &x, double avgX) {
  double stdDev = 0;
  //double avgX = variance(x);

  for (uint32_t i = 0; i < x.size(); ++i) {
    stdDev += pow(x[i] - avgX, 2.);
  }
  return sqrt((stdDev / x.size()));
}

/*----------------------------------------------------------------------------*/

static double covar(const std::vector<double> &x,
    const std::vector<double> &y, double avgX, double avgY) {
  double cov = 0;

  assert(x.size() == y.size());
  for (uint32_t i = 0; i < x.size(); ++i) {
    cov += (x[i] - avgX) * (y[i] - avgY);
  }
  return cov / x.size();
}

/*----------------------------------------------------------------------------*/

double corrPearson(const std::vector<double> &x, const std::vector<double> &y, double avgx, double avgy, double stddevx, double stddevy) {
  const double numerator = covar(x, y, avgx, avgy);
  const double denominator = stddevx * stddevy;
  /* dont't divide by zero. return 0 in those cases */
  return denominator != 0.0f ? numerator / denominator : 0;
}

/*----------------------------------------------------------------------------*/

double corrPearson(const std::vector<double> &x, const std::vector<double> &y) {
  const double avgX = avg(x);
  const double avgY = avg(y);
  const double stddevX = stddev(x, avgX);
  const double stddevY = stddev(y, avgY);
  return corrPearson(x, y, avgX, avgY, stddevX, stddevY);
}

/*----------------------------------------------------------------------------*/

static bool cmpSmaller(const std::pair<double, uint32_t>& a, const std::pair<
    double, uint32_t>& b) {
   return a.first < b.first;
}

static std::vector<uint32_t> rank(const std::vector<double> &c) {
   std::vector<uint32_t> rank;
   rank.resize(c.size());

   std::vector<std::pair<double, uint32_t> > tmp;
   tmp.resize(c.size());
   for (uint32_t i = 0; i < c.size(); ++i) {
      tmp[i] = (std::pair<double, uint32_t>(c[i], i));
   }
   std::sort(tmp.begin(), tmp.end(), cmpSmaller);
   assert(tmp.size() == c.size());

   std::vector<std::pair<double, uint32_t>> tmp2;
   tmp2.resize(c.size());
   for (uint32_t i = 0; i < c.size(); ++i) {
      tmp2[i] = (std::pair<double, uint32_t>(tmp[i].second, i));
   }
   std::sort(tmp2.begin(), tmp2.end(), cmpSmaller);
   assert(tmp2.size() == c.size());

   for (uint32_t i = 0; i < c.size(); ++i) {
      rank[i] = tmp2[i].second + 1;
   }
   assert(rank.size() == c.size());
   return rank;
}

/*----------------------------------------------------------------------------*/

double corrSpearsRanked(const std::vector<uint32_t> &rx, const std::vector<uint32_t> &ry) {
  double sum = 0;
   assert(rx.size() == ry.size());
   for (uint32_t i = 0; i < rx.size(); ++i) {
      sum += pow(rx[i] - (double) ry[i], 2);
   }
   return 1 - ((6 * sum) / (rx.size() * (pow(rx.size(), 2) - 1)));
}

/*----------------------------------------------------------------------------*/

double corrSpears(const std::vector<double> &x, const std::vector<double> &y) {
  assert(x.size() == y.size());
  std::vector<uint32_t> rx = rank(x);
  std::vector<uint32_t> ry = rank(y);
  assert(rx.size() == x.size());
  assert(ry.size() == x.size());
  double res = corrSpearsRanked(rx, ry);
  return res;
}


#endif /* SRC_CORRELATION_H_ */
