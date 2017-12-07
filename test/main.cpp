/*
 * main.cpp
 *
 *  Created on: 2017年12月7日
 *      Author: xueda
 */

#include <unistd.h>
#include "src/network/NetWrapper.h"

int main() {
  network::NetWrapper netwarpper;
  netwarpper.Launch();
//  sleep(15);
  return 0;
}
