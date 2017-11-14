#ifndef __UTILS_H__
#define __UTILS_H__

#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>
using namespace std;

std::vector<std::string> &split(const std::string &s, const std::string &delims,
                                std::vector<std::string> &elems);
#endif
