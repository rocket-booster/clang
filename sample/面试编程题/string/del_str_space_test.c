// gcc del_str_space_test.c -o del_str_space_test && ./del_str_space_test
#include "del_str_space.hpp"

void remove_all_space_test(){
  {
  char s1[] = "  I am  K   ey    ";
  printf("s1: %s %lu", s1, strlen(s1)+1);
  remove_all_space(s1, sizeof(s1)/sizeof(char));
  printf(" --> %s %lu\n", s1, strlen(s1)+1);
  }

  {
  char s1[] = "IamKey";
  printf("s1: %s %lu", s1, strlen(s1)+1);
  remove_all_space(s1, sizeof(s1)/sizeof(char));
  printf(" --> %s %lu\n", s1, strlen(s1)+1);
  }  
}

int main(){
  remove_all_space_test();
}