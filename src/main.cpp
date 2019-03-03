#include <iostream>
#include <chrono>
#include <iomanip>

#include "Bitboards.h"

using namespace std;

struct space_out : std::numpunct<char> {
  char do_thousands_sep()   const { return '.';  } // separate with spaces
  std::string do_grouping() const { return "\03"; } // groups of 1 digit
};

int main() {

  std::locale loc (std::cout.getloc(), new space_out);
  std::cout.imbue(loc);

  Bitboards::init();

  std::cout << Bitboards::print(EMPTY_BB) << std::endl;
  std::cout << Bitboards::printFlat(EMPTY_BB) << std::endl;
  std::cout << Bitboards::print(ALL_BB) << std::endl;
  std::cout << Bitboards::printFlat(ALL_BB) << std::endl;

//  for (int i = 0; i < SQ_NONE; ++i) {
//    std::cout << Bitboards::print(SquareBB[i]) << std::endl;
//  }

  std::cout << Bitboards::print(SquareBB[SQ_A1]) << std::endl;
  std::cout << Bitboards::printFlat(SquareBB[SQ_A1]) << std::endl;
  std::cout << Bitboards::print(SquareBB[SQ_H1]) << std::endl;
  std::cout << Bitboards::printFlat(SquareBB[SQ_H1]) << std::endl;
  std::cout << Bitboards::print(SquareBB[SQ_A8]) << std::endl;
  std::cout << Bitboards::printFlat(SquareBB[SQ_A8]) << std::endl;
  std::cout << Bitboards::print(SquareBB[SQ_H8]) << std::endl;
  std::cout << Bitboards::printFlat(SquareBB[SQ_H8]) << std::endl;

  std::cout << Bitboards::print(SquareBB[SQ_H8]) << std::endl;
  std::cout << Bitboards::printFlat(SquareBB[SQ_H8]) << std::endl;

  std::cout << Bitboards::print(ALL_BB) << std::endl;

  return 0;
}