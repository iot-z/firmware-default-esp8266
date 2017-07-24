# IoT-Z Firmware

Default firmware of Module (C++)

## Mode config Network
Connect on the network created by the module
Pass: 123456789 (is the detault, but can be changed)
Access the config page address: 192.168.4.1

## Todo

- Add a RGB led for inform state
    - Yellow: Button reset Pressed
    - Red: Button reset Pressed for a long time (Format)
    - Blue: Config mode
    - Green: Slave mode
        - Blinking rapid: Trying to connect to WIFI AP
        - Blinking slowly: Trying to connect to Server UDP
        - Static: Ok and connected to the Server

# Thanks to

@MarcioLaubstein for teaching me some C++ basics
