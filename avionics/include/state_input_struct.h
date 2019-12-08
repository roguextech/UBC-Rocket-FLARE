#ifndef STATE_INPUT_STRUCT_H
#define STATE_INPUT_STRUCT_H

#include "Eigen/Core"

struct StateInput {
    bool ignitor_good = false;
    float altitude;
    Eigen::Vector3f accel_rocket;
    Eigen::Vector3f accel_ground;
    /*
    Orientation quaternion represents the rotation of the rocket, from the ground-based coordinate system. The quaternion is expected to be normalized by the time any states deal with it. */
    Eigen::Quaternionf orientation;
};

struct StateAuxilliaryInfo {
    bool in_abort = false;
}

#endif
