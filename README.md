# CSWallhack
CounterStrike wall hack, tool to see through wall
------------------------------------------------
How to use
------------------------------------------------
1. Download or compile wallhack.dll
2. Download or compile dllInjector, which I've forked to https://github.com/zhutoulala/dllinjector, and added Windows 10 support
3. Lauch CounterStrike game and find its process id
4. Run below command
```
dllInjector-x86.exe -p [pid] -l wallhack.dll -c
```
5. Press F1 in game to enable the hack

Note: It was tested to be working with CS 1.6. It's assumed not to be working with CS:GO