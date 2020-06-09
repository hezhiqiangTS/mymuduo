#include <assert.h>
#include <stdint.h>
#include <sys/timerfd.h>
#include <time.h>

int main(int argc, char** argv) {
  struct itimerspec ts;
  struct timespec start, now;
  int maxExp, fd, secs, nanosecs;

  uint64_t numExp, totalExp;
  int s;

  fd = timerfd_create(CLOCK_REALTIME, 0);
  assert(timerfd_settime(fd, 0, &ts, NULL) != -1);
  assert(clock_gettime(CLOCK_MONOTONIC, &start) != -1);
  for (totalExp = 0; totalExp < maxExp;) {
    s = read(fd, &numExp, sizeof(numExp));
    assert(s == sizeof(numExp));

    totalExp += numExp;
    assert(clock_gettime(CLOCK_MONOTONIC, &now) != -1);

    secs = now.tv_sec - start.tv_sec;
  }
}