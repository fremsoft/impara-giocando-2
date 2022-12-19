//Calculate the time the button  pressed
void readButton()
{
  if (digitalRead(PULSANTE) == 0)
  {
    delay(10);
    if (digitalRead(PULSANTE) == 0)
    {
      tempoPressionePulsanteDS = 0;
      while (!digitalRead(PULSANTE))
      {
        tempoPressionePulsanteDS ++;
        delay(100);
      }
    }
  }  
}
