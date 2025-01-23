# micro rosso odom helper

This a module for the [micro_rosso](https://github.com/xopxe/micro_rosso_platformio) system.

It's a helper module for writing mobile robots. It performs dead reconning, integrates the robot's movement, and publishes ROS `/odom` and `/tf` topics.

## Loading and starting

First, import the module into your project's `platformio.ini`:

```ini
lib_deps =
    ...
    https://github.com/xopxe/micro_rosso_odom_helper.git
```

Then, in your `main.cpp`:

```cpp
...
#include "odom_helper.h"
static OdomHelper odom;
...
void setup() {
  ...
  odom.setup();
  ...
}
```

The setup method allows optionally passing a topic name, the base and child frames for the tf, and a different micro_rosso timer to change the publication rate (by default, it uses the 5Hz timer). It is declared as follows:

```h
  static bool setup(const char *topic_odom = "/odom",
                    const char *frame_id = "odom",
                    const char *child_frame_id = "base_link",
                    timer_descriptor &timer = micro_rosso::timer_report);
```

## Dependencies

If building fails on missing tf2 headers, you must install them manually into your main project:

1. Clone [this](https://github.com/ros2/geometry2.git) repository and check out the branch of interest (e.g. jazzy).

1. Copy the `tf2_msgs` directory (make sure it has a package.xml inside) into an `extra_packages` directory in the root of your project.

1. Delete the .pio directory and rebuild everything.

## Using the module

You must call `update_pos(float vx, float vy, float vphi, float dt);` periodically, passing the linear and angular velocities and the time step. You typically will do this from the motor control code, perhaps inside a control loop associated with the micro_rosso::timer_control timer.

The resulting integrated position is stored in the static member `odom.pos`. The last instantaneous velocity is in `odom.vel`. Both are of the type `odom_t`:

```h
typedef struct
{
  float x;
  float y;
  float phi;
} odom_t;
```

You can set an absolute position at any moment using the `odom.set(x, y, phi)` method.

The module emits a [nav_msgs/msg/odometry](https://docs.ros2.org/foxy/api/nav_msgs/msg/Odometry.html) topic.

## TODO

* Refine QoS for the tf publisher.

## Authors and acknowledgment

<jvisca@fing.edu.uy> - [Grupo MINA](https://www.fing.edu.uy/inco/grupos/mina/), Facultad de Ingeniería - Udelar, 2024

## License

MIT
