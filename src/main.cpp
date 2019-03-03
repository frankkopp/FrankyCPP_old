

#include "UCIProtocolHandler.h"

int main() {

  auto uci = new UCIProtocolHandler();

  uci->start();
  
  delete(uci);

  return 0;
}