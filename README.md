# Locker Device
The device is an Arduino Uno, with HC-05 Bluetooth module, to simulate a locker, which must be opened or closed if the user is authorized

# Usage
Install the external libaries:
1. Base64 from https://github.com/agdl/Base64
2. AES-128 Enc/Dec from https://github.com/DavyLandman/AESLib
3. SHA-256 Hmac from https://github.com/daknuett/cryptosuite2

Connect the Bluetooth module, on these Arduino pins:
- A2 // PIN bluetooth power
- A3 // PIN bluetooth state
- A4 // PIN bluetooth reception
- A5 // PIN bluetooth transmission

Run the code on your device

When the operation is successfull, the code prints the string:
"OP DONE!"
(You can use this software to authorize any operation (open/lock a locker, turn on/off the light...)

## It is safe to regenerate the key, before using it

# Other Software
Mobile App (React Native iOS/Android): https://github.com/DistributedSystemsProject/MobileApp

Authorization Server (Lua + Redis DB): https://github.com/DistributedSystemsProject/AuthorizationServer
