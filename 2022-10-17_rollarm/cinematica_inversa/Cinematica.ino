void calcoloCinematicoDiretto(int alfa1024, int beta1024, int gamma1024, float *xmm, float *ymm, float *zmm) {

  float alfaDeg, betaDeg, gammaDeg;
  alfaDeg  = map(alfa1024, 0, 1023,  18000,    0) / 100.0;
  betaDeg  = map(beta1024, 0, 1023,  -9000, 9000) / 100.0;
  gammaDeg = map(gamma1024, 0, 1023, 18000,    0) / 100.0;

  /* trasformo da gradi a radianti */
  float alfaRad, betaRad, gammaRad;
  alfaRad  = alfaDeg * 3.141592/180.00; 
  betaRad  = betaDeg * 3.141592/180.00; 
  gammaRad = gammaDeg * 3.141592/180.00; 
  
  float xOmm, yOmm, zOmm;
  xOmm = 0;
  yOmm = 0;
  zOmm = 0;

  float xO1mm, yO1mm, zO1mm;
  xO1mm = xOmm + 0;
  yO1mm = yOmm + 0;
  zO1mm = zOmm + 64;

  float xO2mm, yO2mm, zO2mm;
  xO2mm = xO1mm + 60*cos(alfaRad)*cos(betaRad);
  yO2mm = yO1mm + 60*cos(alfaRad)*sin(betaRad);
  zO2mm = zO1mm + 60*sin(alfaRad);
  
  float xPmm, yPmm, zPmm;
  xPmm = xO2mm + 110*cos(alfaRad - gammaRad)*cos(betaRad);
  yPmm = yO2mm + 110*cos(alfaRad - gammaRad)*sin(betaRad);
  zPmm = zO2mm + 110*sin(alfaRad - gammaRad);

  *xmm = xPmm;
  *ymm = yPmm;
  *zmm = zPmm;

  /*  
  Serial.print(xPmm);
  Serial.print(",");
  Serial.print(yPmm);
  Serial.print(",");
  Serial.println(zPmm);
  */
}

void writeServo() {

  Servo_1.write(map(alfa1024, 0, 1023, 0, 180)); 
  Servo_0.write(map(beta1024, 0, 1023, 0, 180)); 
  Servo_2.write(map(gamma1024, 0, 1023, 0, 180)); 
  Servo_3.write(map(potenziometro[3], 0, 1023, 100, 180)); 

}
  
