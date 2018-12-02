/*Sensors Source*/

/*Includes------------------------------------------------------------*/
#include "sensors.h"
#include "SparkFun_LIS331.h"        //accelerometer
#include "SparkFun_MS5803_I2C.h"    //barometer
#include "SparkFunTMP102.h"         //temp sensor
#include "MPU9250.h"                //IMU
#include "Venus638FLPx.h"           //GPS

#include <Arduino.h>
#include <HardwareSerial.h>
#include <i2c_t3.h>
#include <SD.h>

/*Variables------------------------------------------------------------*/
File datalog;

LIS331 accelerometer;
MS5803 barometer(ADDRESS_HIGH);
TMP102 temp_sensor(TEMP_SENSOR_ADDRESS);
MPU9250 IMU(Wire, IMU_ADDRESS);
//TinyGPS gps;

/*Functions------------------------------------------------------------*/

/*initialize all the sensors*/
bool initSensors(void)
{
    bool status = true;

    /*init SD card*/
    #ifdef TESTING
    SerialUSB.println("Initializing SD card");
    #endif
    if (!SD.begin(BUILTIN_SDCARD)) {
        status = false;
        #ifdef TESTING
        SerialUSB.println("ERROR: SD card initialization failed!");
        #endif
    } else {
        datalog = SD.open("datalog.txt", FILE_WRITE);
        if (!datalog) {
            status = false;
            #ifdef TESTING
            SerialUSB.println("ERROR: Opening file failed!");
            #endif
        } else {
            datalog.write("SENSOR LOG DATA\n");
            datalog.write("Time (ms),Accelerometer - Acceleration X (g),Accelerometer - Acceleration Y (g),"
            "Accelerometer - Acceleration Z (g),Barometer - Pressure (mbar),Barometer - Temperature (C),"
            "Temperature Sensor - Temperature (C),IMU - Acceleration X (g),IMU - Acceleration Y (g),"
            "IMU - Acceleration Z (g),IMU - Angular Velocity X (rad/s),IMU - Angular Velocity Y (rad/s),"
            "IMU - Angular Velocity Z (rad/s),IMU - Magnetism X (uT),IMU - Magnetism Y (uT),"
            "IMU - Magnetism Z (uT),GPS - Latitude (DDM),GPS - Longitude (DDM),GPS - Altitude (m)\n");
        }
    }

    /*init accerlerometer*/
    #ifdef TESTING
    SerialUSB.println("Initializing accelerometer");
    #endif
    accelerometer.setI2CAddr(ACCELEROMETER_ADDRESS);
    accelerometer.begin(LIS331::USE_I2C);

    /*init barometer*/
    #ifdef TESTING
    SerialUSB.println("Initializing barometer");
    #endif
    barometer.reset();
    barometer.begin();

    /*init temp sensor*/
    #ifdef TESTING
    SerialUSB.println("Initializing temperature sensor");
    #endif
    temp_sensor.begin();

    /*init IMU*/
    #ifdef TESTING
    SerialUSB.println("Initializing IMU");
    #endif
    int error = 0;
    error = IMU.begin();
    if (error < 0) {
        status = false;
        #ifdef TESTING
        SerialUSB.print("ERROR: IMU initialization failed! Error code ");
        SerialUSB.println(error);
        #endif
    }

    /*init GPS*/
    #ifdef TESTING
    SerialUSB.println("Initializing GPS");
    #endif
    SerialGPS.begin(9600);
    while (!SerialGPS) {}   //TODO this can hang if baud rate is different change to delay?
    SerialGPS.write(GPS_reset_defaults, sizeof(GPS_reset_defaults));
    SerialGPS.write(GPS_set_baud_rate, sizeof(GPS_set_baud_rate));
    SerialGPS.end();
    SerialGPS.begin(115200);
    while (!SerialGPS) {}
    SerialGPS.write(GPS_set_NMEA_message, sizeof(GPS_set_NMEA_message));
    SerialGPS.write(GPS_set_update_rate, sizeof(GPS_set_update_rate));

    /*init radio*/
    #ifdef TESTING
    SerialUSB.println("Initializing radio");
    #endif
    SerialRadio.begin(921600);
    while (!SerialRadio) {}

    return status;
}

/*poll all the sensors*/
void pollSensors(unsigned long *timestamp, float acc_data[], float bar_data[],
                float *temp_sensor_data, float IMU_data[], char GPS_data[][GPS_FIELD_LENGTH])
{
    int16_t x, y, z;
    char sentence[SENTENCE_SIZE];

    *timestamp = millis();

    #ifdef TESTING
    SerialUSB.println("Polling accelerometer");
    #endif
    accelerometer.readAxes(x, y, z);
    acc_data[0] = accelerometer.convertToG(ACCELEROMETER_SCALE, x);
    acc_data[1] = accelerometer.convertToG(ACCELEROMETER_SCALE, y);
    acc_data[2] = accelerometer.convertToG(ACCELEROMETER_SCALE, z);

    #ifdef TESTING
    SerialUSB.println("Polling barometer");
    #endif
    bar_data[0] = barometer.getPressure(ADC_4096);
    bar_data[1] = barometer.getTemperature(CELSIUS, ADC_512);

    #ifdef TESTING
    SerialUSB.println("Polling temperature sensor");
    #endif
    *temp_sensor_data = temp_sensor.readTempC();

    #ifdef TESTING
    SerialUSB.println("Polling IMU");
    #endif
    IMU.readSensor();
    IMU_data[0] = IMU.getAccelX_mss() / EARTHS_GRAVITY; //convert to g
    IMU_data[1] = IMU.getAccelY_mss() / EARTHS_GRAVITY; //convert to g
    IMU_data[2] = IMU.getAccelZ_mss() / EARTHS_GRAVITY; //convert to g
    IMU_data[3] = IMU.getGyroX_rads();
    IMU_data[4] = IMU.getGyroY_rads();
    IMU_data[5] = IMU.getGyroZ_rads();
    IMU_data[6] = IMU.getMagX_uT();
    IMU_data[7] = IMU.getMagY_uT();
    IMU_data[8] = IMU.getMagZ_uT();

    #ifdef TESTING
    SerialUSB.println("Polling GPS");
    #endif
    if (getGPSData(sentence)) {
        getGPSField(sentence, GPS_data[0], 2); //latitude
        getGPSField(sentence, GPS_data[1], 4); //longitude
        getGPSField(sentence, GPS_data[2], 9); //altitude
    }
}

/*log all the data*/
void logData(unsigned long *timestamp, float acc_data[], float bar_data[],
            float *temp_sensor_data, float IMU_data[], char GPS_data[][GPS_FIELD_LENGTH])
{
    /*write data to SD card*/
    #ifdef TESTING
    SerialUSB.println("Writing to SD card");
    #endif
    datalog.print(*timestamp);
    datalog.print(",");
    for (unsigned int i = 0; i < ACC_DATA_ARRAY_SIZE; i++) {
       datalog.print(acc_data[i]);
       datalog.print(",");
    }
    for (unsigned int i = 0; i < BAR_DATA_ARRAY_SIZE; i++) {
        datalog.print(bar_data[i]);
        datalog.print(",");
    }
    datalog.print(*temp_sensor_data);
    datalog.print(",");
    for (unsigned int i = 0; i < IMU_DATA_ARRAY_SIZE; i++) {
        datalog.print(IMU_data[i]);
        datalog.print(",");
    }
    for (unsigned int i = 0; i < GPS_DATA_ARRAY_SIZE; i++) {
        datalog.print(GPS_data[i]);
        datalog.print(",");
    }
    datalog.print("\n");
    datalog.flush();

    /*output data to serial*/
    #ifdef TESTING
    SerialUSB.print("Time (ms):                          ");
    SerialUSB.println(*timestamp);
    SerialUSB.print("Accelerometer acceleration X (g):   ");
    SerialUSB.println(acc_data[0]);
    SerialUSB.print("Accelerometer acceleration Y (g):   ");
    SerialUSB.println(acc_data[1]);
    SerialUSB.print("Accelerometer acceleration Z (g):   ");
    SerialUSB.println(acc_data[2]);
    SerialUSB.print("Barometer pressure (mbar):          ");
    SerialUSB.println(bar_data[0]);
    SerialUSB.print("Barometer temperature (C):          ");
    SerialUSB.println(bar_data[1]);
    SerialUSB.print("Temperature sensor temperature (C): ");
    SerialUSB.println(*temp_sensor_data);
    SerialUSB.print("IMU acceleration X (g):             ");
    SerialUSB.println(IMU_data[0]);
    SerialUSB.print("IMU acceleration Y (g):             ");
    SerialUSB.println(IMU_data[1]);
    SerialUSB.print("IMU acceleration Z (g):             ");
    SerialUSB.println(IMU_data[2]);
    SerialUSB.print("IMU angular velocity X (rad/s):     ");
    SerialUSB.println(IMU_data[3]);
    SerialUSB.print("IMU angular velocity Y (rad/s):     ");
    SerialUSB.println(IMU_data[4]);
    SerialUSB.print("IMU angular velocity Z (rad/s):     ");
    SerialUSB.println(IMU_data[5]);
    SerialUSB.print("IMU magnetism X (uT):               ");
    SerialUSB.println(IMU_data[6]);
    SerialUSB.print("IMU magnetism X (uT):               ");
    SerialUSB.println(IMU_data[7]);
    SerialUSB.print("IMU magnetism X (uT):               ");
    SerialUSB.println(IMU_data[8]);
    SerialUSB.print("GPS latitude:                       ");
    SerialUSB.println(GPS_data[0]);
    SerialUSB.print("GPS longitude:                      ");
    SerialUSB.println(GPS_data[1]);
    SerialUSB.print("GPS altitude:                       ");
    SerialUSB.println(GPS_data[2]);
    SerialUSB.println("");
    #endif
}

void calculateValues(float acc_data[], float bar_data[], float* abs_accel,
                    float* prev_altitude, float* altitude, float* delta_altitude)
{
    // *abs_accel = sqrtf(powf(acc_data[0], 2) + powf(acc_data[1], 2) + powf(acc_data[2]), 2);
    // *prev_altitude = *altitude;
    // *altitude = 44330.0 * (1 - powf(bar_data[0] / BASELINE_PRESSURE, 1 / 5.255));
    // *delta_altitude = altitude - prev_altitude;
}



void processRadioData(unsigned long *timestamp, float acc_data[], float bar_data[],
    float *temp_sensor_data, float IMU_data[], char GPS_data[][GPS_FIELD_LENGTH] ){

   float time = *timestamp;
    sendRadioData(time, 't');

    sendRadioData(acc_data[0], UID_acc_acc_x);
    sendRadioData(acc_data[1], UID_acc_acc_y);
    sendRadioData(acc_data[2], UID_acc_acc_z);

    sendRadioData(bar_data[0], UID_bar_pres);
    sendRadioData(bar_data[1], UID_bar_temp);

    sendRadioData(IMU_data[0], UID_IMU_acc_x);
    sendRadioData(IMU_data[1], UID_IMU_acc_y);
    sendRadioData(IMU_data[2], UID_IMU_acc_z);
    sendRadioData(IMU_data[3], UID_IMU_gyro_x);
    sendRadioData(IMU_data[4], UID_IMU_gyro_y);
    sendRadioData(IMU_data[5], UID_IMU_gyro_z);
    sendRadioData(IMU_data[6], UID_IMU_mag_x);
    sendRadioData(IMU_data[7], UID_IMU_mag_y);
    sendRadioData(IMU_data[8], UID_IMU_mag_z);

    sendRadioData(ToFloat(GPS_data[0]), UID_GPS_lat);
    sendRadioData(ToFloat(GPS_data[1]), UID_GPS_long); 
    sendRadioData(ToFloat(GPS_data[2]), UID_GPS_alt);
    


    /*
    radio_data += String(UID_acc_acc_x) + floatToString(acc_data[0]);
    radio_data += String(UID_acc_acc_y) + floatToString(acc_data[1]);
    radio_data += String(UID_acc_acc_z) + floatToString(acc_data[2]);

    radio_data += String(UID_bar_pres) + floatToString(bar_data[0]);
    radio_data += String(UID_bar_temp) +floatToString(bar_data[1]);

    radio_data += String(UID_temp_temp) + floatToString(*temp_sensor_data);

    radio_data += String(UID_IMU_acc_x) + floatToString(IMU_data[0]);
    radio_data += String(UID_IMU_acc_y) + floatToString(IMU_data[1]);
    radio_data += String(UID_IMU_acc_z) + floatToString(IMU_data[2]);

    radio_data += String(UID_IMU_gyro_x) +floatToString(IMU_data[3]);
    radio_data += String(UID_IMU_gyro_y) + floatToString(IMU_data[4]);
    radio_data += String(UID_IMU_gyro_z) + floatToString(IMU_data[5]);

    radio_data += String(UID_IMU_mag_x) + floatToString(IMU_data[6]);
    radio_data += String(UID_IMU_mag_y) +floatToString(IMU_data[7]);
    radio_data += String(UID_IMU_mag_z) + floatToString(IMU_data[8]);
    */

                //  radio_data += String(UID_GPS_lat) + floatToString(GPS_data[0]);
                //  radio_data += String(UID_GPS_long) + floatToString(GPS_data[1]);
                //  radio_data += String(UID_GPS_alt) + floatToString(GPS_data[2]);



                   
   // const char *radio_char_data = radio_data.c_str();

//    radio_char_data[0] = 'A';
 //  radio_char_data[1] = '\0';

            
}

void sendRadioData(float data, char id){
    //teensy should be little endian, which means least significant is stored first, make sure ground station decodes accordingly 
     u_int8_t b[4];
      *(float*) b = data;
      SerialRadio.write(id);
      for(int i=0; i<4; i++)
      {
          SerialRadio.write(b[i]);
      }
}

