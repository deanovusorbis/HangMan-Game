#include <Wire.h> // I2C library
#include <LiquidCrystal_I2C.h> // LCD I2C library
#include <Adafruit_SSD1306.h> // OLED library
#include <string.h> // For strlen and strchr

// Word list
const char* words[] = {"CODE", "GAME", "ARRAY", "LOOP", "PIXEL", "SCREEN", "BUTTON", "PLAYER", "LEVEL", "LOGIC", "MEMORY", "OBJECT", "STRUCT", "POINTER", "RANDOM", "INPUT", "OUTPUT", "SIGNAL", "VECTOR", "NUMBER", "STRING", "TARGET", "SPRITE", "ACTION", "ENGINE"}; 

// Game variables
const char* selectedWord; // Pointer to the selected word
int wordCount = sizeof(words) / sizeof(words[0]); // Number of words in the list
int wordLength; // Length of the selected word
int wrongGuesses = 0; // Number of wrong guesses
char currentLetter = 'A'; // Currently selected letter
bool joyLocked = false; // Joystick lock to prevent rapid changes
bool openedLetters[8]; // Track opened letters (max word length 7 and a null terminator equals 8)
bool usedLetters[26]; // Track used letters

// LCD and OLED initialization
LiquidCrystal_I2C lcd(0x27, 20, 4); // LCD I2C address 0x27, 20 columns, 4 rows
Adafruit_SSD1306 display(128, 64, &Wire); // OLED display 128x64

void setNewGame() // Function to set up a new game
{
	// Clear displays
	lcd.clear(); // Clear LCD
	display.clearDisplay(); // Clear OLED
	display.display(); // Update OLED

	// Reset game variables
	wrongGuesses = 0; // Reset wrong guesses
	currentLetter = 'A'; // Reset current letter
	joyLocked = false; // Unlock joystick

	// Select a new word
	int index = random(wordCount); // Random index
	selectedWord = words[index]; // Select word
	wordLength = strlen(selectedWord); // Get word length

	// Reset opened letters
	for (int i = 0; i < 8; i++) // For each letter in the word is hidden
		openedLetters[i] = false;
	
	for (int i = 0; i < 26; i++) // Reset used letters
		usedLetters[i] = false;

	// Wrong letters header
	lcd.setCursor(0, 0);
	lcd.print("Wrong:");

	// Draw word lines
	lcd.setCursor(0, 3);
	for (int i = 0; i < wordLength; i++)
		lcd.print("_ ");

	// Gallows
	lcd.setCursor(17, 0);
	lcd.print("___");
	lcd.setCursor(16, 1);
	lcd.print("|");
	lcd.setCursor(16, 2);
	lcd.print("|");
	lcd.setCursor(16, 3);
	lcd.print("|");
	/* Gallows structure:
	 ___
	|
	|
	|
	*/
}

void setup()
{
	// Initilize pins
	pinMode(2, INPUT_PULLUP); // for joystick button 

	// For random
	randomSeed(analogRead(A1));

	// LCD setup
	lcd.init(); // Initialize the LCD
	lcd.backlight(); // Turn on backlight

	// OLED setup
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Initialize OLED with I2C address 0x3C
	display.clearDisplay(); // Clear OLED
	display.display(); // Update OLED

	// Start new game
	setNewGame();
}

void updateLetterFromJoystick() // Update current letter based on joystick input
{
	// Read joystick X-axis
	int xValue = analogRead(A0); // Read X-axis value
	if (!joyLocked) // If joystick is not locked
	{
		if (xValue < 300) // Move left
		{
			if (currentLetter == 'A') // Wrap around
				currentLetter = 'Z';
			else // Move to previous letter
				currentLetter--;
			joyLocked = true;
		}
		else if (xValue > 700) // Move right
		{
			if (currentLetter == 'Z') // Wrap around
				currentLetter = 'A';
			else // Move to next letter
				currentLetter++;
			joyLocked = true;
		}
	}
	else
	{
		if (xValue > 450 && xValue < 550)
			joyLocked = false;
	}
}

void drawCurrentLetter() // Draw the currently selected letter on OLED
{
	display.clearDisplay();
	display.setCursor(40, 16);
	display.setTextSize(4);
	display.setTextColor(SSD1306_WHITE);
	display.print(currentLetter);
	display.display();
}

bool isWordGuessed() // Check if the entire word has been guessed
{
	for (size_t i = 0; i < wordLength; i++)
	{
		if (!openedLetters[i])
			return false;
	}
	return true;
}
void loop()
{
	// Check for win condition
	if (isWordGuessed())
	{
		lcd.clear();
		lcd.setCursor(7, 1);
		lcd.print("YOU WIN!");
		lcd.setCursor(0, 3);
		lcd.print("Tap joystick to restart");
		while (digitalRead(2) == LOW); // Wait for joystick release
		while (digitalRead(2) == HIGH); // Wait for joystick press to restart
		setNewGame();
	}
	updateLetterFromJoystick();
	drawCurrentLetter();
	// Read joystick for letter selection
	if (digitalRead(2) == LOW && wrongGuesses < 5) // If joystick button is pressed
	{
		if (usedLetters[currentLetter - 'A']) // If letter has already been used
		{
			display.clearDisplay();
			display.setTextSize(1);
			display.setTextColor(SSD1306_WHITE);
			display.setCursor(10, 24);
			display.print("Letter ");
			display.print(currentLetter);
			display.print(" already used");
			display.display();
			delay(700);
		}
		else
		{
			usedLetters[currentLetter - 'A'] = true; // Mark letter as used
			delay(200); // Debounce delay
			if (strchr(selectedWord, currentLetter)) // If letter is in the word
			{
				for (size_t i = 0; i < wordLength; i++)
				{
					if (selectedWord[i] == currentLetter)
					{
						lcd.setCursor(i * 2, 2);
						lcd.print(currentLetter);
						openedLetters[i] = true;
					}
				}
			}
			else // If letter is not in the word
			{
				wrongGuesses++;
				lcd.setCursor(5 + wrongGuesses, 0);
				lcd.print(currentLetter);
				lcd.print(" ");
			}
		}
	}
	switch (wrongGuesses)
	{
	case 1:
		lcd.setCursor(18, 1);
		lcd.print("O");
		break;
		/*
		Gallows with head:
		 ___
		| O
		|
		|
		*/

	case 2:
		lcd.setCursor(18, 2);
		lcd.print("|");
		break;
		/*
		Gallows with head and body:
		 ___
		| O
		| |
		|
		*/

	case 3:
		lcd.setCursor(17, 2);
		lcd.print("/");
		lcd.setCursor(19, 2);
		lcd.print("\\");
		break;
		/*
		Gallows with head, body, and arms:
		 ___
		| O
		|/|\
		|
		*/

	case 4:
		lcd.setCursor(17, 3);
		lcd.print("/");
		lcd.setCursor(19, 3);
		lcd.print("\\");
		break;
		/*
		Gallows with full figure:
		 ___
		| O
		|/|\
		|/ \
		*/
	case 5:
		// Game over
		joyLocked = true; // Lock joystick
		lcd.setCursor(18, 1);
		lcd.print("X");
		/* Final state (head detached):
		 ___
		| X
		|/|\
		|/ \
		*/
		delay(1000);
		lcd.clear();
		lcd.setCursor(6, 1);
		lcd.print("GAME OVER");
		lcd.setCursor(4, 2);
		lcd.print("Word: ");
		lcd.print(selectedWord);
		lcd.setCursor(0, 3);
		lcd.print("Tap joystick to restart");
		while (digitalRead(2) == LOW); // Wait for joystick release
		while (digitalRead(2) == HIGH); // Wait for joystick press to restart
		setNewGame(); // Restart the game
		break;
	default:
		break;
	}
}
