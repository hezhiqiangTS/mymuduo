在 muduo0.2 基础上添加TimerQueue定时器

## Linux timerfd API
Linux-specific timerfd API 可以用来创建定时器，该定时器的超时通知可以通过文件描述符去获取。
### 创建 timerfd
```c
#include <sys/timerfd.h>

int timerfd_create(int clockid, int flags);
```
`timerfd_create`创建一个新的定时器对象，返回指向该对象的fd。
clockid参数可选：
* CLOCK_REALTIME 
* CLOCK_MONOTONIC
flags参数：
* TFD_CLOEXEC 
* TFD_NONBLOCK
当timerfd_create创建的定时器不再被使用时，需要用close调用告诉操作系统可以回收相关资源。
### 设置时间
```c
#include <sys/timerfd>

int timerfd_settime(int fd, int flags, const struct itimerspec *new_value, 
                    struct itimerspec *old_value);

struct itimerspec{
    struct timespec it_interval;    /* Interval for periodic timer*/
    struct timespec it_value;   /* Current value (time until next expiration) */
};

struct timespec{
    __time_t tv_sec;		/* Seconds.  */
    __syscall_slong_t tv_nsec;	/* Nanoseconds.  */
};
```
`timerfd_settime`用来为timerfd设置时间。flags标志:
* 0 表示 `new_value.it_value` 与调用 timerfd_settime() 的时刻有关。
* `TFD_TIMER_ABSTIME` 表示 `new_value.it_value` 是一个绝对时间（从时钟的 zero point 开始记起）

`itiemrspec.it_value` 用于设置定时器下一次 expiration 发生的时间。
`itimerspec.it_interval` 用于设置定时器是否为周期性定时器，如果其值为 0，则表示定时器只会 expiration 一次

### 获取时间

```c
#include <sys/timerfd>

int timerfd_gettime(int fd, struct itimerspec *curr_value);
```
`timerfd_gettime`用于获取 timerfd 的 interval 与 remaining time。curr_value 指向的 itimerspec 结构体用来保存查询结果。

### Reading from timerfd

当我们已经通过`timerfd_settime()`设置过定时器之后，就可以使用 read 调用来获取定时器的超时次数信息。

如果自从上次`timerfd_settime()`或者`read()`之后定时器有过一次或者多次 expiration，那么read就会立刻返回，并且buffer中将会保存 the number of expiration.

如果没有 expiration 发生，那么 read 将会阻塞，或者设置 error EAGAIN（前提是使用 fcntl() 将 timerfd 设置为 O_NONBLOCK）

## TimerQueue
## Timer
## TimerID
## test4.cpp
