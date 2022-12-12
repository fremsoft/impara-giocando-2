//Read the values ot the potentiometers.
void readPot()
{
  potenziometro[0] = analogRead( POTENZIOMETRO_1 );
  potenziometro[1] = analogRead( POTENZIOMETRO_2 );
  potenziometro[2] = analogRead( POTENZIOMETRO_3 );
  potenziometro[3] = analogRead( POTENZIOMETRO_4 );
}
