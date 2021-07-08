# Question 1
## Description
> Exercise 1.4. Implement shift cipher and vigenere cipher attacks that are designed to be automated (i.e. the index of coincidence)
Implemented using python.

## Running
File name of code: atk_cipher.py
To launch the program we have to type in the terminal: PROGRAM_NAME.PY ENCRYPTED_FILE
* The one argument is the file to attack/decrypt,
  * If none were selected then user needs to put in ciphertext to decrypt.
* Prompt to select "**shift**" or "**vigenere**". Defaults to vigenere.
* Prompt to select key length $$t$$ if vigenere cipher is selected.

## Output
The decrypted file is stored in "deciphered.txt". Both ciphers will still output the decoded message and key in the terminal.


## Notes
* I encrypted my test cases in https://cryptii.com/pipes/vigenere-cipher because I was too lazy to implement one myself. (I hope that's not an issue since it's not really part of the question)
* I limited the key length search to $$|m|/30$$ because any more and it would be impossible for the attack to be successful.




