# ESP Hardware

## ESP 32 Pinout

| GPIO | Function               |
|------|----------              |
|00    |RETCLK                  |
|12    |Output detection        |
|13    |Input detection         |
|14    |Master & Slave          |
|17    |NC		        		|
|18    |MDC                     |
|19    |TX 0                    |
|21    |TX Enable               |
|22    |TX 1                    |
|23    |MDIO                    |
|25    |RX 0                    |
|26    |RX  1                   |
|27    |CRS                     |


## Was musste am PHY geändert werden?

- Remove ClockGen from LAN8720 Board and use ESP32 as source
