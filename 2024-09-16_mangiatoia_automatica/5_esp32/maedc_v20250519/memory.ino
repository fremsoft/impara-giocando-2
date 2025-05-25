#include "esp_heap_caps.h"

void printFreeRAM() {
  Serial.print("Heap disponibile: ");
  Serial.print(esp_get_free_heap_size());
  Serial.println(" byte");

  Serial.print("Largest block disponibile: ");
  Serial.print(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
  Serial.println(" byte");

  Serial.print("Heap IRAM disponibile: ");
  Serial.print(heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
  Serial.println(" byte");
}
