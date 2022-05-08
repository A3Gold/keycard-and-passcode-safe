#include <Adafruit_GFX.h>                              // Includes library for the OLED
#include <Adafruit_SSD1306.h>                          // Includes library for the OLED
#include <Wire.h>                                      // Includes I2C library
#include <Keypad.h>                                    // Includes keypad library
#include <SPI.h>                                       // Includes SPI library
#include <MFRC522.h>                                   // Includes RFID reader library
#include <Servo.h>.                                    // Includes servo library
#define RST_PIN 9                                      // Defines RFID RST pin
#define SS_PIN 10                                      // Defines RFID SS pin
#define SERVO_PIN A0                                   // Defines servo control pin
#define SCREEN_WIDTH 128                               // Defines screen width, in pixels
#define SCREEN_HEIGHT 32                               // Defines screen height, in pixels
#define CARDID "02 CD 2D 34"                           // Defines UID of correct card
#define PASSCODELENGTH 4                               // Defines length of passcode
#define ROWS 4                                         // Defines keypad rows
#define COLUMNS 3                                      // Defines keypad columns
uint8_t advance = 0;                                   // Variable for how far code should advance
uint8_t correctDigits = 0;                             // Holds how many correct code digits entered
uint8_t cursorPosition = 0;                            // Holds cursor position for passcode
uint8_t codePosition = 0;                              // Holds number of passcode digits entered
char passcode[PASSCODELENGTH] = {'1', '2', '3', '4'};  // Correct passcode
char inputCode[PASSCODELENGTH] = {'0', '0', '0', '0'}; // Blank input code
uint8_t rowPins[ROWS] = {5, 6, 7, 8};                  // Keypad row pins
uint8_t colPins[COLUMNS] = {4, 3, 2};                  // Keypad column pins
char keys[ROWS][COLUMNS] = {
    // Keypad map
    {'1', '2', '3'}, // Row 1 configuration
    {'4', '5', '6'}, // Row 2 configuration
    {'7', '8', '9'}, // Row 3 configuration
    {'*', '0', '#'}  // Row 4 configuration
};
// Configures keypad based on given information...
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLUMNS);
// Configures display based on given information...
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// Configures RFID reader based on given information...
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo SERVO; // Creates object for the servo

void setup()
{
    SPI.begin();                               // Initializes SPI bus
    mfrc522.PCD_Init();                        // Initializes RFID reader
    SERVO.attach(SERVO_PIN);                   // Creates servo
    SERVO.write(90);                           // Sets servo to locked position
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Starts OLED
    display.setTextSize(1);                    // Sets text size to default
    display.setTextColor(WHITE);               // Sets text colour to white
    setDisplay();                              // Prepares display... prints message...
    display.print("Enter Passcode - * to Reset; # to enter");
    display.display(); // Displays message
}

void setDisplay()
{                            // Function to prepare display
    display.clearDisplay();  // Clears text
    display.setCursor(0, 0); // Sets cursor to home
}

void loop()
{
    // Waits for key to be pressed and assigns it to char
    char inputKey = keypad.getKey();
    if (advance == 0)
    { // First step: passcode
        if (inputKey)
        { // If key has been pressed...
            switch (inputKey)
            {                           // check it with case statement
            case '*':                   // If * pressed, reset input
                correctDigits = 0;      // Resets variable
                cursorPosition = 0;     // Resets variable
                codePosition = 0;       // Resets variable
                setDisplay();           // Prepares display
                display.print("Reset"); // Prints message
                display.display();      // Displays message
                delay(1000);            // Delay
                setDisplay();           // Prepares display... prints message
                display.print("Enter Passcode - * to Reset; # to enter");
                display.display(); // Displays message
                break;             // End case
            case '#':              // If # pressed, check passcode
                for (uint8_t i = 0; i < PASSCODELENGTH; i++)
                {
                    if (inputCode[i] == passcode[i])
                    {                    // Compares input code...
                        correctDigits++; // with passcode
                    }                    // If digit equal, increase variable
                }                        // When 4 digits equal, advance
                if (correctDigits == PASSCODELENGTH)
                {
                    setDisplay(); // Prepares display... prints message
                    display.print("Passcode Correct");
                    display.display(); // Displays message
                    delay(1000);       // Delay
                    advance++;         // Move on to next step
                }
                else
                {                       // If passcode incorrect, reset for new input
                    codePosition = 0;   // Resets variable
                    cursorPosition = 0; // Resets variable
                    correctDigits = 0;  // Resets variable
                    setDisplay();       // Prepares display... prints message
                    display.print("Passcode Incorrect");
                    display.display(); // Displays message
                    delay(1000);       // Delay
                    setDisplay();      // Prepared display... prints message
                    display.print("Enter Passcode - * to Reset; # to enter");
                    display.display(); // Displays message
                }
                break; // End case
            default:   // When any other key pressed...
                // put input key into inputCode array
                inputCode[codePosition] = inputKey;
                // Advance and set display cursor
                cursorPosition = cursorPosition + 10;
                display.setCursor(cursorPosition, 20);
                // Prints digit entered
                display.print(inputCode[codePosition]);
                display.display(); // Displays digit
                codePosition++;    // Advances code position
            }
        }
    }
    if (advance == 1)
    {                                // When passcode correct...
        setDisplay();                // Prepares displays
        display.println("Tap Card"); // Prints message
        display.display();           // Displays message
        if (!mfrc522.PICC_IsNewCardPresent())
        {
            return; // If RFID card near reader advance...
        }           // otherwise, return to top of statement
        if (!mfrc522.PICC_ReadCardSerial())
        {
            return;             // If RFID data available advance...
        }                       // otherwise, return to top of statement
        String readCardID = ""; // Prepare string for UID
        // Creates string with card's UID
        for (uint8_t i = 0; i < mfrc522.uid.size; i++)
        {
            readCardID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
            readCardID.concat(String(mfrc522.uid.uidByte[i], HEX));
        }                         // Converts it to HEX
        readCardID.toUpperCase(); // Converts it to upper case
        // If UID matches correct one...
        if (readCardID.substring(1) == CARDID)
        {
            setDisplay(); // Prepares display... prints message
            display.print("Safe Open - Press * (asterisk) To Close");
            display.display(); // Displays message
            SERVO.write(0);    // Opens safe by turning servo
            advance++;         // Advances code position
        }
        else
        {                                    // If card not correct...
            setDisplay();                    // Prepares display
            display.print("Incorrect Card"); // prints message
            display.display();               // Displays message
            delay(1000);                     // Delay
        }
    }
    if (advance == 2)
    { // If safe open...
        if (inputKey == '*')
        {                                 // and * pressed
            setDisplay();                 // Prepares display
            display.print("Safe Closed"); // Prints message
            display.display();            // Displays message
            delay(1000);                  // Delay
            setDisplay();                 // Prepares display
            display.print("Power Off");   // Prints messafe
            display.display();            // Displays message
            SERVO.write(90);              // Lock safe by turning servo
            while (1)
                ; // Wait forever; user must power off
        }
    }
}
