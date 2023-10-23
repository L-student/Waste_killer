// GRUPO: 
// Daniel Pessoa Máximo
// Lívia de Maria Calado Machado Soares
// Matheus Vieira Faria 
// Rangel Gonçalves
// Ronaldo Cândido dos Santos Sobrinho

// Período 2023.1

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <Servo.h>

 

#define in1 5
#define in2 6
#define in3 11
#define in4 9

#define TOL 1.5

Servo myservo;

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire,-1);
Adafruit_MPU6050 mpu;
double roll = 0;
double pitch = 0;
double yaw = 0;
double temperature = 0;

void setup() {
  Serial.begin(9600);
  
  //configuração dos motores
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  myservo.attach(3);

  if (!mpu.begin()) {
    Serial.println("Não foi possível encontrar um sensor MPU6050.");
    while (1);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
 // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; 
  }
  display.display();
  display.clearDisplay();
  delay(5);
  display.setTextColor(WHITE);
  display.setRotation(0);
}

void loop() {


  getAngles(&roll, &pitch, &yaw);
  getTemperature(&temperature);
  setDisplay(roll, temperature, yaw);
  ajustRote(yaw);
  autoLevel(roll);
  /*
  Serial.print("Roll: ");
  Serial.print(roll);
  Serial.print(" Pitch: ");
  Serial.print(pitch);
  Serial.print(" Yaw: ");
  Serial.println(yaw);
*/
  delay(200);
}

void getAngles(double *roll,double *pitch,double *yaw){
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float accelRoll = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / PI;
  float accelPitch = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180.0 / PI;

  *roll = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / PI;
  *pitch = atan(-a.acceleration.x / sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180.0 / PI;
  *yaw += (g.gyro.z / 131.0) *1000;  // Taxa de amostragem do sensor // vei é vezes 1000  e pronto
  *yaw += -0.012; // testato empiricamente, como um bom engenheiro, comfia!
  if (yaw > 180) yaw -= 360;
  if (yaw < -180) yaw += 360;
}

void setDisplay(double x, double y, double z){
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("X: ");
  display.println(x);
  
  display.setTextSize(1);
  display.setCursor(0,10);
  display.print("T: ");
  display.println(y);
  
  display.setTextSize(1);
  display.setCursor(0,20);
  display.print("Z: ");
  display.println(z);
  display.display();
}

void ajustRote(double currentAngle) { // melhor logica que eu encontrei

  int powerA, powerB;

  if (currentAngle > TOL) {
    // Aumente a potência do motor A, inversamente proporcional a currentAngle
    powerA = map(abs(currentAngle), TOL, 90, 100, 50);
  } else if (currentAngle < -TOL) {
    // Aumente a potência do motor B, inversamente proporcional a currentAngle
    powerB = map(abs(currentAngle), TOL, 90, 100, 50);
  } else {
    // Ângulo dentro da tolerância, ambos os motores a 100%
    powerA = 100;
    powerB = 100;
  }


  // Converta powerA e powerB de porcentagem para valores PWM (0-255)
  powerA = map(powerA, 50, 100, 50, 255);
  powerB = map(powerB, 50, 100, 50, 255);
  
  //Serial.print("PowerA: ");
  //Serial.print(powerA);
  //Serial.print(" - PowerB: ");
  //Serial.println(powerB);
  analogWrite(in1, powerA);
  analogWrite(in2,LOW);
  analogWrite(in3, powerB);
  analogWrite(in4,LOW);
}


void autoLevel(double x){
  double ang = map(x, -90, 90, 180, 0);
  if(ang>5 && ang <175){
    if(ang > myservo.read()+TOL || ang < myservo.read()-TOL){
      myservo.write(ang); // meu servo esta com problema 
      Serial.println(ang);
    }
  }
}

void getTemperature(double *temperature) {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  *temperature = temp.temperature;
}
