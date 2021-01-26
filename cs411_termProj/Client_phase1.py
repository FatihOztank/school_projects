import math
import timeit
import random
import sympy
import warnings
from random import randint, seed
import sys
from ecpy.curves import Curve, Point
from ecpy.keys import ECPublicKey, ECPrivateKey
from Crypto.Hash import SHA3_256
import requests
from Crypto.Cipher import AES
from Crypto import Random
from Crypto.Util.Padding import pad
from Crypto.Util.Padding import unpad
import random
import re
import json
from helpers import sign_msg,decrypt

API_URL = 'http://cryptlygos.pythonanywhere.com'

stuID = 25060

# HERE CREATE A LONG TERM KEY
E = Curve.get_curve('secp256k1')
n = E.order
p = E.field
P = E.generator
a = E.a
b = E.b
# long term key
priv_key = 25599029407980303215847102866217791268869757416928311560876676855629646051246
# priv key was a rand num between 0 and n-1
lkey = priv_key * P  # lkey = public key

#print(lkey)
# server's long term key
QSer_long = Point(0xc1bc6c9063b6985fe4b93be9b8f9d9149c353ae83c34a434ac91c85f61ddd1e9,
                  0x931bd623cf52ee6009ed3f50f6b4f92c564431306d284be7e97af8e443e69a8c, E)

# long term key done here
h,s = sign_msg(stuID,priv_key,E)  #i just implemented a function to sign a messages, thats all

# HERE GENERATE A EPHEMERAL KEY
eph_priv_key = randint(1, n - 2)
ekey = eph_priv_key * P


try:

    # REGISTRATION, done.
    mes = {'ID': stuID, 'h': h, 's': s, 'LKEY.X': lkey.x, 'LKEY.Y': lkey.y}
    print(mes)
    print("priv key = ", priv_key)
    response = requests.put('{}/{}'.format(API_URL, "RegStep1"), json=mes)
    if ((response.ok) == False): raise Exception(response.json())
    print(response.json())

    print("Enter verification code which is sent to you: ")
    code = int(input())

    mes = {'ID': stuID, 'CODE': code}
    response = requests.put('{}/{}'.format(API_URL, "RegStep3"), json=mes)
    if ((response.ok) == False): raise Exception(response.json())
    print(response.json())


    # STS PROTOCOL

    mes = {'ID': stuID, 'EKEY.X': ekey.x, 'EKEY.Y': ekey.y}
    response = requests.put('{}/{}'.format(API_URL, "STSStep1&2"), json=mes)
    if ((response.ok) == False): raise Exception(response.json())
    res = response.json()
    SKEY = Point(res["SKEY.X"],res["SKEY.Y"], E)

    # calculate T,K,U
    T = eph_priv_key*SKEY
    U = (str(T.x) + str(T.y) + "BeYourselfNoMatterWhatTheySay")
    K = SHA3_256.new(U.encode("utf-8"))  # session key, this key is used between this communication


    # Sign Message
    w1 = str(ekey.x) + str(ekey.y) + str(SKEY.x) + str(SKEY.y)  # msg contains pubkey of server & client
    s_w1, h_w1 = sign_msg(w1,priv_key,E)  # my function to sign messages
    pl_txt = ("s"+str(s_w1)+"h"+str(h_w1)).encode("utf-8")

    # Encyption
    cip = AES.new(K.digest(),AES.MODE_CTR)
    Y1 = cip.nonce + cip.encrypt(pl_txt)
    ctext = int.from_bytes(Y1,byteorder="big")

    ###Send encrypted-signed keys and retrive server's signed keys
    mes = {'ID': stuID, 'FINAL MESSAGE': ctext}
    response = requests.put('{}/{}'.format(API_URL, "STSStep4&5"), json=mes)
    if ((response.ok) == False): raise Exception(response.json())
    ctext = response.json()


    # Decrypt
    dtext = decrypt(ctext, AES.MODE_CTR, K)  # decrypt function i implemented.

    # verify
    w2 = str(SKEY.x) + str(SKEY.y) + str(ekey.x) + str(ekey.y)  # msg that server side signs
    s_w2 = int(dtext[1:dtext.index("h")])
    h_w2 = int(dtext[dtext.index("h")+1:])
    V = s_w2*P - h_w2*QSer_long
    v = V.x % n
    h_ = SHA3_256.new(w2.encode("utf-8") + v.to_bytes((v.bit_length() + 7) // 8, byteorder='big'))
    h_ = int.from_bytes(h_.digest(), byteorder='big') % n
    #print("h_ = ",h_)
    #print("h_w2 = ", h_w2)
    print("Result of the signature verification in 2.5: ", h_ == h_w2)  # if true, signature match

    # get a message from server for
    mes = {'ID': stuID}
    response = requests.get('{}/{}'.format(API_URL, "STSStep6"), json=mes)
    ctext = response.json()


    # Decrypt
    print("Decrypted text 2.6: ", decrypt(ctext,AES.MODE_CTR,K))
    w3 = decrypt(ctext,AES.MODE_CTR,K)
    msg = w3.split(".")[0]  # When you read this message I'll be far away
    rand = w3.split(".")[1]  # rand num

    # Add 1 to random to create the new message and encrypt it
    rand = int(rand) + 1
    msg = msg + ". " + str(rand)
    cip = AES.new(K.digest(), AES.MODE_CTR)
    W4 = cip.nonce + cip.encrypt(msg.encode("utf-8"))
    ct = int.from_bytes(W4, byteorder="big")

    # send the message and get response of the server
    mes = {'ID': stuID, 'ctext': ct}
    response = requests.put('{}/{}'.format(API_URL, "STSStep7&8"), json=mes)
    ctext = response.json()
    print("Decrypted text 2.7: ",decrypt(ctext,AES.MODE_CTR,K))

except Exception as e:
    print(e)
