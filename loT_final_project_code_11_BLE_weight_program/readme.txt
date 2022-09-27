If you are going to execute or run our program.
We use Arduino IDE as our editor and compiler.
First, you should add two zip header files which are 'Grove_LED_Bar-master.zip' and 'HX711_ADC-master.zip' to the Arduino IDE.
Then load our code to LinkIt 7697 board.

Our code will first initialize the weight module by calculating the offset.
This step could let the user get much precise weight data.
After initializing the weight module, we can put things on the weight module and we could get a precise weight value.
And the weight will change into a visible stage and show on the LED bar.
If we want to transmit the data to the phone by bluetooth.
First, we should connect the phone and LinkIt 7697.
Then, press the USR button on the LinkIt 7697, then LinkIt 7697 will send the data to the phone.
Simultaneously, Led light on LinkIt 7697 will turn on to show that it is in sending mode, and the LED bar will turn into a loading bar.
After sending data to phone, LinkIt 7697 will turn back to original mode, then user can continue getting weight.
