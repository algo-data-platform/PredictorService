#include "common/file/file_util.h"


#include "common/util.h"
#include "folly/FileUtil.h"
#include "boost/filesystem.hpp"

namespace common {

namespace {

constexpr char LOG_CATEGORY[] = "file_util.cc";

}  // namespace

bool FileUtil::is_file_exist(const std::string &full_file_name) {
  boost::filesystem::path full_path(full_file_name);
  return boost::filesystem::exists(full_path);
}

bool FileUtil::read_file_str(std::string *out_str, const std::string &full_file_name) {
  return folly::readFile(full_file_name.c_str(), *out_str);
}

bool FileUtil::read_file_all_lines(std::vector<std::string> *line_vec, const std::string &full_file_name) {
  std::ifstream input;
  input.open(full_file_name);
  if (!input.is_open()) {
    LOG(ERROR) << "Fail to open file " << full_file_name;
    return false;
  }
  SCOPE_EXIT { input.close(); };
  std::string line;
  while (!input.eof()) {
    std::getline(input, line);
    // remove all space char
    auto it = std::remove_if(line.begin(), line.end(), ::isspace);
    line.erase(it, line.end());
    if (line.empty()) {
      continue;
    }
    line_vec->emplace_back(line);
  }
  return true;
}

std::string FileUtil::extract_file_name(const std::string &full_file_name) {
  boost::filesystem::path full_path(full_file_name);
  return full_path.filename().string();
}

std::string FileUtil::extract_file_name_noextension(const std::string &full_file_name) {
  boost::filesystem::path full_path(full_file_name);
  return full_path.stem().string();
}

std::string FileUtil::extract_file_extension(const std::string &full_file_name) {
  boost::filesystem::path full_path(full_file_name);
  return full_path.extension().string();
}

std::string FileUtil::extract_dir(const std::string &full_file_name) {
  boost::filesystem::path full_path(full_file_name);
  return full_path.parent_path().string();
}

void FileUtil::list_dir(std::vector<std::string> *file_vec, const std::string &full_path_name) {
  if (!is_file_exist(full_path_name)) {
    return;
  }
  boost::filesystem::path full_path(full_path_name);
  for (boost::filesystem::directory_entry &x : boost::filesystem::directory_iterator(full_path)) {
    (*file_vec).emplace_back(x.path().string());
  }
}

bool FileUtil::is_file_with_suffix(const std::string &full_file_name, const std::string &suffix) {
  boost::filesystem::path full_path(full_file_name);
  return (full_path.extension().string() == suffix);
}

bool FileUtil::is_directory(const std::string &path_name) {
  return boost::filesystem::is_directory(path_name);
}

bool FileUtil::create_file(const std::string &file_path) {
  boost::filesystem::path file(file_path);
  return boost::filesystem::create_directory(file);
}

bool FileUtil::remove_dir(const std::string &dir_name) {
  boost::filesystem::path dir(dir_name);
  try {
    boost::filesystem::remove_all(dir);
  }
  catch (boost::filesystem::filesystem_error e) {
    AD_LOG(ERROR) << "remove_dir caught an exception: " << e.what();
    return false;
  }
  return true;
}

}  // namespace common
