/*
 * @Author: RockyWu
 * @Date: 2022-02-25 17:34:43
 * @LastEditors: RockyWu
 * @LastEditTime: 2022-02-25 22:47:22
 * @FilePath: /gitee/clang/sample/boost/process_strings_with_regular_expressions.cpp
 * @Description: 
 * 
 * Copyright (c) 2022 by RockyWu/Personal, All Rights Reserved. 
 * sudo apt install libboost1.65-all-dev
 * g++ process_strings_with_regular_expressions.cpp -std=c++17 -I.. -lboost_regex -o a.out;./a.out
 */

#include <iostream>
#include <string>

#if 0
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#else
#include "../../core/boost.hpp"
#endif 

/**
 * @description: 指定特定字符集, 进行删除
 * @param {string} &str 待处理字符串
 * @param {string} rm_pattern 待删除字符正则表达式
 * @return {*}
 */
void roc_boost_trim(std::string &str, std::string pattern){
  boost::regex expression(pattern); // 格式: " |\n|\r|\t" or "B|\\s|A", 支持转义 ex: 空格[\\s] 
  std::cout << __FUNCTION__ << " - " << __LINE__ << " str:" << str << std::endl;  
  str = boost::regex_replace(str, expression, "");
  std::cout << __FUNCTION__ << " - " << __LINE__ << " str:" << str << std::endl;
}

/**
 * @description: 删除用例
 * @param {*}
 * @return {*}
 */
int sample_boost_trim(){
  std::string str = "A B C";
      
  // str = "A B C"
  roc_boost_trim(str, " |B"); 
  // str = "AC"  
  
  return 0;
}

/**
 * @description: 指定特定格式的表达式，进行提取子串
 * @param {string} str  待处理字符串
 * @param {string} pattern 待提取子串格式的正则表达式
 * @param {int} max 最多需要返回max个子串
 * @return {*}
 */
std::vector<std::string> roc_boost_search(std::string str, std::string pattern, int max){
  
  std::vector<std::string> v_re;
    
  boost::smatch what;

  std::string::const_iterator begin = str.begin();
  std::string::const_iterator end = str.end();
  boost::regex expression(pattern);

  std::cout << __FUNCTION__ << " - " << __LINE__ << " pattern:" << pattern << std::endl;
  int idx = 0;
  while(boost::regex_search(begin, end, what, expression)){
    const std::string& msg = what[0];
    begin = what[0].second;
    v_re.emplace_back(msg);
    idx++;

    std::cout << __FUNCTION__ << " - " << __LINE__ << " [" << idx << "] :" << msg << std::endl; 

    if (idx >= max) return v_re;    
  }

  return v_re;
}

/**
 * @description: 字符串格式提取/提取
 * @param {*}
 * @return {*}
 */
int sample_boost_search(){
  std::string str = "SSS={'k1':'xxx1', 'k2':'xxx2', 'k3':'xxx3'}";
  std::string target = "";

  roc_boost_trim(str, " ");

  // step1: 检查格式
  std::string pattern = "[s|S]+=\\{'k1':'+[0-9|a-z|A-Z]+','k2':'+[0-9|a-z|A-Z]+','k3':'+[0-9|a-z|A-Z]+'";  
  auto re = roc_boost_search(str, pattern, 1);
  if (re.size() > 0){
    // step2: 提取 k3键值对 的 值
    std::string pattern_k3 = "'k3':'+[0-9|a-z|A-Z]+'";
    re = roc_boost_search(str, pattern_k3, 1);
    if (re.size() > 0){
      std::string str_k3 = re[0];
      std::string pattern_value = "'+[0-9|a-z|A-Z]+'";
      re = roc_boost_search(str_k3, pattern_value, 2);
      if (re.size() >= 2){
        target = re[1];
      }      
    }
  }      
}
int main(){

  std::cout << std::endl << std::endl << __FUNCTION__ << " - " << __LINE__ << " === delete === " << std::endl; 
  sample_boost_trim();
  std::cout << __FUNCTION__ << " - " << __LINE__ << " === end ===" << std::endl;

  std::cout << std::endl << std::endl << __FUNCTION__ << " - " << __LINE__ << " === search or check === " << std::endl;
  sample_boost_search();
  std::cout << __FUNCTION__ << " - " << __LINE__ << " === end ===" << std::endl;
  return 0;
}



