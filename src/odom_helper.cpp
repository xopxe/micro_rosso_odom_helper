#include "odom_helper.h"
odom_t OdomHelper::pos;
odom_t OdomHelper::vel;

static nav_msgs__msg__Odometry msg_odom;
static publisher_descriptor pdescriptor_odom;

#include <geometry_msgs/msg/transform_stamped.h>
#include <tf2_msgs/msg/tf_message.h>
static geometry_msgs__msg__TransformStamped msg_transform;
static tf2_msgs__msg__TFMessage msg_tf;
static publisher_descriptor pdescriptor_tf;
rmw_qos_profile_t qos_profile_tf = rmw_qos_profile_default;

#include <micro_ros_utilities/type_utilities.h>
#include <micro_ros_utilities/string_utilities.h>

#define RCNOCHECK(fn)       \
  {                         \
    rcl_ret_t temp_rc = fn; \
    (void)temp_rc;          \
  }

static void euler_to_quat(float roll, float pitch, float yaw, float *q)
{
  float cy = cos(yaw * 0.5);
  float sy = sin(yaw * 0.5);
  float cp = cos(pitch * 0.5);
  float sp = sin(pitch * 0.5);
  float cr = cos(roll * 0.5);
  float sr = sin(roll * 0.5);

  q[0] = cy * cp * cr + sy * sp * sr;
  q[1] = cy * cp * sr - sy * sp * cr;
  q[2] = sy * cp * sr + cy * sp * cr;
  q[3] = sy * cp * cr - cy * sp * sr;
}

OdomHelper::OdomHelper()
{
  // we don't change these anymore
  msg_odom.pose.covariance[0] = 0.0001;
  msg_odom.pose.covariance[7] = 0.0001;
  msg_odom.pose.covariance[35] = 0.0001;

  msg_odom.twist.covariance[0] = 0.0001;
  msg_odom.twist.covariance[7] = 0.0001;
  msg_odom.twist.covariance[35] = 0.0001;

  // qos_profile_tf.reliability = RMW_QOS_POLICY_RELIABILITY_VOLATILE;

  msg_tf.transforms.size = msg_tf.transforms.capacity = 1;
  msg_tf.transforms.data = &msg_transform;
}

void OdomHelper::set(float x, float y, float phi)
{
  OdomHelper::pos.x = x;
  OdomHelper::pos.y = y;
  OdomHelper::pos.phi = phi;
}

void OdomHelper::reset()
{
  set(0.0, 0.0, 0.0);
}

void OdomHelper::update_pos(float vx, float vy, float vphi, float dt)
{
  OdomHelper::vel.x = vx;
  OdomHelper::vel.y = vy;
  OdomHelper::vel.phi = vphi;

  OdomHelper::pos.phi += vphi * dt;
  float phi_y = OdomHelper::pos.phi + PI / 2;

  float dx = vx * dt;
  float dy = vy * dt;

  OdomHelper::pos.x += cos(OdomHelper::pos.phi) * dx;
  OdomHelper::pos.y += sin(OdomHelper::pos.phi) * dx;
  OdomHelper::pos.x += cos(phi_y) * dy;
  OdomHelper::pos.y += sin(phi_y) * dy;
}

static void report_cb(int64_t last_call_time)
{
  int64_t now = rmw_uros_epoch_millis(); // TODO replace with real timestamp

  // robot's position in x,y,phi
  msg_odom.pose.pose.position.x = OdomHelper::pos.x;
  msg_odom.pose.pose.position.y = OdomHelper::pos.y;

  // calculate robot's heading in quaternion angle
  // ROS has a function to calculate yaw in quaternion angle
  float q[4];
  euler_to_quat(0, 0, OdomHelper::pos.phi, q);
  // robot's heading in quaternion
  msg_odom.pose.pose.orientation.x = q[1];
  msg_odom.pose.pose.orientation.y = q[2];
  msg_odom.pose.pose.orientation.z = q[3];
  msg_odom.pose.pose.orientation.w = q[0];

  // speed from encoders
  msg_odom.twist.twist.linear.x = OdomHelper::vel.x;
  msg_odom.twist.twist.linear.y = OdomHelper::vel.y;
  msg_odom.twist.twist.angular.z = OdomHelper::vel.phi;

  micro_rosso::set_timestamp_ms(msg_odom.header.stamp, now);
  RCNOCHECK(rcl_publish(&pdescriptor_odom.publisher, &msg_odom, NULL));

  msg_transform.transform.translation.x = msg_odom.pose.pose.position.x;
  msg_transform.transform.translation.y = msg_odom.pose.pose.position.y;
  msg_transform.transform.translation.z = msg_odom.pose.pose.position.z;
  msg_transform.transform.rotation = msg_odom.pose.pose.orientation;

  micro_rosso::set_timestamp_ms(msg_transform.header.stamp, now);
  RCNOCHECK(rcl_publish(&pdescriptor_tf.publisher, &msg_tf, NULL));

  /*
  D_print(OdomHelper::vel.x);
  D_print(" O ");
  D_println(OdomHelper::vel.phi);
  */
}

bool OdomHelper::setup(const char *topic_odom,
                       const char *frame_id,
                       const char *child_frame_id,
                       timer_descriptor &timer)
{
  D_print("setup: odom_helper... ");

  msg_odom.header.frame_id = micro_ros_string_utilities_set(msg_odom.header.frame_id, frame_id);
  msg_odom.child_frame_id = micro_ros_string_utilities_set(msg_odom.child_frame_id, child_frame_id);

  msg_transform.header.frame_id = micro_ros_string_utilities_set(msg_transform.header.frame_id, frame_id);
  msg_transform.child_frame_id = micro_ros_string_utilities_set(msg_transform.child_frame_id, child_frame_id);

  pdescriptor_odom.qos = QOS_DEFAULT;
  pdescriptor_odom.type_support = (rosidl_message_type_support_t *)
      ROSIDL_GET_MSG_TYPE_SUPPORT(nav_msgs, msg, Odometry);
  pdescriptor_odom.topic_name = topic_odom;
  micro_rosso::publishers.push_back(&pdescriptor_odom);

  pdescriptor_tf.qos = QOS_CUSTOM;
  pdescriptor_tf.qos_profile = &qos_profile_tf;
  pdescriptor_tf.type_support =
      (rosidl_message_type_support_t *)ROSIDL_GET_MSG_TYPE_SUPPORT(
          tf2_msgs, msg, TFMessage);
  pdescriptor_tf.topic_name = "/tf";
  micro_rosso::publishers.push_back(&pdescriptor_tf);

  timer.callbacks.push_back(&report_cb);

  D_println("done.");
  return true;
}
