#include "xgboost_predictor.h"
#include "xgboost/learner.h"
#include "simple_csr_source.h"

namespace xgb {

XGBoostPredictor::XGBoostPredictor() : learner_ptr_(xgboost::Learner::Create({})) {}

bool XGBoostPredictor::load(const std::string &model_file) {
  try {
    std::unique_ptr<dmlc::Stream> fi(dmlc::Stream::Create(model_file.c_str(), "r"));
    learner_ptr_->Load(fi.get());
    learner_ptr_->Configure({});  // must config,or core dump
    learner_ptr_->InitModel();
  }
  catch (std::exception &e) {
    return false;
  }

  return true;
}

bool XGBoostPredictor::predict(const std::vector<std::vector<std::pair<uint64_t, double> > > &rows,
                              std::vector<double> *preds) {
  if (rows.empty()) {
    return false;
  }

  std::unique_ptr<xgboost::DMatrix> matrix(createMatrix(rows));
  xgboost::HostDeviceVector<xgboost::bst_float> tmp_preds;
  try {
    learner_ptr_->Predict(matrix.get(), false, &tmp_preds);
  }
  catch (std::exception &e) {
    return false;
  }

  preds->reserve(tmp_preds.HostVector().size());
  preds->assign(tmp_preds.HostVector().begin(), tmp_preds.HostVector().end());
  return true;
}

xgboost::DMatrix *XGBoostPredictor::createMatrix(const std::vector<std::vector<std::pair<uint64_t, double> > > &rows) {
  std::unique_ptr<xgboost::data::SimpleCSRSource> source = std::make_unique<xgboost::data::SimpleCSRSource>();
  xgboost::data::SimpleCSRSource &mat = *source;
  auto &offset_vec = mat.page_.offset;
  auto &data_vec = mat.page_.data;
  offset_vec.resize(1 + rows.size());

  for (uint64_t i = 0; i < rows.size(); ++i) {
    offset_vec[i + 1] = offset_vec[i] + rows[i].size();
  }
  data_vec.reserve(offset_vec.back());

  uint64_t num_column = 0;
  for (auto &row : rows) {
    for (auto &v : row) {
      data_vec.emplace_back(xgboost::Entry(v.first, v.second));
      num_column = std::max(num_column, v.first + 1);
    }
  }
  mat.info.num_row_ = rows.size();
  mat.info.num_col_ = num_column;
  mat.info.num_nonzero_ = mat.page_.data.size();
  return xgboost::DMatrix::Create(std::move(source));
}

}  // namespace xgb

