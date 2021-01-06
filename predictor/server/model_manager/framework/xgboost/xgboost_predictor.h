#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <memory>

/**
 * forward declaration
 */
namespace xgboost {
class Learner;
class DMatrix;
}

namespace xgb {
/**
 * Wrapper class of Xgboost.
 * Usage:
 * const string model_file = "example.model";
 * XGBoostPredictor predictor;
 * if (!predictor.load(model_file)) {
 *  ...
 * }
 * std::vector<std::vector<std::pair<uint64_t, double> > > rows;
 * // generate samples to rows,  pair<index, value>
 * std::vector<double> result;
 * if (!predictor.predict(rows, &result)) {
 *  ...
 * }
 */
class XGBoostPredictor {
 public:
  XGBoostPredictor();
  ~XGBoostPredictor() = default;
  XGBoostPredictor(const XGBoostPredictor &) = delete;
  XGBoostPredictor(XGBoostPredictor &&) = delete;
  void operator=(const XGBoostPredictor &) = delete;
  void operator=(XGBoostPredictor &&) = delete;

  bool load(const std::string &model_file);
  /**
   * rows: multi rows of samples, every element of a row is <index, value>
   * preds: predicted results to pass back
   */
  bool predict(const std::vector<std::vector<std::pair<uint64_t, double> > > &rows, std::vector<double> *preds);

 private:
  /**
   * transform rows to matrix needed by xgboost
   */
  xgboost::DMatrix *createMatrix(const std::vector<std::vector<std::pair<uint64_t, double> > > &rows);

 private:
  std::shared_ptr<xgboost::Learner> learner_ptr_;
};

}  // namespace xgb

