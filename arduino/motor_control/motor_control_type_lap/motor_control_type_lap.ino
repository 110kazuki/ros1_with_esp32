/* [ DC motor control with Locked anti phase PWM type motor driver ]
 *  version : 1.0 (20220422)
 *  Auther  : Kazuki Ito
 */

#undef   ESP32
#include <ros.h> //ESP32 USB serial mode
#define  ESP32
#include <std_msgs/Int16.h>

/* ==== User setting ==== */
//motor driver property
#define PWM_FREQUENCY  3000     //PWM pulse frequency
#define PWM_RESOLUTION 10       //Duty ratio resolution, 10bit (0~1023)
#define PWM_duty_limit_under 10 //[%] Lower limit of PWM duty ratio
#define PWM_duty_limit_upper 90 //[%] Upper limit of PWM duty ratio

//ESP32 pin number setting
#define PWM_PIN      17         //pin number connect with PWM input channel
#define MOTOR_EN_PIN 18         //pin number connect with enable channel (option)
/* ======================== */

#define MOTOR_0 0               //PWM channel
double motor_cmd = 0;           // motor command speed, specified as an integer value from -100 to 100.
int    pwm_cmd   = 0;         
int    PWM_duty_range = (pow( 2, PWM_RESOLUTION )-1) * (PWM_duty_limit_under + (100-PWM_duty_limit_upper)) * 0.005;

//ros setting
ros::NodeHandle nh;

void messageCb( const std_msgs::Int16& ctrl_msg ){
  motor_cmd = ctrl_msg.data/100.0;
}

//subscriber
  ros::Subscriber<std_msgs::Int16> sub( "motor_ctrl", &messageCb );

void setup() {
  //ros mode
  nh.getHardware()->setBaud( 57600 );
  nh.initNode();
  nh.subscribe( sub );
  //nh.advertise(pub);

  //pin setting
  pinMode( MOTOR_EN_PIN, OUTPUT );

  //pwm setting
  ledcSetup( MOTOR_0, PWM_FREQUENCY, PWM_RESOLUTION );
  ledcAttachPin( PWM_PIN, MOTOR_0 );
  
  //desable ESCON (safety)
  digitalWrite( MOTOR_EN_PIN, LOW );

  PWM_duty_limit_under = pow( 2, PWM_RESOLUTION ) * PWM_duty_limit_under * 0.01;
  PWM_duty_limit_upper = pow( 2, PWM_RESOLUTION ) * PWM_duty_limit_upper * 0.01;
}

void loop() {
  if ( nh.connected() ){
    //convert motor command to duty ratio
    pwm_cmd = constrain( motor_cmd * PWM_duty_range + 511, PWM_duty_limit_under, PWM_duty_limit_upper );

    digitalWrite( MOTOR_EN_PIN, HIGH );
    ledcWrite( MOTOR_0, pwm_cmd );    

  }else{
    pwm_cmd = 511;
    digitalWrite( MOTOR_EN_PIN, LOW );
    ledcWrite( MOTOR_0, 511 );
  }

  nh.spinOnce();
  delay(5);

}
