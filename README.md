# Ubi-House Carbon Dioxide Measuring Device

This repository contains the source code for the Ubi-House Carbon Dioxide Measuring Device. The device reads CO2 levels using the MHZ19 sensor, displays it on the M5StickC screen, and uploads the data to a WordPress site.

## Prerequisites

- Arduino IDE with support for M5StickC.
- Libraries:
  - M5StickC
  - MHZ19
  - WiFi
  - HTTPClient
  - ArduinoJson

## Setup

1. Clone this repository.
2. Open the `Ubi-House Carbon Dioxide Measuring Device.cpp` in Arduino IDE.
3. Install the required libraries using the Library Manager.
4. Update the `ssid`, `password`, `serverName`, and authentication credentials in the code.
5. Compile and upload the code to your M5StickC device.

## Usage

Once uploaded, the device will:
- Display CO2 levels in ppm.
- Indicate high CO2 levels with the built-in LED.
- Upload the data to the specified WordPress site every 10 seconds.

## License

This project is open source and available under the [MIT License](LICENSE).

## Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

