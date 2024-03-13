# CO2 Monitoring

Attempts to monitor CO2 with microcontrollers and sensors.

![screenshot of graph of CO2 over time](images/CO2_plot.png)

## Sensor used

[Sensirion SCD40](https://developer.sensirion.com/products-support/scd4x-co2-sensor/) -> [Sensirion_SCD4x_Datasheet.pdf](./Sensirion_SCD4x_Datasheet.pdf)

## Microcontroller used

[ESP8266 D1 Mini](https://www.wemos.cc/en/latest/d1/d1_mini.html)

## Requirements

[Platform IO](https://platformio.org/) for VSCode. Follow their guide to get started.

## How to build

1. Select correct board with Platform IO.
1. Build & upload with Platform IO
1. Connec to COM Port with serial monitor

## Plot data

Live-plot example data: use `PuTTY` to create a log file (`putty.log`), and use bash's `watch` to plot the data, e.g., using [gnuplot/eplot](https://gist.github.com/alifeee/2e1ea8ad5290a553316e715f658f1fd7).