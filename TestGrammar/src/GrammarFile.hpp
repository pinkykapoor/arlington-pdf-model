///////////////////////////////////////////////////////////////////////////////
// GrammarFile.hpp
// Copyright (c) 2020 Normex. All Rights Reserved.
///////////////////////////////////////////////////////////////////////////////

/*!
  A class to read grammar data from a csv file.
*/
 
#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>

#include "Pdfix.h"

  class CGrammarReader
{
  std::wstring file_name;
  std::string delimeter;
  std::vector<std::vector<std::string> > data_list;
  std::vector<std::string> basic_types = { "BOOLEAN", "NUMBER", "NAME",
    "STRING", "STREAM", "ARRAY", "DICTIONARY", "INTEGER", "DATE", "RECTANGLE" };
public:
  CGrammarReader(std::wstring f_name, std::string delm = "\t") :
    file_name(f_name), delimeter(delm)
  { }

  // Function to fetch data from a CSV File
  bool load();
  std::vector<std::vector<std::string> > get_data();
  bool check(std::ostream &report_stream);
};

/*
  Parses through csv file line by line and loads csv data 
  into data_list (vector of vector of strings)
  if there is wrong structure, returns false, else returns true
*/
bool CGrammarReader::load()
{
  std::ifstream file(file_name);
  std::string line = "";
  // Iterate through each line and split the content using delimeter
  while (getline(file, line))
  {
    std::vector<std::string> vec;
    std::string::size_type prev_pos = 0, pos = 0;
    while ((pos = line.find(delimeter, pos)) != std::string::npos) {
      std::string substring(line.substr(prev_pos, pos - prev_pos));
      vec.push_back(substring);
      prev_pos = ++pos;
    }
    vec.push_back(line.substr(prev_pos, pos - prev_pos)); // Last word
    // check first line - have to have at least 11 columns
    if (data_list.empty() && (vec.size() < 11)) {
      file.close();
      return false;
    }
    // Type, Required, Indirect - convert to uppercase
    std::transform(vec[1].begin(), vec[1].end(), vec[1].begin(), ::toupper);
    std::transform(vec[4].begin(), vec[4].end(), vec[4].begin(), ::toupper);
    std::transform(vec[5].begin(), vec[5].end(), vec[5].begin(), ::toupper);
    data_list.push_back(vec);
  }
  // Close the File
  file.close();
  return true;
}

/*
  Returns internal data_list (vector of vector of strings)
*/
std::vector<std::vector<std::string> > CGrammarReader::get_data()
{
  return data_list;
}

/*
 Checks the validity of the loaded CVS data
 - correct # of columns
 - correct headings (first line)
 - correct basic types 1st column
*/
bool CGrammarReader::check(std::ostream &report_stream) {
  if (data_list.empty()) {
    report_stream << "Empty grammar file:" << file_name.c_str() << std::endl;
    return false;
  }

  // check first line (heading)
  std::vector<std::string> vec = data_list[0];
  if (vec.size() < 11) {
    report_stream << "Wrong number of columns:" << file_name.c_str() << std::endl;
    return false;
  }

  if ((vec[0] != "Key") || (vec[1] != "TYPE") || (vec[2] != "SinceVersion") ||
    (vec[3] != "DeprecatedIn") || (vec[4] != "REQUIRED") || (vec[5] != "INDIRECTREFRENCE") ||
    (vec[6] != "RequiredValue") || (vec[7] != "DefaultValue") || (vec[8] != "PossibleValues") ||
    (vec[9] != "SpecialCase") || (vec[10] != "Link")) {
    report_stream << "Wrong number of columns:" << file_name.c_str() << std::endl;
    return false;
  }

  // check basic types
  for (int i = 1; i < data_list.size(); i++) {
    std::vector<std::string> vc = data_list[i];

    auto is_basic_type = [=](std::string str) {
      return std::find(basic_types.begin(), basic_types.end(), str) != basic_types.end();
    };

    // possible multiple types separated with ";"
    std::string::size_type prev_pos = 0, pos = 0;
    while ((pos = vc[1].find(";", pos)) != std::string::npos) {
      std::string substring(vc[1].substr(prev_pos, pos - prev_pos));
      if (!is_basic_type(substring))
        report_stream << "Wrong type:" << substring << " in:" << file_name.c_str() << "::" << vc[0] << std::endl;
      prev_pos = ++pos;
    }

    if (!is_basic_type(vc[1].substr(prev_pos, pos - prev_pos)))
      report_stream << "Wrong type:" << vc[1].substr(prev_pos, pos - prev_pos) << " in:" << file_name.c_str() << "::" << vc[0] << std::endl;
  }
  return true;
}
