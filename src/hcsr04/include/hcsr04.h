#ifndef _HCSR04_H
#define _HCSR04_H

#include <stdint.h>

/**
 * Represents a unit to measure proximity by.
 */
typedef enum measure {
    MM,  ///< milimeters
    CM,  ///< centimeters
    IN,  ///< inches
} hcsr04_measure_t;

/**
 * Initiates GPIO pins for the sensor.
 *
 * @param trig GPIO pin assigned to the sensor's trigger pin.
 * @param echo GPIO pin assigned to the sensor's echo pin.
 */
void hcsr04_init(uint8_t trig, uint8_t echo);

/**
 * Gets a distance measurement from the sensor and returns it in a given unit
 * of measurement.
 *
 * @param m Unit of measurement in which to return proximity value.
 *
 * @returns Proximity measurement in the specified unit.
 */
double hcsr04_get_distance(hcsr04_measure_t m);

#endif  // _HCSR04_H
