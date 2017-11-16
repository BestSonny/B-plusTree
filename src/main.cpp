#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <assert.h>
#include <typeinfo>
#include "BplusTree.cpp"
#include "utils.h"
#include <map>
using namespace std;

#define DELIMITERS "(),"

int main(int argc, char* argv[]) {
  ifstream file(argv[1]);  // pass file name as argment
  string linebuffer;
  vector<string> elems;
  float key;
  string value;
  string r;
  ofstream myfile("output_file.txt", std::ofstream::out);
  BPlusTree<float, string, 5, 5> bt;
  while (file && getline(file, linebuffer)) {
    if (linebuffer.length() == 0) continue;
    split(linebuffer, DELIMITERS, elems);
    string operation = elems.at(0);
    if (operation == "Insert") {
      assert(elems.size() == 3);
      key = stof(elems.at(1));
      value = elems.at(2);
      bt.insert(key, value);
    } else if (operation == "Search") {
      assert(elems.size() == 3 || elems.size() == 2);
      if (elems.size() == 2) {
        multimap<float, string> res;
        bt.getRange(stof(elems.at(1)), stof(elems.at(1)), res);
        if (res.size() == 0) {
          std::cout << "Null" << std::endl;
          myfile << "Null" << std::endl;
        } else {
          multimap<float, string>::iterator it;
          for (it = res.begin(); it != res.end(); ++it){
            std::cout << it->second << ", ";
            myfile << it->second << ", ";
          }
          myfile.seekp(-2,myfile.cur);
          myfile  << std::endl;
          std::cout << std::endl;
        }
      } else {
          multimap<float, string> res;
          bt.getRange(stof(elems.at(1)), stof(elems.at(2)), res);
          multimap<float, string>::iterator it;
          for (it = res.begin(); it != res.end(); ++it){
            myfile << "(" << it->first << ", " << it->second << "), ";
            std::cout << "(" << it->first << ", " << it->second << "), ";
          }
          myfile.seekp(-2,myfile.cur);
          myfile  << std::endl;
          std::cout << std::endl;
      }
    } else {
      cout << "error" << endl;
    }
    elems.clear();
  }
  file.close();
  myfile.close();

  return 0;
}
