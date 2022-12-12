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
  /* 
   * SENZA CONSIDERARE I 10 MM DI DISALLINEAMENTO
   * 
  xPmm = xO2mm + 110*cos(alfaRad - gammaRad)*cos(betaRad);
  yPmm = yO2mm + 110*cos(alfaRad - gammaRad)*sin(betaRad);
  zPmm = zO2mm + 110*sin(alfaRad - gammaRad);
   *
   */
  /* 
   * CONSIDERANDO I 10 MM DI DISALLINEAMENTO IN Z
   */ 
  xPmm = xO2mm + (110*cos(alfaRad - gammaRad) - 10*sin(alfaRad - gammaRad))*cos(betaRad);
  yPmm = yO2mm + (110*cos(alfaRad - gammaRad) - 10*sin(alfaRad - gammaRad))*sin(betaRad);
  zPmm = zO2mm + (110*sin(alfaRad - gammaRad) + 10*cos(alfaRad - gammaRad));
   
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

float calcoloErrore ( float deltaX, float deltaY, float deltaZ ) {
  
  return ((deltaX*deltaX) + (deltaY*deltaY) + (deltaZ*deltaZ));
  
}


void tentativoMovimento(int alfa1024_tmp, int beta1024_tmp, int gamma1024_tmp, 
                        float *errore1,
                        float x2mm, float y2mm, float z2mm,
                        int *alfa1024_ok, int *beta1024_ok, int *gamma1024_ok) {
                        
  float xmm_tmp, ymm_tmp, zmm_tmp, errore2;
 
  calcoloCinematicoDiretto(alfa1024_tmp, beta1024_tmp, gamma1024_tmp, &xmm_tmp, &ymm_tmp, &zmm_tmp);
  errore2 = calcoloErrore ( (x2mm - xmm_tmp), (y2mm - ymm_tmp), (z2mm - zmm_tmp));

  if (errore2 < *errore1) { 
    *alfa1024_ok = alfa1024_tmp;
    *beta1024_ok = beta1024_tmp;
    *gamma1024_ok = gamma1024_tmp;

    *errore1 = errore2;
  }
  
}

void writeServo() {

  Servo_1.write(map(alfa1024, 0, 1023, 0, 180)       + OFFSET_ALFA); 
  Servo_0.write(map(beta1024, 0, 1023, 0, 180)       + OFFSET_BETA); 
  Servo_2.write(map(gamma1024, 0, 1023, 0, 180)      + OFFSET_GAMMA); 
  Servo_3.write(map(potenziometro[3], 0, 1023, 100, 180)); 

}
  
