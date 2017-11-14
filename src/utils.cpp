#include "utils.h"
std::vector<std::string>& split(const std::string& s, const std::string& delims,
                                std::vector<std::string>& elems) {
  char* tok;
  char cchars[s.size() + 1];
  char* cstr = &cchars[0];
  strcpy(cstr, s.c_str());
  tok = strtok(cstr, delims.c_str());
  while (tok != NULL) {
    elems.push_back(tok);
    tok = strtok(NULL, delims.c_str());
  }
  return elems;
}
