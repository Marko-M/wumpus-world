Wumpus World in C
============

Hunt the Wumpus is an early video game where player navigates cave in search for gold avoiding dangerous monsters (Wumpus) and bottomless pits. In modern studies of intelligent system this world is very often modeled using various programming languages where computer tries to intelligently find gold without being killed. This is my implementation of Wumpus World in C programming language.

Requirements
------------

* Any standard C library and compiler.

Instructions for a Linux based operating system
----------------------------------------------

First make sure you have C library and GCC compiler installed. If you're using Ubuntu based Linux distribution you can use following command to install required tools:

    sudo apt-get install build-essential

Now you can get the latest package, unpack it and compile the code using following commands:

    wget -c https://github.com/Marko-M/wumpus-world/archive/master.zip && unzip master.zip
    cd wumpus-world-master/src/
    make
    chmod +x wumpus-world
    ./wumpus-world

You can play the game from anywhere in your system by creating a symlink. Run the following command in the src directory:

    sudo ln -s $(pwd)/wumpus-world /usr/local/bin/wumpus-world
   
