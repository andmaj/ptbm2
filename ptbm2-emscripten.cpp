/**
 * Parentheses Tree Based Multicast header processor 2
 *
 * Written by Andras Majdan
 * License: GNU General Public License Version 3
 *
 * Report bugs to <majdan.andras@gmail.com>
 */

#ifdef EMSCRIPTEN

#include <stdio.h>
#include <sstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

#include "ptbm2.h"

#include <emscripten/bind.h>

using namespace std;

string ptbm_em_lines = "";

void ptbm_em_concatLines(string text)
{
  ptbm_em_lines.append(text + "\n");
}

void ptbm_em_readIntoVector(string text, vector<unsigned int> &nums)
{
  stringstream ss(text);

  for (int i; ss >> i;)
  {
    nums.push_back(i);
    if (ss.peek() == ',')
      ss.ignore();
  }
}

void ptbm_em_getBracketsNumsTTLs(
    string text,  string &brackets, vector<unsigned int> &nums,
	unsigned int &origTTL, unsigned int &currTTL)
{
  int brStart = text.find(' ');
  int ttlStart = text.find(' ', brStart+1);
  brackets = text.substr(0,brStart);
  vector<unsigned int> ttls;
  ptbm_em_readIntoVector(text.substr(brStart+1, ttlStart-brStart-1), nums);
  ptbm_em_readIntoVector(text.substr(ttlStart+1), ttls);
  origTTL = ttls[0];
  currTTL = ttls[1];
}

void ptbm_em_setHeaderFromText(ptbm2::Ptbm<> &pt, string text)
{
  string brackets;
  vector<unsigned int> nums;
  unsigned int origTTL;
  unsigned int currTTL;
  
  ptbm_em_getBracketsNumsTTLs(text, brackets, nums, origTTL, currTTL);
  pt.setHeader(brackets, nums, origTTL, currTTL);
}

void ptbm_em_setVirtualPorts(ptbm2::Ptbm<> &pt, string ports)
{
  vector<unsigned int> nums;
  ptbm_em_readIntoVector(ports, nums);
  pt.setVirtualPorts(nums);
}

string ptbm_em_processedHeader(ptbm2::Ptbm<> &pt)
{
  ptbm_em_lines = "";
  pt.procHeader(ptbm_em_concatLines);
  return ptbm_em_lines;
}


string ptbm_em_textFromBitset(string bitset) {
  ptbm2::Ptbm<> pt;
  pt.setHeaderBitset(bitset);
  return pt.getHeaderString();
}

string ptbm_em_bitsetFromText(string text)
{
 ptbm2::Ptbm<> pt;
 ptbm_em_setHeaderFromText(pt, text);
 return pt.getHeaderBits().to_string();
}

string ptbm_em_processBitset(string bitset, string virtualPorts)
{
  ptbm2::Ptbm<> pt;
  ptbm_em_setVirtualPorts(pt, virtualPorts);
  pt.setHeaderBitset(bitset);
  return ptbm_em_processedHeader(pt);
}

string ptbm_em_processText(string text, string virtualPorts)
{
  ptbm2::Ptbm<> pt;
  ptbm_em_setVirtualPorts(pt, virtualPorts);
  ptbm_em_setHeaderFromText(pt, text);
  return ptbm_em_processedHeader(pt);
}

EMSCRIPTEN_BINDINGS(ptbm_module) {
    emscripten::function("ptbm_em_textFromBitset", &ptbm_em_textFromBitset);
    emscripten::function("ptbm_em_bitsetFromText", &ptbm_em_bitsetFromText);
    emscripten::function("ptbm_em_processBitset", &ptbm_em_processBitset);
    emscripten::function("ptbm_em_processText", &ptbm_em_processText);
}

#endif
