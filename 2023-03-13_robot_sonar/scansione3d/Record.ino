//Read the values ot the potentiometers.
void readPot()
{
  potenziometro[0] = analogRead( POTENZIOMETRO_1 );
  potenziometro[1] = analogRead( POTENZIOMETRO_2 );
  potenziometro[2] = analogRead( POTENZIOMETRO_3 );
  potenziometro[3] = analogRead( POTENZIOMETRO_4 );

  xmm = map( potenziometro[0], 0, 1023, 0, 150);
  ymm = map( potenziometro[1], 0, 1023, -100, 100);
  zmm = map( potenziometro[2], 0, 1023, 0, 250);
  
}
