# CSWallhack
Caution: Use at your own risk

CounterStrike and CS:GO wall hack, tool to see through wall.
CS:GO 穿墙和自动瞄准外挂
------------------------------------------------
CS:GO
------------------------------------------------
1. Download or compile CSGOWall.dll
2. Download or compile dllInjector, which I've forked to https://github.com/zhutoulala/dllinjector, and added Windows 10 support
3. Launch CS:GO game and find its process id
4. Run below command
```
dllInjector-x86.exe -p [pid] -l CSGOWall.dll -c
```
5. Press Insert in game to see the hacking menu
Caution: this hacking method is subject to be detected. Again, use at your own risk

使用方法
1. 下载或者编译 CSGOWall.dll
2. 下载或者编译 dllInjector, 可以在我的fork repo里面找到
3. 启动游戏，找出游戏进程ID
4. 打开命令行，运行如下命令
```
dllInjector-x86.exe -p [pid] -l CSGOWall.dll -c
```
5. 在游戏按下Insert键开启作弊菜单
注意：风险自负


CS 1.6
------------------------------------------------
1. Download or compile wallhack.dll
2. Download or compile dllInjector, which I've forked to https://github.com/zhutoulala/dllinjector, and added Windows 10 support
3. Launch CounterStrike game and find its process id
4. Run below command
```
dllInjector-x86.exe -p [pid] -l wallhack.dll -c
```
5. Press F1 in game to enable the hack
