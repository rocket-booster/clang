/*
 * @Author: RockyWu
 * @Date: 2022-05-22 17:34:43
 * @LastEditors: RockyWu
 * @LastEditTime: 2022-05-22 22:47:22
 * @FilePath: /clang/sample/面试编程题/string/del_str_space.hpp
 * @Description: 
 *  删除字符串中的空格, 能创建简单的自变量, 不能创建多余的内存空间,时间复杂度为O(n)
 * Copyright (c) 2022 by RockyWu/Personal, All Rights Reserved. 
 */
#include <stdio.h>
#include <string.h>


// from_富途
/*
题目:
    删除字符串中的空格, 能创建简单的自变量, 不能创建多余的内存空间
    空间复杂度为O(1)
    时间复杂度为O(n)
题意:
    1.提示用自变量, 不用额外内存
    2.提示用一次循环, 多个指针
*/
void remove_all_space(char* str, int size){
  if (str == NULL || size <= 0 ) return;
  
  int j = 0;
  for (int i = 0; i < size; i++){
    if (str[i] != ' ' && str[i] != '\0'){
      str[j] = str[i];
      j++;
    }else{
      continue;
    }
  }
  str[j] = '\0';
}