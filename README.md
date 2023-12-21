
# CS2 Music
1、Steam右键CS2 属性,选择DLC 勾选 Counter-Strike Workshop Tools  

2.等待下载完毕开启Workshop Tools 新建项目(Create New Addon) 命名必须英文 点击确定(Create) 双击启动项目  
3.在项目中，我删减了部分代码，因为他为我的公司出力,现有代码修改  **【std::string filename = "mp3.mp3";//音乐文件名】**修改文件名，为您所需要更改的音乐即可  
4.编译这份文件将sole.exe放置在Counter-Strike Global Offensive\content\csgo_addons\test\sounds【test为自己刚刚创建的项目名称】 将您的MP3、wav文件也一并放在这个目录下即可  
5.运行sole.exe您将会在Counter-Strike Global Offensive\game\csgo_addons\test\sounds得到一个mp3.vsnd_C文件  
他可以在CS2上使用 play mp3 播放 使用snd_toolvolume 可以调整声音大小  
#CS2 Music
1. Right-click on the CS2 attribute on Steam, select DLC and check Counter-Strike Workshop Tools

2. Wait for the download to complete and open Workshop Tools Create New Addon The name must be in English Click Create Double-click to start the project
3. In the project, I cut part of the code because he contributed to my company, and the existing code was modified **[std::string filename = "mp3.mp3"; Music File Name]**Modify the file name to the music you need to change
4. Compile this file and place the sole.exe in Counter-Strike Global Offensivecontentcsgo_addonstestsounds【test is the name of the project you just created】 Put your MP3 and wav files in this directory as well
5. Run sole.exe you will get a mp3.vsnd_C file in Counter-Strike Global Offensivegamecsgo_addonstestsounds
He can play mp2 on CS3 using snd_toolvolume and can adjust the sound volume
