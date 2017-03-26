  #include <Arduino_FreeRTOS.h>
  #include <semphr.h>
  
  #include "I2Cdev.h"
  #include "MPU6050.h"

  #define speakerPin 3 
  #define InfraredPin A7
  #define svm_max  2147483645
  #define THRES 20000

  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  #include "Wire.h"
  #endif

  MPU6050 accelgyro;

  SemaphoreHandle_t xSerialSemaphore;

  int helmet_condition;

  // define task that we want to use
  void TaskBlink( void *pvParameters );
  void TaskAcceleroRead( void *pvParameters );
  void TaskAlarmSpeed( void *pvParameters );
  void TaskReadInfrared( void *pvParameters );

  //void TaskReadAccelero( void *pvParameters );
  //void TaskReadSerial(void *pvParameters);


  // the setup function runs once when you press reset or power the board
  void setup() {

    
    // initialize serial communication at 9600 bits per second:
    Serial.begin(115200);
    
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.


    }


  // Semaphores are useful to stop a Task proceeding, where it should be paused to wait,
  // because it is sharing a resource, such as the Serial port.
  // Semaphores should only be used whilst the scheduler is running, but we can set it up here.
  if ( xSerialSemaphore == NULL )  // Check to confirm that the Serial Semaphore has not already been created.
  {
    xSerialSemaphore = xSemaphoreCreateMutex();  // Create a mutex semaphore we will use to manage the Serial Port
    if ( ( xSerialSemaphore ) != NULL )
      xSemaphoreGive( ( xSerialSemaphore ) );  // Make the Serial Port available for use, by "Giving" the Semaphore.
  }


    // Now set up two tasks to run independently.
    xTaskCreate(
      TaskReadInfrared
      , (const portCHAR *) "Infrared"
      , 128
      , NULL
      , 3
      , NULL);

    xTaskCreate(
      TaskAlarmSpeed
      , (const portCHAR *) "Alarm"
      , 128
      , NULL
      , 1
      , NULL);

    xTaskCreate(
      TaskAcceleroRead
      ,  (const portCHAR *) "AcceleroRead"
      ,  128  // Stack size
      ,  NULL
      ,  2  // Priority
      ,  NULL );

    // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
  }

  void loop()
  {
    // Empty. Things are done in Tasks.
  }

  /*--------------------------------------------------*/
  /*---------------------- Tasks ---------------------*/
  /*--------------------------------------------------*/

  void TaskReadInfrared(void *pvParameters)
  {
    (void) pvParameters;
    pinMode(InfraredPin, INPUT);
    int tcrt;

    for(;;)
    {
      tcrt = analogRead(InfraredPin);
      if(tcrt<700)
      {
        helmet_condition = 1;
        digitalWrite(13, HIGH);
      }
      else
      {
        helmet_condition = 0;
        digitalWrite(13, LOW);
      }
      if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
      {
      // We were able to obtain or "Take" the semaphore and can now access the shared resource.
      // We want to have the Serial Port for us alone, as it takes some time to print,
      // so we don't want it getting stolen during the middle of a conversion.
      // print out the state of the button:
        //Serial.println(" ============ TASK 4");
        Serial.print("2");Serial.print(",");
        Serial.println(helmet_condition);
        //Serial.println(tcrt);

        xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
      }
      vTaskDelay( 1000/ portTICK_PERIOD_MS);
    }
  }

  void TaskAlarmSpeed(void *pvParameters)
  {
    (void) pvParameters;
    int speed_flag = 1;

    for(;;)
    {
      //Serial.println(" ===== TASK 3");
      if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
      {
      // We were able to obtain or "Take" the semaphore and can now access the shared resource.
      // We want to have the Serial Port for us alone, as it takes some time to print,
      // so we don't want it getting stolen during the middle of a conversion.
      // print out the state of the button:
        //Serial.println(" ========== TASK 3");

        xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
      }
      while (speed_flag==1)
      {
        analogWrite(3, 120);   // turn the LED on (HIGH is the voltage level)
        vTaskDelay( 300 / portTICK_PERIOD_MS ); // wait for one second
        analogWrite(3, 0);    // turn the LED off by making the voltage LOW
        vTaskDelay( 300 / portTICK_PERIOD_MS ); // wait for one second
        analogWrite(3, 120);   // turn the LED on (HIGH is the voltage level)
        vTaskDelay( 300 / portTICK_PERIOD_MS ); // wait for one second
        analogWrite(3, 0);    // turn the LED off by making the voltage LOW
        vTaskDelay( 300 / portTICK_PERIOD_MS ); // wait for one second
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS );
    }
  }


  void TaskAcceleroRead(void *pvParameters)  // This is a task.
  {
    (void) pvParameters;
    
    int16_t ax, ay, az;
    int32_t ax32, ay32, az32;
    int16_t gx, gy, gz;
    int32_t svm;
    int accident_flag;


        // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
      Wire.begin();
    #else if I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
      Fastwire::setup(400, true);
    #endif

    
      // initialize device
    //Serial.println("Initializing I2C devices...");
    accelgyro.initialize();
    accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
    // verify connection
    //Serial.println("Testing device connections...");
    //Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
    for (;;)
    {
      accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
      ax32 = ax; ay32 = ay; az32 = az;
      svm = ax32 * ax32 + ay32 * ay32 + az32 * az32;
      svm = sqrt(svm);

      //#ifdef OUTPUT_ACCELGYRO_FALL
        // display tab-separated accel/gyro x/y/z values
        if (svm> svm_max)
          svm = svm_max;
        if (svm > THRES)
          {
            if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 30 ) == pdTRUE )
            {
              Serial.print("3");Serial.print(",");
              Serial.println("1");

              xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
            }
          }
          else
          {
            if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 30 ) == pdTRUE )
            {
              Serial.print("3");Serial.print(",");
              Serial.println("0");

              xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
            }
          }
        if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
        {
          // We were able to obtain or "Take" the semaphore and can now access the shared resource.

          Serial.print("1"); Serial.print(",");
          Serial.print(ax); Serial.print(",");
          Serial.print(ay); Serial.print(",");
          Serial.print(az); Serial.print(",");
          Serial.println(svm);  

          xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
        }
      
      vTaskDelay( 300 / portTICK_PERIOD_MS ); // wait for one second
    }
  }