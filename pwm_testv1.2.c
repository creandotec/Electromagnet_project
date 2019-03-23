#include <bcm2835.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define PIN           RPI_GPIO_P1_12
#define PWM_CHANNEL   0
#define RANGE         10

#define BUTTON_1      RPI_V2_GPIO_P1_11
#define BUTTON_2      RPI_V2_GPIO_P1_15
#define BUTTON_3      RPI_V2_GPIO_P1_16
#define BUTTON_4      RPI_V2_GPIO_P1_13

#define OUTPUT_A      RPI_V2_GPIO_P1_38
#define OUTPUT_B      RPI_V2_GPIO_P1_40


#define DEBOUNCE_TIME   10000

int run_pwm = 1;

void INThandler(int);

int calculatePeriods(int frequency, uint8_t direction);
float calculateRealFrequency(int high_period);

int main(int argc, char **argv){
  if(!bcm2835_init())
    return 1;
  bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_ALT5);


  bcm2835_pwm_set_clock(BCM2835_PWM_CLOCK_DIVIDER_2);
  bcm2835_pwm_set_mode(PWM_CHANNEL, 1, 1);
  bcm2835_pwm_set_range(PWM_CHANNEL, RANGE);

  // Set RPI pin P1-15 to be an input
  bcm2835_gpio_fsel(BUTTON_1, BCM2835_GPIO_FSEL_INPT);
  //  with a pullup
  bcm2835_gpio_set_pud(BUTTON_1, BCM2835_GPIO_PUD_UP);
  // Set RPI pin P1-15 to be an input
  bcm2835_gpio_fsel(BUTTON_2, BCM2835_GPIO_FSEL_INPT);
  //  with a pullup
  bcm2835_gpio_set_pud(BUTTON_2, BCM2835_GPIO_PUD_UP);
  // Set RPI pin P1-15 to be an input
  bcm2835_gpio_fsel(BUTTON_3, BCM2835_GPIO_FSEL_INPT);
  //  with a pullup
  bcm2835_gpio_set_pud(BUTTON_3, BCM2835_GPIO_PUD_UP);
  // Set RPI pin P1-15 to be an input
  bcm2835_gpio_fsel(BUTTON_4, BCM2835_GPIO_FSEL_INPT);
  //  with a pullup
  bcm2835_gpio_set_pud(BUTTON_4, BCM2835_GPIO_PUD_UP);

  bcm2835_gpio_fsel(OUTPUT_A, BCM2835_GPIO_FSEL_OUTP);

  bcm2835_gpio_fsel(OUTPUT_B, BCM2835_GPIO_FSEL_OUTP);

  int direction = 1;
  int data = 5;

  //This is the frequency for the cyclic on/off of the electromagnet
  int set_frequency = 1;
  int set_high_period = 500000;

  float real_frequency = 1.0;

  //float f_period = 1/frequency;
  //int period = (int)(f_period * 1000000);
  //int high_period = period/2;

  int level_of_signal = 1;
  int period_counter = 0;
  int update_gui_counter = 0;

  uint8_t button_1_value = bcm2835_gpio_lev(BUTTON_1);
  uint8_t button_2_value = bcm2835_gpio_lev(BUTTON_2);
  uint8_t button_3_value = bcm2835_gpio_lev(BUTTON_3);
  uint8_t button_4_value = bcm2835_gpio_lev(BUTTON_4);

  uint8_t p_button_1_value;
  uint8_t p_button_2_value;
  uint8_t p_button_3_value;
  uint8_t p_button_4_value;

  int b1_fast_increment = 10000;
  int b2_fast_decrement = 10000;

  int b1_fast_increment_factor = 2;
  int b2_fast_decrement_factor = 2;

  int b1_debounce_counter = 0;
  int b2_debounce_counter = 0;
  int b3_debounce_counter = 0;
  int b4_debounce_counter = 0;

  uint8_t b1_on = 0;
  uint8_t b2_on = 0;
  uint8_t b3_on = 0;
  uint8_t b4_on = 0;

  bcm2835_pwm_set_data(PWM_CHANNEL, data);

  bcm2835_gpio_write(OUTPUT_A, HIGH);
  bcm2835_gpio_write(OUTPUT_B, LOW);

  printf("frequency = %uHz        pwm = %u%%\n", set_frequency, data*10);
  (void) signal(SIGINT, INThandler);
  while(1){

    /*
    if(data == 1)
      direction = 1;
    else if(data == RANGE-1)
      direction = -1;
    data += direction;
    */
    //bcm2835_pwm_set_data(PWM_CHANNEL, data);
    //bcm2835_delay(1000);

    p_button_1_value = button_1_value;
    p_button_2_value = button_2_value;
    p_button_3_value = button_3_value;
    p_button_4_value = button_4_value;

    bcm2835_delayMicroseconds(1);

    period_counter += 1;

    update_gui_counter += 10;
    if(period_counter >= set_high_period){
      //printf("%d\n", period_counter);
      //printf("%d\n", high_period);
      period_counter = 0;
      if(level_of_signal == 1){
        bcm2835_pwm_set_data(PWM_CHANNEL, 0);
        level_of_signal = 0;
        //printf("LOW LEVEL\n");
      }
      else{
        bcm2835_pwm_set_data(PWM_CHANNEL, data);
        level_of_signal = 1;
        //printf("HIGH LEVEL\n");
      }
    }

    /*
    *Update the buttons state
    */
    button_1_value = bcm2835_gpio_lev(BUTTON_1);
    button_2_value = bcm2835_gpio_lev(BUTTON_2);
    button_3_value = bcm2835_gpio_lev(BUTTON_3);
    button_4_value = bcm2835_gpio_lev(BUTTON_4);

    if(b1_on == 1 && b4_on == 1){

      if(b1_debounce_counter >= DEBOUNCE_TIME * 50 && b4_debounce_counter >= DEBOUNCE_TIME * 50){
        printf("Thanks for using the Electromagnet controller system\nBye\n");
        system("sudo shutdown -h now");
      }
    }

    //Check if button 1 is pressed
    if(button_1_value == 0){
      //printf("Pressed\n");
      if(p_button_1_value == 0){
        b1_debounce_counter += 1;
        if(b1_debounce_counter >= DEBOUNCE_TIME){
          if(b1_on == 0){
            b1_on = 1;
            if(set_frequency < 250000){
              set_frequency += 1;
              set_high_period = calculatePeriods(set_frequency, 1);
              real_frequency = calculateRealFrequency(set_high_period);
              set_frequency = (int)(real_frequency + 0.5);
              printf("frequency = %uHz        pwm = %u%%\n", set_frequency, data*10);
            }
          }else{
            if(b1_debounce_counter >= DEBOUNCE_TIME * 50){
              if(b1_debounce_counter >= (DEBOUNCE_TIME * 50) + 50000){
                b1_debounce_counter = DEBOUNCE_TIME * 50;

                if(b1_fast_increment > 1000)
                {
                  b1_fast_increment -= 1000;
                }else{

                  b1_fast_increment = 5000;
                  b1_fast_increment_factor *= 2;

                  if(b1_fast_increment_factor > 100){
                    b1_fast_increment_factor = 100;
                  }
                  set_frequency += b1_fast_increment_factor;

                  if(set_frequency > 249000){
                    set_frequency = 249000;
                  }
                }

                if(set_frequency < 250000){
                  int t_set_frequency = set_frequency;
                  int secure_factor = 1;
                  while (set_frequency <= t_set_frequency)
                  {
                    set_frequency += secure_factor;
                    printf("Trying with increase");
                    set_high_period = calculatePeriods(set_frequency, 1);
                    real_frequency = calculateRealFrequency(set_high_period);
                    //printf("%f\n", real_frequency);
                    set_frequency = (int)(real_frequency + 0.5);
                    secure_factor += 1;
                  }

                  printf("frequency = %uHz        pwm = %u%%\n", set_frequency, data*10);
                }
                else{
                  set_frequency = 250000;
                }
              }
            }
          }
        }
      }else{
        b1_debounce_counter = 0;
      }
    }else{
      b1_fast_increment = 10000;
      b1_fast_increment_factor = 1;
      b1_debounce_counter = 0;
      b1_on = 0;
    }

    //Check if button 1 is pressed
    if(button_2_value == 0){
      if(p_button_2_value == 0){
        b2_debounce_counter += 1;
        if(b2_debounce_counter >= DEBOUNCE_TIME){
          if(b2_on == 0){
            b2_on = 1;
            if(set_frequency > 1){
              set_frequency -= 1;
              set_high_period = calculatePeriods(set_frequency, 0);
              real_frequency = calculateRealFrequency(set_high_period);
              set_frequency = (int)(real_frequency);
              printf("frequency = %uHz        pwm = %u%%\n", set_frequency, data*10);
            }
          }
          else{
            if(b2_debounce_counter >= DEBOUNCE_TIME * 50){
              if(b2_debounce_counter >= (DEBOUNCE_TIME * 50) + 50000){
                b2_debounce_counter = DEBOUNCE_TIME * 50;

                if(b2_fast_decrement > 1000)
                {
                  b2_fast_decrement -= 1000;
                }else{

                  b2_fast_decrement = 5000;
                  b2_fast_decrement_factor *= 2;

                  if(b2_fast_decrement_factor > 100){
                    b2_fast_decrement_factor = 100;
                  }
                  set_frequency -= b2_fast_decrement_factor;
                  if(set_frequency < 2){
                    set_frequency = 2;
                  }
                }

                if(set_frequency > 2)
                {
                  int t_set_frequency = set_frequency;
                  int secure_factor = 1;
                  while (set_frequency >= t_set_frequency)
                  {
                    printf("Trying with decrease");
                    set_frequency -= secure_factor;
                    set_high_period = calculatePeriods(set_frequency, 0);
                    real_frequency = calculateRealFrequency(set_high_period);
                    set_frequency = (int)(real_frequency);
                    secure_factor -= 1;
                  }
                  printf("frequency = %uHz        pwm = %u%%\n", set_frequency, data*10);
                }
              }
            }
          }
        }
      }else{
        b2_debounce_counter = 0;
      }
    }else{

      b2_fast_decrement = 10000;
      b2_fast_decrement_factor = 1;

      b2_debounce_counter = 0;
      b2_on = 0;

    }

    //Check if button 1 is pressed
    if(button_3_value == 0){
      //printf("Pressed\n");
      if(p_button_3_value == 0){
        b3_debounce_counter += 1;
        if(b3_debounce_counter == DEBOUNCE_TIME){
          if(b3_on == 0){
            b3_on = 1;

            if(data < 10 )
              data += 1;
            printf("frequency = %uHz        pwm = %u%%\n", set_frequency, data*10);
          }
        }
      }else{
        b3_debounce_counter = 0;
      }
    }else{
      b3_debounce_counter = 0;
      b3_on = 0;
    }

    //Check if button 1 is pressed
    if(button_4_value == 0){
      if(p_button_4_value == 0){
        b4_debounce_counter += 1;
        if(b4_debounce_counter == DEBOUNCE_TIME){
          if(b4_on == 0){
            b4_on = 1;
            if(data > 0){
              data -= 1;
            }
            printf("frequency = %uHz        pwm = %u%%\n", set_frequency, data*10);
          }
        }
      }else{
        b4_debounce_counter = 0;
      }
    }else{
      b4_debounce_counter = 0;
      b4_on = 0;
    }

  }
  printf("Bye, thanks for use this pwm test");
  bcm2835_close();
  return 0;
}

/*
*Calculate the required period to achive the desired frequency
*/
int calculatePeriods(int frequency, uint8_t direction){
  float real_period = 1.0/(float)frequency;
  int high_period;
  real_period = real_period/2.0;
  real_period = real_period  * 1000000.0;
  if(direction == 1){
    high_period = (int)(real_period+0.5);
  }
  else{
    high_period = (int)(real_period-0.5);
  }

  return high_period;
}

float calculateRealFrequency(int high_period){
  float period = high_period / 1000000.0;
  period = period * 2.0;
  float real_frequency = 1.0/period;
  return real_frequency;
}
void INThandler(int sig){
  char c;
  //SIGNAL(SIGINT, INThandler);

  signal(sig, SIG_IGN);
  printf("Do you really want to quit? [y/n]");
  c = getchar();
  if( c=='y'|| c == 'Y'){
    run_pwm = 0;
    bcm2835_pwm_set_mode(PWM_CHANNEL, 1, 0);
    bcm2835_pwm_set_data(PWM_CHANNEL, 0);
    bcm2835_close();
    printf("Bye");
    exit(0);
  }else
    signal(SIGINT, INThandler);
  getchar();
}
