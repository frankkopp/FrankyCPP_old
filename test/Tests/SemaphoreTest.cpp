//
// Created by Frank Kopp on 2019-03-04.
//

#include <gtest/gtest.h>

#include <thread>
#include "../../src/Semaphore.h"

using testing::Eq;

void run();

Semaphore mySemaphore;
std::thread myThread;

enum State {
  NONE, NEW, INITIALIZED, DONE
};

State myState = NONE;

TEST(SemaphoreTest, basic) {

  // semaphore should not be available
  ASSERT_FALSE(mySemaphore.get());
  ASSERT_EQ(NONE, myState);

  std::cout << "Start Thread!\n";
  std::thread myThread = std::thread([] { run(); });
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  ASSERT_EQ(NEW, myState);

  std::cout << "Wait for Thread init!\n";
  mySemaphore.getOrWait();
  ASSERT_FALSE(mySemaphore.get());
  ASSERT_EQ(INITIALIZED, myState);

  std::cout << "Thread Started\n";

  if (myThread.get_id() == std::this_thread::get_id()) std::cout << "start: NEW THREAD\n";
  else std::cout << "start: OLD THREAD\n";

  myThread.join();
  ASSERT_EQ(DONE, myState);

  std::cout << "Thread Ended\n";
}

void run() {
  myState = NEW;
  std::cout << "New Thread: Started!\n";
  std::this_thread::sleep_for(std::chrono::seconds(2));
  myState = INITIALIZED;
  std::cout << "New Thread: Init done!\n";
  mySemaphore.release();
  if (myThread.get_id() == std::this_thread::get_id()) std::cout << "run: NEW THREAD\n";
  else std::cout << "run: OLD THREAD\n";
  std::this_thread::sleep_for(std::chrono::seconds(2));
  myState = DONE;
  std::cout << "New Thread: Finished!\n";
}