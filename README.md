# Timeless

This is a Java class implementing Win32/Linux API hooking JNI for unit-testing date-time dependent applications.

Suppose that you're using Calendar.getInstance() in your code and you want to change what it returns in order to test various situations in your unit test code, what would you do?

In general, as it is hard to mock a static method of the system library like Calendar.getInstance(), you would need to wrap it with your own class and then to mock the wrapping class. It would be fine if you could do so.

A class `TimeMachine` provided by this project tries to approach to this matter in a different way. This is not a universal solution, but it might work very well in certain situations.

With this class, you don't have to mock any time-related classes; you can easily set the current date as you want.

A unit test class `TimeMachineTest` has been tested on Windows 10 and Linux/Cent OS 6. The classes working properly with TimeMachine class are Date, Calendar, LocalDateTime, and ZonedDateTime.

## How to run the unit test class on Windows platform

Prepare Microsoft Visual Studio. I'm using Microsoft Visual Studio Community 2017, which is available free of charge, on Windows 10 Pro x64.

Start Visual Studio IDE, open TimeMachineWin32.sln, and build the solution with the configuration set to `Debug|x64`.

You can build the solution at a command line, too, as follows.

Open x64 Native Tools Command Prompt for VS 2017.

Change the current directory to TimeMachineWin32 in this project.

Build a JNI dynamic link library as follows.

    devenv TimeMachineWin32.sln /rebuild "Debug|x64"

You'll have TimeMachine.dll in TimeMachineWin32\bin\x64\Debug directory.

Then, start Eclipse, import the project in TimeMachine directory, and run the unit test class.

You can run the unit test class at a command line, too, as follows.

Change the current directory to TimeMachine in this project.

Create bin and lib directory.

    mkdir bin
    mkdir lib

Download the JUnit jar 4.10 from http://junit.org/ if you don't have it yet and put the jar file in lib directory.

Compile TimeMachine class. (The path to bin directory in the JDK package is assmed to be set with PATH environment variable.)

    javac -d bin src\com\hideakin\lib\time\TimeMachine.java
    javac -d bin -cp bin;lib\junit-4.10.jar testsrc\com\hideakin\lib\time\TimeMachineTest.java

You'll have TimeMachine.class and TimeMachineTest.class in bin\com\hideakin\lib\time directory. (Note that on Windows a semicolon is used to separate paths with -cp option for javac.)

Run the unit test class as follows.

    java -cp bin;lib\junit-4.10.jar -Djava.library.path=..\TimeMachineWin32\bin\x64\Debug org.junit.runner.JUnitCore com.hideakin.lib.time.TimeMachineTest

You'll get the output something like this:

    JUnit version 4.10
    .0=Sat Jul 08 12:51:41 JST 2017
    LocalDateTime=2001-12-31T12:51:41.720
    LocalDateTime=2050-04-01T12:51:41.720
    9=Sat Jul 08 12:51:41 JST 2017
    .0=Sat Jul 08 12:51:41 JST 2017
    ZonedDateTime=2001-12-31T12:51:41.722+09:00[Asia/Tokyo]
    ZonedDateTime=2050-04-01T12:51:41.722+09:00[Asia/Tokyo]
    9=Sat Jul 08 12:51:41 JST 2017
    .0=Sat Jul 08 12:51:41 JST 2017
    Calendar=Mon Dec 31 12:51:41 JST 2001
    Calendar=Fri Apr 01 12:51:41 JST 2050
    9=Sat Jul 08 12:51:41 JST 2017
    .0=Sat Jul 08 12:51:41 JST 2017
    Date=Mon Dec 31 12:51:41 JST 2001
    Date=Fri Apr 01 12:51:41 JST 2050
    9=Sat Jul 08 12:51:41 JST 2017

    Time: 0.171

    OK (4 tests)

If you use junit 4.12, you might need hamcrest-core 1.3, too. Include it in -cp option.

    java -cp bin;lib\junit-4.12.jar;lib\hamcrest-core-1.3.jar -Djava.library.path=..\TimeMachineWin32\bin\x64\Debug org.junit.runner.JUnitCore com.hideakin.lib.time.TimeMachineTest


