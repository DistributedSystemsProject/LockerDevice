# Locker Device
The device is an Arduino Uno, with HC-05 Bluetooth module, to simulate a locker, which must be opened or closed if the user is authorized

See https://github.com/DistributedSystemsProject/AuthorizationServer to understand the used protocol.

In the ECC branch there is the device code, using Elliptic-curve cryptography (ECC): https://github.com/DistributedSystemsProject/LockerDevice/tree/ecc

# Requirements
Install the external libaries:
1. Base64 from https://github.com/agdl/Base64
2. AES-128 Enc/Dec from https://github.com/DavyLandman/AESLib
3. SHA-256 Hmac from https://github.com/daknuett/cryptosuite2
4. JSON from https://github.com/bblanchon/ArduinoJson

# Usage
Connect the Bluetooth module, on these Arduino pins:
- A2 // PIN bluetooth power
- A3 // PIN bluetooth state
- A4 // PIN bluetooth reception
- A5 // PIN bluetooth transmission

Run the code on your device.

When the operation is successful, the code prints on the Serial Monitor, the following string:
"Operation DONE!"

# Other Software
Mobile App (React Native iOS/Android): https://github.com/DistributedSystemsProject/MobileApp

Authorization Server (Lua + Redis DB): https://github.com/DistributedSystemsProject/AuthorizationServer

# Known Issues
The code works only with Arduino AVR Boards 1.6.21. If you update to a newer version, the program can't start.
