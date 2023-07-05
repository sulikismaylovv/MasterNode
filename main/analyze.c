#include <stdio.h>
#include "sdkconfig.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

//gets input as ADC value and return command as 10 or 11
int LDR_command(int data) { return (data < 20) ? 11 : 10; }

//gets input as ADC and return command as 20 or 21
int SHUTTER_command(int data) { return (data < 20) ? 20 : ((data > 50) ? 21 : 0); }

//converts voltage from ntc sensor to temperature
float convert_to_temp(int voltage) {
    // Constants
    const int R_USED = 1000;
    const float V_USED = (float)voltage / 1000.0f;
    const float RES_REF = 2200.0f;
    const float A = 3.354016e-03f;
    const float B = 2.569850e-04f;
    const float C = 2.620131e-06f;
    const float D = 6.383091e-08f;
    
    // Calculation
    float res0 = (5.0f - V_USED) * R_USED / V_USED;
    float temp = A + B * log(res0 / RES_REF) + C * pow(log(res0 / RES_REF), 2) + D * pow(log(res0 / RES_REF), 3);
    float temperature = pow(temp, -1) - 274.15f;

    
    return temperature;
}

//get input as temp and returns number from 0-5
int STEPPER_command(float temperature) {
    // Constants
    const float TEMP_1 = 22.5f;
    const float TEMP_2 = 23.5f;
    const float TEMP_3 = 24.5f;
    const float TEMP_4 = 25.5f;
    const float TEMP_5 = 21.5f;
    
    // Calculation
    int result = 0;
    if (temperature < TEMP_5) {
        result = 5;
    }else if (temperature < TEMP_1) {
        result = 4;
    } else if (temperature < TEMP_2) {
        result = 3;
    } else if (temperature < TEMP_3) {
        result = 2;
    } else if (temperature < TEMP_4) {
        result = 1;
    } else {
        result = 0;
    }
    
    return result;
}

