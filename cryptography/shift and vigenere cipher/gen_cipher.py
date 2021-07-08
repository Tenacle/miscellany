#!/usr/bin/env python3
import sys

# This file does shift cipher encryption and decryption
# Shift cipher encryption is a fixed encryption cipher which replaces the letter by k spaces. k being the key.

# If no args on launch, there will be prompts to guide user
# Arg 1 will be file to encrypt or decrypt: ./sample.txt
# Arg 2 will be the action (encrypt or decrypt) "enc" or "dec"
# Output will be the file name + "_enc" or "_dec" (sample.txt_enc)

def shift_cipher(oFile, toDec, text):
    k = int(input("Input key 1-25: "))
    while k < 1 or k > 25:
        k = int(input("Input key 1-25: "))
    for c in text:
        # Leave non-alphabets as-is
        if ord(c) < 97 or ord(c) > 122:
            cc = c
        # Shift alphabets
        else:
            if toDec:
                cc = chr(((ord(c) - 97 - k) % 26) + 97)
            else:
                cc = chr(((ord(c) - 97 + k) % 26) + 97).upper()
        
        oFile.write(cc)

def vigenere_cipher(oFile, toDec, text):
    k = input("Enter key (must be alphabets): ")
    while not k.isalpha():
        k = input("Enter key (must be alphabets): ")
    counter = 0
    for c in text:
        if toDec:
            #TODO
            print("hhh")
        else:
            #TODO
            cc = chr((ord(c) - 97 + k[counter] % 26) + 97)
            counter = (counter + 1) % len(k)


def main():
    # check for args
    if len(sys.argv) == 2:
        try:
            with open(sys.argv[1], "r") as f:
                text = f.read().lower()
            print("File %s openened. " % (sys.argv[1]), end='')
            oFileName = sys.argv[1] 
        except:
            text = input("File %s did not exist.\nEnter a phrase to encrypt/decrypt: " % sys.argv[1]).lower()
            oFileName = "phrase"
    else:
        text = input("Enter a phrase to encrypt/decrypt: ").lower()
        oFileName = "phrase"

    # check the cipher
    cipher = ''
    while cipher != "shift" and cipher != "vigenere":
        cipher = input("Cipher (shift/vigenere): ")

    # check if user wants to encrypt or decrypt
    toDec = False
    while True:
        inp = input("Decrypting? (y/n): ")
        if inp == 'y':
            toDec = True
            oFileName += "_dec"
        elif inp == 'n':
            toDec = False
            oFileName += "_enc"
        else:
            continue
        break

    oFile = open(oFileName, "w")

    if cipher == "shift":
        print("Doing shift cipher")
        shift_cipher(oFile, toDec, text)
    elif cipher == "vigenere":
        print("Doing vigenere cipher")
        vigenere_cipher(oFile, toDec, text)
    else:
        print("Error")
        exit()

    oFile.close()

    print("Result (stored in %s):" % oFileName);
    with open(oFileName, 'r') as f:
        print(f.read());

main()
