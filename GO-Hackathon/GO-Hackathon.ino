#include "I2Cdev.h"
#include "MPU6050.h"

//Threshold for fall detection
#define svm_max  2147483645
#define THRES 25000



#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

MPU6050 accelgyro;


int16_t ax, ay, az;
int32_t ax32, ay32, az32;
int16_t gx, gy, gz;
int32_t svm;
int16_t ax32_filtered, ay32_filtered, az32_filtered;

#define OUTPUT_READABLE_ACCELGYRO

#define OUTPUT_ACCELGYRO_FALL

#define LED_PIN 13
bool blinkState = false;

void setup() {
  // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
#else if I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif

  // initialize serial communication
  // (38400 chosen because it works as well at 8MHz as it does at 16MHz, but
  // it's really up to you depending on your project)
  Serial.begin(115200);

  // initialize device
  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();
  accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  // configure Arduino LED for
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // read raw accel/gyro measurements from device
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  //ax32_filtered=Filter.run(ax);
  //ay32_filtered=Filter.run(ay);
  //az32_filtered=Filter.run(az);
  //ax32 = ax32_filtered; ay32 = ay32_filtered; az32 = az32_filtered;
  ax32 = ax; ay32 = ay; az32 = az;
  svm = ax32 * ax32 + ay32 * ay32 + az32 * az32;
  svm = sqrt(svm);

#ifdef OUTPUT_READABLE_ACCELGYRO
  // display tab-separated accel/gyro x/y/z values
  Serial.print(ax); Serial.print(",");
  Serial.print(ay); Serial.print(",");
  Serial.print(az); Serial.print(",");
  Serial.println(svm);  
#endif

#ifdef OUTPUT_ACCELGYRO_FALL
  // display tab-separated accel/gyro x/y/z values
  if (svm> svm_max)
    svm = svm_max;
  if (svm > THRES)
    Serial.println("FALL");
  /*else if ((svm < THRES)&&(svm>=ACC_4)&&(az>0))
    Serial.println("ACC 4");
  else if ((svm < ACC_4)&&(svm>=ACC_3)&&(az>0))
    Serial.println("ACC 3");
  else if ((svm < ACC_3)&&(svm>=ACC_2)&&(az>0))
    Serial.println("ACC 2");
  else if ((svm < ACC_2)&&(svm>=ACC_1)&&(az>0))
    Serial.println("ACC 1");
  else if ((svm < THRES)&&(svm>=ACC_4)&&(az<0))
    Serial.println("DEACC 4");
  else if ((svm < ACC_4)&&(svm>=ACC_3)&&(az<0))
    Serial.println("DEACC 3");
  else if ((svm < ACC_3)&&(svm>=ACC_2)&&(az<0))
    Serial.println("DEACC 2");
  else if ((svm < ACC_2)&&(svm>=ACC_1)&&(az<0))
    Serial.println("DEACC 1");*/
    
#endif


  // blink LED to indicate activity
  blinkState = !blinkState;
  digitalWrite(LED_PIN, blinkState);
  delay(100);
}


