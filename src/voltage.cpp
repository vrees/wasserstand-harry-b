#include <driver/adc.h>
#include <math.h>
#include "esp32-lora-board-pins.h"
#include "voltage.h"
#include "sleep-wakeup.h"

#ifdef __cplusplus
extern "C"
{
#endif

  int number_round = 100;
  int adc_reading_42V = 0;
  int adc_reading_33V = 0;

  uint8_t payload[PAYLOAD_LENGTH];
  sensor_values_t sensor_values;

  /*   Polynom bestimmt aus den folgenden ADC- und Spannungs-Werten (3.3 Volt)
      3.665  4.2
      3.296  3.8
      2.916  3.4
      2.543  3.0
      2.072  2.5

      y = 5.814402272·10-2 x4 - 6.614275168·10-1 x3 + 2.785160725 x2 - 4.077128492 x + 3.802638788
 */
  polynom_coeffients_t module1_42Volt = {0.133858560, +1.99756137, -0.41139964, +0.176813638, -0.026239073};
  polynom_coeffients_t module2_33Volt = {-9.72728919, +12.975906, -5.48479196, +1.095972506, -0.0816047888};

  void initVoltage()
  {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_6); // VCC Voltage
  }

  int readRoundedAdc(adc1_channel_t channel)
  {
    printf("ADC: (Channel=%d)\n", channel);
    int adc;
    int adc_sum = 0;

    for (size_t i = 0; i < number_round; i++)
    {
      adc = adc1_get_raw(channel);

      adc_sum += adc;
      printf("%-4d\t", adc);
      if ((i % 20) == 19)
        printf("\n");
    }

    adc_sum = adc_sum / number_round;
    printf("Rounded ADC value=%-4d\n", adc_sum);

    return adc_sum;
  }

  double calulateVoltageCompensated(double adc, polynom_coeffients_t coeff)
  {
    /* 
    google search term: "polynominterpolation online"
    Calulate ploynom with 
    http://www.xuru.org/rt/PR.asp
    http://www.jaik.de/js/Interpolation.htm
    https://valdivia.staff.jade-hs.de/interpol.html
  */

    if (adc < 1 || adc > 4095)
      return 0;

    adc = adc / 1000;

    return coeff.c4 * pow(adc, 4) + coeff.c3 * pow(adc, 3) + coeff.c2 * pow(adc, 2) + coeff.c1 * adc + coeff.c0;
  }

  water_level_t getWaterLevel()
  {
    printf("Sensor unten: %i\n", gpio_get_level(WATER_LEVEL_SENSOR));

    if (gpio_get_level(WATER_LEVEL_SENSOR) == 0)
      return LOW_0_PERCENT;
    else
      return HIGH_100_PERCENT;
  }

  void decodeToPayload(water_level_t waterLevel, float vccVoltage, int16_t bootCount, int16_t execTooLongCount)
  {
    payload[0] = waterLevel;

    int16_t val = vccVoltage * 100;
    payload[1] = val >> 8;
    payload[2] = val;

    payload[3] = operation_mode;
    payload[4] = bootCount >> 8;
    payload[5] = bootCount;
    payload[6] = execTooLongCount >> 8;
    payload[7] = execTooLongCount;
  }

  void readSensorValues()
  {
    initVoltage();

    // read VCC Voltage (3.3 Volt)

    // internal Volatge
    adc_reading_33V = readRoundedAdc(ADC1_CHANNEL_6);
    sensor_values.vccVoltage = calulateVoltageCompensated(adc_reading_33V, module2_33Volt);
    // float vccVoltage = (float)adc_reading_33V * 2 * 2.2 / 4095; // wegen ADC_ATTEN_DB_6

    //external Volate
    adc_reading_42V = readRoundedAdc(ADC1_CHANNEL_7);
    sensor_values.vccVoltage = calulateVoltageCompensated(adc_reading_42V, module1_42Volt);

    printf("VCC-Voltage: %f Volt)\n", sensor_values.vccVoltage);

    sensor_values.waterLevel = getWaterLevel();
    printf("Water Level is %i Percent \n", sensor_values.waterLevel);

    decodeToPayload(sensor_values.waterLevel, sensor_values.vccVoltage, sensor_values.bootCount, sensor_values.execTooLongCount);
  }

#ifdef __cplusplus
}
#endif