# LEDpicker

### "Let's take it to the next level"

#### This allows you to easily pick colors when using ESP32 and a String of LEDs.

![ledcolor](https://github.com/user-attachments/assets/9ebe9d09-8b2b-4ac8-802b-5c9fc22b6131)

I have seen Color Charts with Hex Color codes and Decimal Color codes.

Using these can be **difficult** because you only know what you really have when you Upload the .ino file and is **especially difficult** if you are using **low brightness levels and low color** values to conserve power.

#### But this combines everything thing together.  It provides easy color selection with sliders and displays:

+ Decimal Codes

+ Hex Codes

+ Color Box

Pressing the "Send Button" will upload the codes to an ESP32 with chain of LEDs and will light up selected LEDs on your chain, so you know exactly what that color looks like.

If it is not quite right, then move the Sliders again and press the Button to send new codes.

**Note: In the above example,** the decimal values are less than 128. The color box shows "Dark Grey (almost Black)" but the LEDs are "Dim White" which is what I want.

Now copy the codes and paste them into your code.

### And it's fun to use!!
