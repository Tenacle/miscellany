import os       #
import json     # data handling
import re       # regex
import pprint, random
import re

# looks at possible matches in the result without mixing words (doesn't reuse previous words)
#   > should be able to skip letters/spaces/etc.
# should be able to make multiple words
#   > can do this by recursively searching after it matches the first word

# future: probably matches with dank sentences
#path = "words_alpha.txt"
path = "words_alpha.txt"
pp = pprint.PrettyPrinter(indent = 4)
words = ''


with open(path) as dict_file:
    words = dict_file.read().splitlines()

print("Dictionary loaded. There are", len(words), "words stored")
maxLen = (input("Set max character length: "))
if maxLen == "" or int(maxLen) > 20:
    maxLen = 20
maxLen = int(maxLen)
minLen = (input("Set min character length: "))
if minLen == "" or int(minLen) < 0:
    minLen = 0
minLen = int(minLen)
noChar = input("Enter excluded chars (if any): ")

result = []
while True:

    # ask user for their input
    letters = input("Enter the first letter(s): ")
    # breaks loop if 0
    if letters == "0":
        break
    # use prev input if no input given
    elif letters != "":
        char = letters

    # dusting of MinMax and initializing var result
    minLen = len(char) if minLen < len(char) else minLen
    maxLen = len(char)+maxLen if maxLen < len(char) else maxLen
    result = []

    # parse dictionary to find results
    for word in words:
        n = len(word)
        if (not word.startswith(char) or 
                #TODO: I don't get why this is not working
                n < minLen or n > maxLen):
            continue

        if noChar and re.search(f"[{noChar}]", word) is not None:
            continue
        result.append(word)

    # result.sort(lambda x, y: cmp(len(x) < len(y)))
    # result.sort(key=len, reverse=True)

    # shuffle the list to output varying results
    random.shuffle(result)
    pp.pprint(result[:10])

input("Exit sequence initiated. press enter to close")
# sbeve it


