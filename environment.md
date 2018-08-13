## Qt Environment Setting Up

### Installing Windows SDK for debuging
Windows SDK: https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk  

you can debug program on qt creator(qt creator will detect VS debugger automatically, otherwise, you can specify it yourself)  


### Installing VS2015 plugin (for creating qt project on VS2015)
Downloading VS Plugin: https://download.qt.io/official_releases/vsaddin/  

Installing pugin successfully, "Qt VS tools" option can be seen on VS main page options.  

### Qt Resources downloads
https://download.qt.io/  

the directorie contains QT development, Qt creator, VS addins and etc.  

NOTE: Qt creator is a development tools for programming, configure qt environment on qt creator as you need.  

### VS Tips
1. Enabling console output when developing UI program.  
VS specific configuration：  
Configuration Property -> Linker -> System -> Subsystem, choose '/SUBSYSTEM:CONSOLE'.  

