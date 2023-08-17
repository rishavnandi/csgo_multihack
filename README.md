# CSGO MultiHack

A simple multi-hack for CSGO

## Features

- [x] Aimbot
- [x] Triggerbot
- [x] Glow
- [x] Recoil Control System
- [x] Skin Changer

You can modify the trigger bot activation key in this line in main.cpp

```cpp
// aimbot key
if (GetAsyncKeyState(VK_SHIFT))
    continue;
```

You can find the virtual key codes here https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

Thanks to Cazz for the memory header https://github.com/cazzwastaken/pro-bhop/blob/master/cheat/memory.h and Hazedumper for the offsets https://github.com/frk1/hazedumper/blob/master/csgo.hpp'

Follow Cazz's tutorial to compile the cheat https://youtu.be/gzKVqeu5H28