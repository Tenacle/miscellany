import os       #
import json     # data handling
import re       # regex

# looks at possible matches in the dictionary without mixing words (doesn't reuse previous words)
#   > should be able to skip letters/spaces/etc.
# should be able to make multiple words
#   > can do this by recursively searching after it matches the first word

# future: probably matches with dank sentences
path = "sample.txt"
text = "she believed"
dictionary = {}

with open(path) as dict_file:
    words = dict_file.read().splitlines()
for word in words:
    if word[0] not in text:
        continue
    if word[0] not in dictionary.keys():
        dictionary[word[0]] = []
    dictionary[word[0]].append(word)
print(dictionary)
text = "".join(text.split())
for i in text:
    if i not in dictionary.keys():
        continue
    
    print(i)

# sbeve it


