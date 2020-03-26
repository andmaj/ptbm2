/**
 * Parentheses Tree Based Multicast header processor 2
 *
 * Written by Andras Majdan
 * License: GNU General Public License Version 3
 *
 * Report bugs to <majdan.andras@gmail.com>
 */

#include <stdio.h>
#include <string>
#include <stdexcept>

#include <cxxopts.hpp>
#include "ptbm2.h"

using namespace std;

void setVirtualPorts(ptbm2::Ptbm<> &p, cxxopts::ParseResult opts)
{
  if(opts.count("virtual"))
    p.setVirtualPorts(opts["virtual"].as<vector<unsigned int>>());
}

void printProcLine(string line)
{
  cout << line << endl;
}

int main(int argc, char **argv)
{
  cxxopts::Options options("PTBM2", "Parentheses Tree Based Multicast 2");

  options.add_options()
    ("b,brackets", "Brackets in the header",
      cxxopts::value<string>()->default_value(""))
    ("n,numbers", "Numbers in the header",
      cxxopts::value<vector<unsigned int>>())
    ("v,virtual", "Virtual ports",
      cxxopts::value<vector<unsigned int>>())
    ("o,origttl", "Original TTL",
      cxxopts::value<unsigned int>()->default_value("64"))
    ("c,currttl", "Current TTL",
      cxxopts::value<unsigned int>()->default_value("64"))
    ("g,generate", "Generate header (binary)",
      cxxopts::value<bool>()->default_value("false"))
    ("i,input", "Input header (binary)",
      cxxopts::value<bool>()->default_value("false"))
    ("p,print", "Print header",
      cxxopts::value<bool>()->default_value("false"))
    ("help", "Print usage")
    ;

  auto opts = options.parse(argc, argv);

  if (opts.count("help"))
  {
    std::cout << options.help() << std::endl;
     exit(0);
  }

  ptbm2::Ptbm<> pt;

  if(opts["input"].as<bool>())
  {
    if(opts.count("brackets") ||
       opts.count("numbers") ||
       opts.count("generate"))
    {
      throw cxxopts::OptionException(
              "input option cannot be used with: brackets, numbers, generate");
    }

    string bits;
    cin >> bits;
    pt.setHeaderBitset(bits);

    if(opts.count("print"))
      cout << pt.getHeaderString();
    else
    {
      setVirtualPorts(pt, opts);
      pt.procHeader(printProcLine);
    }

    exit(0);
  }

  string brackets = opts["brackets"].as<string>();
  vector<unsigned int> nums = {};

  if(opts["numbers"].count())
    nums = opts["numbers"].as<vector<unsigned int>>();

  unsigned int origTTL = opts["origttl"].as<unsigned int>();
  unsigned int currTTL = opts["currttl"].as<unsigned int>();

  if(opts["generate"].as<bool>())
  {
    if(opts.count("input") || opts.count("print"))
      throw cxxopts::OptionException(
          "generate option cannot be used with: input, print");

    pt.setHeader(brackets, nums, origTTL, currTTL);
    cout << pt.getHeaderBits();
    exit(0);
  }

  pt.setHeader(brackets, nums, origTTL, currTTL);
  setVirtualPorts(pt, opts);
  pt.procHeader(printProcLine);

  return 0;
}
