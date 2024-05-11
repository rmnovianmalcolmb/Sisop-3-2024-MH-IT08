# Soal nomor 3
## actions.c

This repository contains a set of functions to assist with racing strategy decisions based on various factors such as gap to leader, fuel level, tire wear, and tire type.

```bash
#include <stdio.h>
#include <string.h>

char* handleGap(float gap) {
    if (gap < 3.5) {
        return "Gogogo";
    } else if (gap >= 3.5 && gap <= 10) {
        return "Push";
    } else {
        return "Stay out of trouble";
    }
}

char* handleFuel(float fuel) {
    if (fuel > 80) {
        return "Push Push Push";
    } else if (fuel >= 50 && fuel <= 80) {
        return "You can go";
    } else {
        return "Conserve Fuel";
    }
}

char* handleTire(int tireWear) {
    if (tireWear > 80) {
        return "Go Push Go Push";
    } else if (tireWear >= 50 && tireWear <= 80) {
        return "Good Tire Wear";
    } else if (tireWear >= 30 && tireWear < 50) {
        return "Conserve Your Tire";
    } else {
        return "Box Box Box";
    }
}

char* handleTireChange(char* tireType) {
    if (strcmp(tireType, "Soft") == 0) {
        return "Mediums Ready";
    } else if (strcmp(tireType, "Medium") == 0) {
        return "Box for Softs";
    } else {
        return "Invalid Tire Type";
    }
}
```

## Fungsi Functions

### 1. `handleGap(float gap)`

- Mengembalikan "Gogogo" jika gap kurang dari 3.5.
- Mengembalikan "Push" jika gap antara 3.5 dan 10.
- Mengembalikan "Stay out of trouble" jika gap lebih dari 10.

### 2. `handleFuel(float fuel)`

- Mengembalikan "Push Push Push" jika bahan bakar lebih dari 80.
- Mengembalikan "You can go" jika bahan bakar antara 50 dan 80.
- Mengembalikan "Conserve Fuel" jika bahan bakar kurang dari 50.

### 3. `handleTire(int tireWear)`

- Mengembalikan "Go Push Go Push" jika keausan ban lebih dari 80.
- Mengembalikan "Good Tire Wear" jika keausan ban antara 50 dan 80.
- Mengembalikan "Conserve Your Tire" jika keausan ban antara 30 dan 50.
- Mengembalikan "Box Box Box" jika keausan ban kurang dari 30.

### 4. `handleTireChange(char* tireType)`

- Mengembalikan "Mediums Ready" jika jenis ban "Soft".
- Mengembalikan "Box for Softs" jika jenis ban "Medium".
- Mengembalikan "Invalid Tire Type" jika jenis ban tidak valid.

## How to Use

1. Include the header file `#include "racing_strategy.h"` in your C program.
2. Call the appropriate function with the relevant parameters to get the racing strategy advice.
