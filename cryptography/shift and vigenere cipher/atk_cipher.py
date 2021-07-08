#!/usr/bin/env python3
import sys
import math
import numpy

# This code performs attacks on a shift/vigenere cipher in an effort to retrieve the keys.

FREQDICT = [0.082, 0.015, 0.028, 0.043, 0.127, 0.022, 0.020, 0.061, 0.070, 0.002, 0.008, 0.040, 0.024, 0.067, 0.075, 0.019, 0.001, 0.060, 0.063, 0.091, 0.028, 0.010, 0.024, 0.002, 0.020, 0.001]
FREQI = 0.065
FREQEQ = 0.038

# from https://stackoverflow.com/questions/2566412/find-nearest-value-in-numpy-array
def find_index_of_nearest(arr, val):
    arr = numpy.asarray(arr)
    return (numpy.abs(arr - val)).argmin()

def find_index_of_biggest(arr):
    arr = numpy.asarray(arr)
    return arr.argmax()

def get_k(freq_list):
    ij_arr = [0.0] * 26     # square sum of a given freq list and freq dictionary
    for ij_index in range(26):
        ij_curr = 0
        index = 0

        for p in FREQDICT:
            ij_curr += (p * freq_list[(index + ij_index) % 26])
            index += 1
        ij_arr[ij_index] = ij_curr

    #k = find_index_of_nearest(ij_arr, FREQI)
    k = find_index_of_biggest(ij_arr)
    return k

def find_letter_freq(c):
    c_count = [0] * 26
    for char in c:
        char_int = ord(char) - 97
        if char_int >= 0 and char_int <= 25:
            c_count[char_int] += 1
    c_freq = [0.0] * 26
    c_len = float(len(c))
    index = 0
    if c_len != 0:
        for c_letter_index in c_count:
            c_freq[index] = c_letter_index/c_len
            index += 1

    return c_freq

def get_alpha_only(c):
    # strip characters not part of the M space
    c_stripped = ''
    for elem in c:
        if ord(elem) >= 97 and ord(elem) <= 122:
            c_stripped += elem
    return c_stripped

def get_sum_sqr(arr):
    sum_sqr = 0
    for elem in arr:
        sum_sqr += math.pow(float(elem), 2)

    return sum_sqr

def segment_string(c, t_curr, t):
    # find the chars related to segment t_curr
    c_seg = ''
    for index in range (t_curr, len(c), t):
        c_seg += c[index]
    return c_seg

def find_key_length(c):
    t_curr = 1
    t_best = -1
    t_max = len(c)/30       # must have a sufficient sample size n >= 30. Otherwise it's pointless
    sum_sqr_curr = 0
    sum_sqr_best = -1
    letter_freq_curr = []

    # get sum squared when t_curr = 1
    sum_sqr_curr = get_sum_sqr(find_letter_freq(c))
    #print("%3d @ %5f" % (t_curr, sum_sqr_curr))
    if numpy.abs(FREQI - sum_sqr_curr) < numpy.abs(FREQEQ - sum_sqr_curr):
        #print("SELECTED")
        sum_sqr_best = sum_sqr_curr
        t_best = t_curr

    # get values of other lengths
    found = False
    while t_curr < t_max:
        t_curr += 1
        sum_sqr_curr = 0
        letter_freq_curr = []

        # find the chars of length t
        c_seg = segment_string(c, 0, t_curr)
        
        # compute
        sum_sqr_curr = get_sum_sqr(find_letter_freq(c_seg))
        #print("%3d @ %5f" % (t_curr, sum_sqr_curr))
        
        if numpy.abs(FREQI - sum_sqr_curr) < numpy.abs(FREQEQ - sum_sqr_curr):
            #print("candidate found")
            for t_step in range(1, t_curr):
                c_step_seg = segment_string(c_seg, t_step, t_curr)
                sum_sqr_curr += get_sum_sqr(find_letter_freq(c_step_seg))
                #print("\t%3d @ %5f" % (t_step, get_sum_sqr(find_letter_freq(c_step_seg))))
            sum_sqr_curr = sum_sqr_curr/t_curr
            #print("AVG %3d @ %5f" % (t_curr, sum_sqr_curr))

            # good candidate?
            if find_index_of_nearest([sum_sqr_best, sum_sqr_curr], FREQI) == 1:
                #print("comparing: %f(best) %f(curr)" % (numpy.abs(sum_sqr_best - FREQI), numpy.abs(sum_sqr_curr - FREQI)))
                #print("SELECTED")
                sum_sqr_best = sum_sqr_curr
                t_best = t_curr
                t_max = t_curr*3 if (t_curr*3 < t_max) else t_max # would have used *2 but I want to be safe

        # E
        if sum_sqr_curr == 1:
            return t_curr

    return t_best

def decrypt_vigenere_cipher(c, k):
    # decypt
    t_step = 0
    message = ''
    k_index = 0
    for char in c:
        # Leave non-alphabets as-is
        if ord(char) < 97 or ord(char) > 122:
            cc = char
        # Shift alphabets
        else: 
            cc = chr(((ord(char) - 97 - (k[t_step])) % 26) + 97)
            t_step = (t_step + 1) % len(k)
        message += cc

    return message

def break_vigenere_cipher(c):
    c_stripped = get_alpha_only(c)

    try:
        t = int(input("Input key length: "))
    except:
        t = -1
    # t not found. brute force finding k length
    if t < 1:
        t = find_key_length(c_stripped)
        print("Most likely key length is %d" % (t))

    # get values of k
    k_arr = [0] * t
    for t_curr in range(t):
        c_seg = segment_string(c_stripped, t_curr, t)
        k_arr[t_curr] = get_k(find_letter_freq(c_seg))

    if len(k_arr) != t:
        print("Hey! You're not supposed to be here!")
        exit()

    # decrypt
    message = decrypt_vigenere_cipher(c, k_arr)

    k = ''
    for elem in k_arr:
        k += chr(elem+97)

    print("Message decrypted with key: %s" % k)

    return message

# decypt shift encrypted msg
def decrypt_shift_cipher(c, k):
    message = ''
    for char in c:
        # Leave non-alphabets as-is
        if ord(char) < 97 or ord(char) > 122:
            cc = char
        # Shift alphabets
        else: 
            cc = chr(((ord(char) - 97 - k) % 26) + 97)
        
        message += cc

    print("Message decrypted with key: %s" % chr(k+97))

    return message

# shift cipher
def break_shift_cipher(c):
    # get k using ciphered msg letter frequency
    k = get_k(find_letter_freq(get_alpha_only(c)))

    return decrypt_shift_cipher(c,k)

def main():
    # Open and read the encrypted file
    c = ''
    if len(sys.argv) == 2:
        try:
            with open(sys.argv[1], 'r') as f:
                c = f.read().lower()
            print("File %s openened. " % (sys.argv[1]))
        except:
            c = input("Ecrypted file not found. Enter the ciphered message: ").lower()
    else:
        c = input("Enter the ciphered message: ").lower()

    # Select cipher.
    cipher = ''
    while cipher != "shift" and cipher != "vigenere":
        cipher = input("Cipher (SELECT: shift/vigenere): ")
        if cipher == '':
            print("None selected. Defaulting to vigenere")
            cipher = "vigenere"

    # Do cipher
    message = ''
    if cipher == "shift":
        message = break_shift_cipher(c)
    elif cipher == "vigenere":
        message = break_vigenere_cipher(c)
    else:
        print("Selected cipher not found.")

    # Decipher, save to a file, and then read it
    if message != '':
        oFileName = "./deciphered.txt"
        oFile = open(oFileName, 'w')
        oFile.write(message)
        oFile.close()
        print("\nDeciphered text written in %s:\n" % oFileName)
        with open(oFileName, 'r') as f:
            print(f.read());

main()
