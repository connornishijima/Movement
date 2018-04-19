#include "motion.h"

// Attach 6" wire to A0
// Open Arduino Serial Plotter @ 250000 baud
// Top graph is motion, bottom graph is proximity!

void setup() {
  // put your setup code here, to run once:
  Serial.begin(250000);
  motion_start(A0);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("-300\t300\t0\t");
  Serial.print(motion()+150);
  Serial.print("\t");
  Serial.println(proximity()-150);
  delay(5);
}