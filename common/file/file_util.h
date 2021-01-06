#pragma once

#include <vector>
#include <string>

namespace common {

class FileUtil {
 public:
  static bool is_file_exist(const std::string &full_file_name);

  static bool read_file_str(std::string *out_str, const std::string &full_file_name);

  static bool read_file_all_lines(std::vector<std::string> *line_vec, const std::string &full_file_name);

  // file name including extension
  static std::string extract_file_name(const std::string &full_file_name);

  // file name without extension
  static std::string extract_file_name_noextension(const std::string &full_file_name);

  // file extension
  static std::string extract_file_extension(const std::string &full_file_name);

  // dir file is in
  static std::string extract_dir(const std::string &full_file_name);

  static void list_dir(std::vector<std::string> *file_vec, const std::string &full_path_name);

  static bool is_file_with_suffix(const std::string &full_file_name, const std::string &suffix);

  static bool is_directory(const std::string &path_name);

  static bool create_file(const std::string &file_path);

  static bool remove_dir(const std::string& dir_name);
};

}  // namespace common
