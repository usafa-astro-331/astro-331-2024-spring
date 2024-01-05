# Getting started

You will need to install several programs. This folder contains programs and batch scripts to setup the correct computing environment. These tools are setup for Windows. Equivalent tools exist for Mac or Linux, but you'll have to install your own. 



## install git

Run the following commands from the Windows command prompt to install git and github. 

- open Windows command line
  - `WIN`-`r` -> cmd -> `OK`
- install **git** (version control software)
  - `winget install git.git`

- install **TortoiseGit** (windows explorer integration for git)
  - `winget install tortoisegit`




## clone the course repository

Go to My Documents. Do not create a new course folder yet. 

Right click -> Git clone

Paste the course URL (https://github.com/usafa-astro-331/astro-331-2024-spring) and click OK. 



## install tools

Go to the `computer_environment` directory. 

- Verify the contents of `install_tools.bat` (right click -> edit)
  - This batch script uses Windows’ built-in `winget` command line package management tool to install several programs you will use in the course. 
    - **Arduino IDE**
    - **arduino-cli** a scriptable command line tool for managing arduino boards and libraries
    - **marktext**: a markdown editor/viewer that you will use to view lab instructions
    - **teraterm**: a serial terminal you will use to power your ground station
    -  **notepad++**: a useful text editor

- After you’ve verified, double-click `install_tools.bat` to run it 



## update arduino

Verify and run `update_arduino.bat` which updates the Arduino IDE/environment using the `arduino-cli` tool

- updates Arduino environment

- installs drivers for newer SAMD-based Arduino boards such as Arduino MKR Zero




# before each lab

(This step is not necessary if you just cloned the course repository.)

**This step should be performed before every lab to ensure you're using current material.** 

- Pull latest changes from course repository
  - Right click the course folder (or any location within the course folder). 
  - Git Sync -> Pull
- Update Arduino environment using `install_libraries.bat` found in that lab's folder

