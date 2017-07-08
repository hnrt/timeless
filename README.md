# Timeless

This is a Java class implementing Win32/Linux API hooking JNI for unit-testing date-time dependent applications.

Suppose that you're using Calendar.getInstance() in your code and you want to change what it returns in order to test various situations in your unit test code, what would you do?

In general, as it is hard to mock a static method of the system library like Calendar.getInstance(), you would need to wrap it with your own class and then to mock the wrapping class. It would be fine if you could do so.

The class provided by this repository tries to approach to this matter in a different way. This is not a universal solution, but it might work very well in certain situations.

With this class, you don't have to mock any time related classes; you can easily set the current date as you want.
