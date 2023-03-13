void readSonar()
{
  unsigned long t;

  /* produco impulso */
  digitalWrite( SONAR_TRIG, HIGH );
  delayMicroseconds( 10 );
  digitalWrite( SONAR_TRIG, LOW );
  
  /* leggo il segnale di ritorno */
  t = pulseIn( SONAR_ECHO, HIGH, 30000 ); /* oltre i 450 cm ritorna 0 */

  /* calcolo distanza */
  dist_sonar_mm = t / 6;  /* approssimata ed espressa in millimetri */
}
