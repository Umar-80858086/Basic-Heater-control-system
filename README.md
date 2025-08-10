# Basic Heater Control System

A temperature-based heater control system for cooking devices, developed as part of the **Embedded Systems Intern Assignment** at **Upliance.AI**.  
The system monitors temperatures from multiple DS18B20 sensors and controls a heater to maintain desired cooking profiles while preventing overheating.
## read note 
> **NOTE**  
> I have implemented **Bluetooth advertising** to broadcast the current heating state.  
> There are a few steps required when implementing this in the real world:  
> - In the simulation, I commented out the Bluetooth code as it was not responding in Wokwi.  
> - The logic in the simulation is purely based on **temperature readings** and **target temperature**.  
> - When applying Bluetooth in a real setup, you can receive and send data (communicate with the Heater Control System) using:  
>   - **Android/Mobile** → Apps like *Serial Bluetooth Terminal* or *Serial Bluetooth HC-05*.  
>   - **PC** → Tools like *PuTTY* or *Arduino Serial Monitor*.  
>   i have attached LINK below 
> In the repository code, the **full Bluetooth implementation** is present.  
> In the simulation version, BT snippets are commented to avoid Wokwi limitations.

## Features
- **Three-point temperature sensing**:
  - **Bottom**: Direct heat input detection & boiling point.
  - **Side Wall**: Food mass temperature for simmer control.
  - **Lid**: Steam level monitoring.
- **State Machine Control**:
  - Idle → Heating → Stabilizing → Target Reached → Overheat.
- **Overheat Protection**: Automatic shutdown when temperature exceeds safe limits.
- **Bluetooth Interface**: Adjust target temperature and monitor states remotely (ESP32).
- **1-Wire Protocol**: Simple wiring and high reliability for DS18B20 sensors.

## Hardware Used
- **ESP32** microcontroller
- **3× DS18B20 temperature sensors**
- Relay module (for heater control)
- Buzzer (overheat alert)
- LED for Heater visual

## Communication Protocol
- **1-Wire** between ESP32 and DS18B20 sensors (minimal wiring, high noise immunity).
- **Bluetooth Classic** for wireless control and monitoring.

## Circuit Overview
- DS18B20 sensors connected to separate GPIO pins.
- Relay module controlled via ESP32 for heater ON/OFF.
- Buzzer connected for audible alerts.

## Future Roadmap
- **Fail-safe Overheat Protection** beyond current limits (sensor fault detection, rapid temp rise shutdown).
- **Multiple Heating Profiles**:
  - Fast Heat
  - Eco Mode
  - Hold Mode (with smooth transitions to avoid thermal shock)
- **PID or PWM Control** for finer temperature regulation.
- **Wi-Fi + Web UI** or **MQTT** for advanced remote monitoring.

## References
- [Wokwi Simulation Project](https://wokwi.com/projects/438829491939294209)  
- [GitHub Repository](https://github.com/Umar-80858086/Basic-Heater-control-system)  
- [ESP32 + DS18B20 Guide](https://www.makerguides.com/esp32-ds18b20-digital-thermometer/#Pin_Definition_of_DS18B20)
