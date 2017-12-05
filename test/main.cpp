#include <unistd.h>
#include "src/network/NetWrapper.h"

int main() {
  network::NetWrapper netwarpper;
  netwarpper.Launch();
  sleep(1);
  return 0;
}
