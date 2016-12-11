#!/usr/bin/env python
import random
import string


#stores the string that will be displayed on screen
displayString = ''

#loops through to create 3 files and 10 lower case characters and \n in each file
for i in range(0,3):
    file = open("alphabet"+str(i)+".txt", 'w+')
    for j in range(0,10):
        char = random.choice(string.ascii_lowercase)
        displayString += char
        file.write(char)
    file.write("\n")
    displayString += '\n'
    file.close()

#displays string and numbers and product of the two numbers
print(displayString)
number1 = random.randint(1, 42)
number2 = random.randint(1, 42)
print(number1)
print(number2)
print(number1*number2)
