#ifndef SLEW_C
#define SLEW_C

/*
  SLEW.C
  This file implements a slew rate control, so instead of changing motor voltage suddenly, it gradually (but not slowly) changes the voltage.

  USAGE

*/

#define MOTOR_NUM kNumbOfTotalMotors
#define MOTOR_PWM_0 10       //pwm value when rpm ready to take off from 0
#define MOTOR_PWM_1 90       //pwm value when rpm almost has peaked
int MOTOR_SLEW[10];          //Array to store each slew value
int motorReq[MOTOR_NUM];     //Array to hold requested speed for the motors
int BAILOUT = 0;

//----------------------------------------------------------------------------------
int pwmzone(int pwm){
  /*
    seperate the pwm-rpm curve into the following zones to linearing slew rate control
    returns
      0: dead zone 0           [-MOTOR_PWM_0,MOTOR_PWM_0]
      -1: dead zone 1 negative [-127, -MOTOR_PWM_1]
      1: deadzone 1 postive    [MOTOR_PWM_1,127]
      2: normal                [MOTOR_PWM_0 MOTOR_PWM_1]
      -2: normal               [-MOTOR_PWM_1 -MOTOR_PWM_0]
  */
  if (pwm <  -MOTOR_PWM_1) return -2;
  if (pwm <  -MOTOR_PWM_0) return -1;
  if (pwm <=  MOTOR_PWM_0) return 0;
  if (pwm <=  MOTOR_PWM_1) return 1;
  return 2;
}

task MotorsTask(){
  //Takes care of motors if there is no slew rate

  //Initialize and set all motors to 0V
  for (unsigned int i = 0; i < MOTOR_NUM; i++){
    motorReq[i] = 0;
  }

  while(true){
    for (unsigned int i = 0; i < MOTOR_NUM; i++){
      motor[i] = motorReq[i];
    }
    wait1Msec(50);
  }
}

task MotorSlewRateTask(){
  //Applies slew rate control to motors

  int req, now;

  //Initialize and set all motors to 0V
  for (unsigned int i = 0; i < MOTOR_NUM; i++){
    motorReq[i] = 0;
  }

  while(true){
    //Go through each motor
    for (unsigned int i = 0; i < MOTOR_NUM; i++){

      //Bailout
      if (vexRT[BAILOUT_BUTTON] == 1){
        motor[i] = 0;
  			motorReq[i] = 0;
        //Bailout Code
        continue;
      }
      else{
        BAILOUT = 0;
      }

      req = motorReq[i]; //What we want the voltage to be
      now = motor[i]; //What the voltage is

      if(req == now) continue; //If the voltage is what we want, continue to next motor.
      else if(req > now){ //Increase voltage
        //Set the now value to upper boundary of the zones
        if(pwmzone(now) == -2) now = -MOTOR_PWM_1;    // getting out of zone -1
        else if(pwmzone(now) == 0) now = MOTOR_PWM_0;     // getting out of zone 0
        else if(pwmzone(now) == 2) now = 127;           // in zone 1

        //Do slew rate addtion and set motor PWM
        motor[i] = MIN(req, now+MOTOR_SLEW[i]);
      }
      else if(req < now){ //Decrease voltage
        //Set the now value to lower boundary of the zones
        if(pwmzone(now) == -2) now = -127;
        else if(pwmzone(now) == 0) now = -MOTOR_PWM_0;
        else if(pwmzone(now) == 2) now = MOTOR_PWM_1;

        //Do slew rate addtion and set motor PWM
        motor[i] = MAX(req, now-MOTOR_SLEW[i]);
      }
    }

    wait1Msec(50);
  }
}

#endif
