

#include "UCIProtocolHandler.h"

int main() {

  auto uci = new UCIProtocolHandler();

  uci->loop();
  
  return 0;
}