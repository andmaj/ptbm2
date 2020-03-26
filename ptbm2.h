/**
 * Parentheses Tree Based Multicast header processor 2
 *
 * Written by Andras Majdan
 * License: GNU General Public License Version 3
 *
 * Report bugs to <majdan.andras@gmail.com>
 */

#ifndef PTBM_H
#define PTBM_H

#include <bitset>
#include <string>
#include <vector>
#include <stdexcept>

using namespace std;

namespace ptbm2
{

// HEADER_SIZE  The size of the header in bits
// PORT_SIZE    The size of the port numbers in bits
// MAX_OPEN_BRACKETS  The maximal number of open brackets in the header
// PORTS_START_AT     At which bit does the list of ports start
// ORIG_TTL_STARTS_AT From which bit do we store the original TTL
// CURR_TTL_STARTS_AT From which bit do we store the current TTL
// TTL_SIZE           The size of TTL in bits
template
  <int HEADER_SIZE=256,
   int PORT_SIZE=4,
   int MAX_OPEN_BRACKETS=40,
   int PORTS_START_AT=80,
   int ORIG_TTL_STARTS_AT=240,
   int CURR_TTL_STARTS_AT=248,
   int TTL_SIZE=8>
class Ptbm
{

public:

Ptbm()
{
  pbs = bitset<HEADER_SIZE>(0);
}

/// Sets the header from bits (in string form)
void
setHeaderBitset(string bits)
{
  pbs = bitset<HEADER_SIZE>(bits);
}

/// Sets the header from brackets, port numbers and TTLs
void
setHeader(
    string brackets,
    vector<unsigned int> nums,
    unsigned int origTTL,
    unsigned int currTTL)
{
  pbs = generateHeader(brackets, nums, origTTL, currTTL);
}

/// Gets the header in string form (of brackets, port numbers and TTLs)
string
getHeaderString()
{
  return headerToString(pbs);
}

/// Returns the header in bitset form
bitset<HEADER_SIZE>
getHeaderBits()
{
  return pbs;
}

/// Sets the virtual ports list
void
setVirtualPorts(vector<unsigned int> vports)
{
  pvports = vports;
}

/// Process the header as a router and call procFunc for each port output
void
procHeader(void (*procFunc)(string))
{
  vector<portttl> ports;

  processHeader(pbs, pvports, ports);

  for(portttl const& val: ports) {
    procFunc(to_string(val.port) + " " + // Output port
             to_string(val.ttl));        // Current TTL to send
  };
}

private:
bitset<HEADER_SIZE> pbs;
vector<unsigned int> pvports = {};

struct portttl
{
  unsigned int port;
  unsigned int ttl;
};

struct compare
{
        int key;
        compare(int const &i): key(i) { }

        bool operator()(int const &i)
        {
                return (i == key);
        }
};

/// Reads a number from a position in a bitset
template<unsigned FIELD_SIZE>
unsigned int
readInt(bitset<HEADER_SIZE> &bs, size_t pos)
{
  unsigned int res=0, bm = 0;
  bool bit;

  for(size_t i=0; i<FIELD_SIZE; i++)
  {
    bit = bs[pos+i];
    res += bit ? 1<<bm : 0;
    ++bm;
  }
  return res;
}

/// Sets bits of a number from a position in a bitset
template<unsigned FIELD_SIZE>
void
setBits(bitset<HEADER_SIZE> &bs, size_t pos, unsigned int num)
{
  if(num > (1<<FIELD_SIZE)-1 )
    throw runtime_error(
        "Number out of range: "
        + to_string(num) + " cannot fit in "
        + to_string(FIELD_SIZE) + " bits");

  for(size_t i=0; i<FIELD_SIZE; i++)
  {
    bs[pos+i] = num%2;
    num /= 2;
  }
}

// Gets bitset from the textual form (brackets and ports numbers) of the header
bitset<HEADER_SIZE>
generateHeader(
    string br,
    vector<unsigned int> nums,
    unsigned int origTTL,
    unsigned int currTTL)
{
  bitset<HEADER_SIZE> bs(0); // just for safety, default: initialize to zero
  int totalOpenBrackets = 0;

  for(string::size_type i = 0; i < br.size(); i++)
  {
    if(br[i]=='(')
    {
      bs[i] = true;
      ++totalOpenBrackets;
    }
    else if (br[i]!=')')
      throw runtime_error("Invalid bracket: " + to_string(br[i]));
  }

  if(totalOpenBrackets>MAX_OPEN_BRACKETS)
    throw runtime_error("Too many open brackets, header limit overflow");

  if(totalOpenBrackets!=(int)nums.size())
    throw runtime_error("Number of open brackets != Number of numbers");

  for(int pos=PORTS_START_AT, i=0; i<totalOpenBrackets; i++, pos+=PORT_SIZE)
    setBits<PORT_SIZE>(bs, pos, nums[i]);

  setBits<TTL_SIZE>(bs, ORIG_TTL_STARTS_AT, origTTL);
  setBits<TTL_SIZE>(bs, CURR_TTL_STARTS_AT, currTTL);

  return bs;
}

void
processVirtualSubtree(
    bitset<HEADER_SIZE> bs,
    unsigned int &currBracketPos,
    unsigned int &currNodeNum,
    unsigned int virtualPort,
    vector<portttl> &ports,
    unsigned int origTTL)
{
  unsigned int openBracketsNum = 0, totalOpenBrackets = 0;
  unsigned int portPlusPart, realPort, savedTotalOpenBrackets;
  unsigned int savedNodeNum;
  bool foundChild = false;

  while(currBracketPos<PORTS_START_AT)
  {
      if(bs[currBracketPos])
      {
        // (
        ++currNodeNum;
        ++openBracketsNum;
        ++totalOpenBrackets;
        if(openBracketsNum==1)
        {
          savedNodeNum = currNodeNum;
          savedTotalOpenBrackets = totalOpenBrackets;
        }
      }
      else
      {
        // )
        if(openBracketsNum==0)
        {
           if(!foundChild)
             throw runtime_error("Virtual port (" +
                                 to_string(virtualPort) +
                                 ") has no child at " +
                                 to_string(currBracketPos));
           else
           {
             ++currNodeNum;
             return;
           }
        }
        else if(openBracketsNum==1)
        {
           portPlusPart =
               readInt<PORT_SIZE>(bs, PORTS_START_AT+(savedNodeNum*PORT_SIZE));
           realPort = (virtualPort+1)*(1<<PORT_SIZE) + portPlusPart;

          if(totalOpenBrackets==savedTotalOpenBrackets)
            ports.push_back({realPort,0});
          else
            ports.push_back({realPort,origTTL-(savedNodeNum+1)});

          foundChild = true;
        }
        --openBracketsNum;
      }
      ++currBracketPos;
  }

  throw runtime_error("Virtual subtree oveflow");

}

void
processRealSubtree(
    bitset<HEADER_SIZE> bs,
    unsigned int &currBracketPos,
    unsigned int &currNodeNum,
    vector<portttl> &ports,
    unsigned int origTTL)
{
  unsigned int currPort;
  unsigned int openBracketsNum = 0, totalOpenBrackets = 0;

  // Found real subtree
  currPort = readInt<PORT_SIZE>(bs, PORTS_START_AT+(currNodeNum*PORT_SIZE));
  unsigned int savedNodeNum = currNodeNum;

  while(currBracketPos<PORTS_START_AT)
  {
      if(bs[currBracketPos])
      {
        // (
        ++currNodeNum;
        ++openBracketsNum;
        ++totalOpenBrackets;
      }
      else
      {
        // )
        if(openBracketsNum<=1)
        {
          if(totalOpenBrackets>1)
            ports.push_back({currPort,origTTL-(savedNodeNum+1)});
          else
            ports.push_back({currPort,0});
          return;
        }
        else
          --openBracketsNum;
      }
      ++currBracketPos;
  }

  throw runtime_error("Real subtree oveflow");
}

/// Process the trees from a specified node (myself)
void
processTreesFromHere(
  bitset<HEADER_SIZE> bs,
  vector<unsigned int> virtualPorts,
  unsigned int &myBracketPos,
  unsigned int &myNodeNum,
  vector<portttl> &ports,
  unsigned int origTTL)
{
  unsigned int myPortNum =
      readInt<PORT_SIZE>(bs, PORTS_START_AT + PORT_SIZE * myNodeNum);

  if(any_of(virtualPorts.begin(), virtualPorts.end(), compare(myPortNum)))
  {
    // my port is virtual
    processVirtualSubtree(bs, ++myBracketPos, myNodeNum, myPortNum, ports, origTTL);
    ;
  }
  else
  {
    // my port is real
    processRealSubtree(bs, myBracketPos, myNodeNum, ports, origTTL);
  }

  return;
};



/// Process the header
void
processHeader(
  bitset<HEADER_SIZE> bs,
  vector<unsigned int> virtualPorts,
  vector<portttl> &ports)
{
  unsigned int origTTL = readInt<TTL_SIZE>(bs, ORIG_TTL_STARTS_AT);
  unsigned int currTTL = readInt<TTL_SIZE>(bs, CURR_TTL_STARTS_AT);

  if(currTTL > origTTL)
    throw runtime_error("currTTL > origTTL");

  unsigned int myNodeNum = origTTL - currTTL;
  unsigned int bracketPos=0, openBracketsNum=0, currOpenBrackets=0;

  bool processTreesFromNow = false;
  unsigned int minOpenBrackets=0;

  while(bracketPos<PORTS_START_AT)
  {
    if(bs[bracketPos])
    {
      // (
      if(myNodeNum==openBracketsNum) {
        processTreesFromHere(bs, virtualPorts, bracketPos,
                             myNodeNum, ports, origTTL);
        processTreesFromNow = true;
        minOpenBrackets = currOpenBrackets;
      }
      else if(processTreesFromNow)
      {
        processTreesFromHere(bs, virtualPorts, bracketPos,
                             myNodeNum, ports, origTTL);
      }
      else
      {
        ++openBracketsNum;
        ++currOpenBrackets;
      }
    }
    else
    {
      // )
      if(currOpenBrackets<=minOpenBrackets)
        return;
      else
        --currOpenBrackets;
    }
    ++bracketPos;
  }

  if(!ports.size())
    throw runtime_error("I can't find my node (" +
                        to_string(myNodeNum) +
                        ") in the tree");
}

/// Convert a header bitset to its textual form (brackets, ports and TTLs)
string
headerToString(bitset<HEADER_SIZE> bs)
{
  string s = "";
  int currOpenBrackets = 0;
  int totalOpenBrackets = 0;
  bool checkOnlyOpenBrackets = false;

  int pos = 0;

  for(;pos<PORTS_START_AT; pos++)
  {
    if(checkOnlyOpenBrackets && bs[pos])
      throw runtime_error("Open bracket at a wrong place: " + to_string(pos+1));

    if(bs[pos])
    {
      ++currOpenBrackets;
      ++totalOpenBrackets;
      s.push_back('(');
    }
    else
    {
      if(currOpenBrackets <=0)  // just for safety, can't be < 0
      {
        // There must be no more open brackets
        checkOnlyOpenBrackets = true;
        continue;
      }
      --currOpenBrackets;
      s.push_back(')');
    }
  }

  if(currOpenBrackets)
    throw runtime_error("Not all brackets have been closed (after all)");

  pos = PORTS_START_AT; // just for safety, pos has been set by the for cycle
  bool firstReadInt = true;

  while(totalOpenBrackets-- > 0) // just for safety, can't be < 0
  {
    if(firstReadInt)
    {
     s.push_back(' ');
     firstReadInt = false;
    }
    else
     s.push_back(',');

    s.append(to_string(readInt<PORT_SIZE>(bs, pos)));
    pos += PORT_SIZE;
  }

  s.append(" " + to_string(readInt<TTL_SIZE>(bs, ORIG_TTL_STARTS_AT)));
  s.append("," + to_string(readInt<TTL_SIZE>(bs, CURR_TTL_STARTS_AT)));

  return s;
}

};

}

#endif // PTBM_H
