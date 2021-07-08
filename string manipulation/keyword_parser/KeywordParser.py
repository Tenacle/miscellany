import sys
import os
import re

#TODO: don't use the sys vars throughout the code. it doesn't work running it outside the terminal

to_parse_string = ''    # the file stringified for parsing (if it contains any keywords)
keywords_arr = []       # contains the keywords for each keyword file
keyword_type = []       # contains the type/name of the keyword group

# if there are no specified arguments then request input from users
if len(sys.argv) <= 1:
    sys.argv.append(input("Enter file path to file that will be parsed: "))
    while True:
        usr_input = input("Enter file path to keywords (or 'OK' if done): ")
        if usr_input == "OK":
            break
        sys.argv.append(usr_input)

# open and parse the files and store them as variables
if len(sys.argv) > 1:
    while True:
        if os.path.isfile(sys.argv[1]):
            with open(sys.argv[1], "r", encoding='utf8') as to_parse_file:
                to_parse_string = to_parse_file.read().replace('\n', ' ')
            break
        else:
            usr_input = input("File not found. enter file to be parsed (or QUIT): ")
            if usr_input == "QUIT":
                exit()
            else:
                sys.argv[1] = usr_input

    print(len(sys.argv))
    for i in range(2, len(sys.argv)):
        if os.path.isfile(sys.argv[i]):
            with open(sys.argv[i], "r", encoding='utf8') as keyword_file:
                keywords_arr.append(list(set((filter(None, keyword_file.read().lower().splitlines())))))
                keyword_name = input("Keyword {0} name (leave blank to use filename):".format(sys.argv[i]))
                if keyword_name.isalnum():
                    keyword_type.append(keyword_name)
                else:
                    keyword_type.append(sys.argv[i])
        else:
            print("Keyword file {} not found. ignoring..".format(sys.argv[i]))

# look for keywords. seperate each sentences
output_arr = list(filter(None, re.split(r"(?<!\w\.\w.)(?<![A-Z][a-z]\.)(?<=[.?!;:])\s", to_parse_string)))
# go through the array of sentences
for i in range(len(output_arr)):
    # check if the sentence is a header
    if re.search(r"^\s*#\s.*$", output_arr[i]):
        continue
    # go through the array of keyword lists
    for j in range(len(keywords_arr)):
        # go through the array of a keyword list
        for keyword in keywords_arr[j]:
            keyword_index = re.search(r"\b{}\b".format(keyword), output_arr[i].lower())
            if keyword_index:
                keyword_index = keyword_index.start()
                if keyword_index is not -1:
                    output_arr[i] = output_arr[i][0:keyword_index] + "**" + output_arr[i][keyword_index:keyword_index+len(keyword)] + "**" + output_arr[i][keyword_index+len(keyword):len(output_arr[i])] + str(" (***{}***)".format(keyword_type[j]))
                    brea

# store the parsed string into a file
output_file = open("./{}.output.md".format(sys.argv[1]), "w")
counter = 1
for i in range(len(output_arr)):
    if re.search(r"^\s*#\s.*$", output_arr[i]):
        output_file.write("#{}\n".format(output_arr[i]))
        counter = 1
        continue
    output_file.write("{0}. {1}  \n".format(counter, output_arr[i]))
    counter += 1
output_file.close()

print("Finished searching through {}\nOutput file: {}".format(sys.argv[1], sys.argv[1]+".output.md"))
#while True:
#    if input("press any key to quit"):
#        break
