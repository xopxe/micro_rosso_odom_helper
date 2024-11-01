# micro rosso odom helper

This a module for the [micro_rosso](https://github.com/xopxe/micro_rosso_platformio) system.

It's a helper module for writing mobile robots. It performs dead reconning, integrates the movement of the robot, and publishes ROS /odom topics.

## Loading and starting

In your main.cpp 

```
...
#include "odom_helper.h"
static OdomHelper odom;
...
void setup() {
  ...
  odm.setup() {
  ...
}
```

The setup method allows the passing of an optional topic name and a different timer to change the publication rate (by default, it uses the 5Hz timer). It is declared as follows:

```
  static bool setup(const char *topic_odom = "/odom", timer_descriptor &timer_report = micro_rosso::timer_report);
```

## Using

You must call `update_pos(float vx, float vy, float vphi, float dt);` periodically, passing the linear and angular velocities and the time step. You typically will do this from the motor control code, perhaps inside a control loop associated to the micro_rosso::timer_control timer.

The integrated position is stored in the static member `static odom_t pos`. The last instantaneous velocity is in `static odom_t vel`.

At any moment you can set an absolute position 

....

## Authors and acknowledgment

jvisca@fing.edu.uy - [Grupo MINA](https://www.fing.edu.uy/inco/grupos/mina/), Facultad de Ingeniería - Udelar, 2024

## License

MIT


