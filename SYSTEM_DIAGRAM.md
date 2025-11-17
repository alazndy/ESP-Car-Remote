# System Diagram: ESP32 with INMP441 I2S Microphone

This diagram shows the connection between the ESP32 and the INMP441 I2S microphone.

```mermaid
graph TD
    subgraph ESP32
        direction LR
        A[GPIO 26]
        B[GPIO 25]
        C[GPIO 22]
        D[3.3V]
        E[GND]
    end

    subgraph INMP441
        direction LR
        F[SCK]
        G[WS]
        H[SD]
        I[VCC]
        J[GND]
    end

    A -- I2S Clock --> F
    B -- I2S Word Select --> G
    C -- I2S Data --> H
    D -- Power --> I
    E -- Ground --> J
```

## Connections

| INMP441 Pin | ESP32 Pin | Description      |
|-------------|-----------|------------------|
| SCK         | GPIO 26   | I2S Serial Clock |
| WS          | GPIO 25   | I2S Word Select  |
| SD          | GPIO 22   | I2S Serial Data  |
| VCC         | 3.3V      | Power            |
| GND         | GND       | Ground           |
