void polarToRect(int alfa1024, int beta1024, int gamma1024, float *x, float *y, float *z) {

  /* sulla base della geometria di Robot RollArm, determino i gradi di ogni servo */
  float alfaDeg, betaDeg, gammaDeg;
  alfaDeg = map(alfa1024, 0, 1023, 18000, 0) / 100.0;
  betaDeg = map(beta1024, 0, 1023, -9000, 9000) / 100.0;
  gammaDeg = map(gamma1024, 0, 1023, 18000, 0) / 100.0;

  /* trasformo da gradi a radianti */
  float alfaRad, betaRad, gammaRad;
  alfaRad = alfaDeg * PI / 180.00;
  betaRad = betaDeg * PI / 180.00;
  gammaRad = gammaDeg * PI / 180.00;

  /* origine del Robot */
  float xO, yO, zO;
  xO = 0;
  yO = 0;
  zO = 0;

  /* orogine del primo giunto */
  float xO1, yO1, zO1;
  xO1 = xO + 0;
  yO1 = yO + 0;
  zO1 = zO + 64;

  /* origine del secondo giunto */
  float xO2, yO2, zO2;
  xO2 = xO1 + 60 * cos(alfaRad) * cos(betaRad);
  yO2 = yO1 + 60 * cos(alfaRad) * sin(betaRad);
  zO2 = zO1 + 60 * sin(alfaRad);

  /* estremità del Robot RollArm */
  float xP, yP, zP;
  xP = xO2 + 110 * cos(alfaRad - gammaRad) * cos(betaRad);
  yP = yO2 + 110 * cos(alfaRad - gammaRad) * sin(betaRad);
  zP = zO2 + 110 * sin(alfaRad - gammaRad);

  *x = xP; *y = yP; *z = zP;
}
