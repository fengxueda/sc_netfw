/*
 * main.cpp
 *
 *  Created on: 2017年12月7日
 *      Author: xueda
 */

#include <thread>
#include "src/network/NetWrapper.h"

int main() {
  network::NetWrapper netwarpper;
  netwarpper.Launch();
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  return 0;
}
