from ecpy.curves import Curve, Point
from Crypto.Hash import SHA3_256, HMAC, SHA256
from Crypto.Cipher import AES
import requests
from random import randint
from helpers import sign_msg, decrypt, decryptAndAuth_p2

API_URL = 'http://cryptlygos.pythonanywhere.com'

stuID = 25060  # my id

# HERE CREATE A LONG TERM KEY
E = Curve.get_curve('secp256k1')
n = E.order
p = E.field
P = E.generator
a = E.a
b = E.b
# long term key, same rand key i used for phase 1
priv_key = 25599029407980303215847102866217791268869757416928311560876676855629646051246

# priv key was a rand num between 0 and n-1,
QCli_long = priv_key * P  # long term public key, same as phase 1

QSer_long = Point(0xc1bc6c9063b6985fe4b93be9b8f9d9149c353ae83c34a434ac91c85f61ddd1e9,
                  0x931bd623cf52ee6009ed3f50f6b4f92c564431306d284be7e97af8e443e69a8c, E)

eph_keys = [20502355107986444887476443464543453221255125678997613825737115758616498806643,
            84968644559653955463335091424109285287666967333991804418582142676218879251881,
            62012633539336500137722306692622068122277275906143647080895042886089372362288,
            24140478941403166752441306207106186271926975308850540000627126198184401711752,
            56726992000978288915550946137454107800231904171103748575597502190614444102832,
            110897573391634113116342703108634746458570553556983554235569136651256398394147,
            13665392528372869221699232727588041987207605630455785287943430960638415866488,
            106642390332532476433271258962819794256624289694967231292093595843203135142019,
            82518801677889721985024847781184259808075811517431754345404062704248236807054,
            2410627964221980623784412428665070135003305965226249436711335332553508955473]  # my current eph keys.

# register the long term keys again if needed. Uncomment for registiration
"""
h,s = sign_msg(stuID,priv_key,E)
####Register Long Term Key
mes = {'ID':25060, 'H': h, 'S': s, 'LKEY.X': QCli_long.x, 'LKEY.Y': QCli_long.y}
response = requests.put('{}/{}'.format(API_URL, "RegLongRqst"), json = mes)
print(response.json())
code = input()

mes = {'ID':25060, 'CODE': code}
response = requests.put('{}/{}'.format(API_URL, "RegLong"), json = mes)
print(response.json())

"""

# Check Status
h, s = sign_msg(stuID, priv_key, E)
mes = {'ID_A': 25060, 'H': h, 'S': s}
response = requests.get('{}/{}'.format(API_URL, "Status"), json=mes)
print(response.json())

rem_key = int(response.json().split(" ")[2])  # number of remaining keys

## Get your message, check messages
flag = False
while not flag:
    h, s = sign_msg(stuID, priv_key, E)
    mes = {'ID_A': stuID, 'S': s, 'H': h}
    response = requests.get('{}/{}'.format(API_URL, "ReqMsg_PH3"), json=mes)
    print(response.json())
    try:
        print(decryptAndAuth_p2(response.json(), E, eph_keys))
    except Exception as ex:
        flag = True
        template = "An exception of type {0} occurred. Arguments:\n{1!r}"
        message = template.format(type(ex).__name__, ex.args)
        print(message)

    if rem_key == 1 and flag:
        h, s = sign_msg(stuID, priv_key, E)
        mes = {'ID': stuID, 'S': s, 'H': h}
        print(mes)
        response = requests.get('{}/{}'.format(API_URL, "RstEKey"), json=mes)
        print(response.json())
        # HERE GENERATE A EPHEMERAL KEY
        for ij in range(10):
            eph_priv_key = randint(1, n - 2)
            ekey = eph_priv_key * P
            print(eph_priv_key)
            eph_keys[ij] = eph_priv_key
            h, s = sign_msg(str(ekey.x) + str(ekey.y), priv_key, E)
            mes = {'ID': stuID, 'KEYID': ij, 'QAI.X': ekey.x, 'QAI.Y': ekey.y, 'Si': s, 'Hi': h}
            response = requests.put('{}/{}'.format(API_URL, "SendKey"), json=mes)
            # print(response.json())

# message generation to student B
print("Please enter the id of the user you want to send message")
stuID_B = int(input("Enter 0 if you don't want to send a message: "))  # id of the receiver
if stuID_B == 0:
    exit()
h_B, s_B = sign_msg(stuID_B, priv_key, E)
### Get key of the Student B
mes = {'ID_A': stuID, 'ID_B': stuID_B, 'S': s_B, 'H': h_B}
response = requests.get('{}/{}'.format(API_URL, "ReqKey"), json=mes)
res = response.json()
print(res)

# msg generation
T = Point(res["QBJ.x"], res["QBJ.y"], E) * eph_keys[int(res["i"])]
U = (str(T.x) + str(T.y) + "NoNeedToRunAndHide")
# https://www.youtube.com/watch?v=W6oQUDFV2C0&ab_channel=MatthewF.

K_enc = SHA3_256.new(U.encode("utf-8"))  # enc. session key
K_mac = SHA3_256.new(K_enc.digest())  # mac key

msg_to_send = input("Enter your message: ")
msg_to_send = msg_to_send.encode("utf-8")
cipher = AES.new(K_enc.digest(), AES.MODE_CTR)
ctext = cipher.nonce + cipher.encrypt(msg_to_send)
mac = HMAC.new(K_mac.digest(), digestmod=SHA256)
mac.update(ctext[8:])
ctext = ctext + mac.digest()
ctext = int.from_bytes(ctext, byteorder="big")
# print(ctext)

### Send message to student B

mes = {'ID_A': stuID, 'ID_B': stuID_B, 'I': int(res["i"]), 'J': int(res["j"]), 'MSG': ctext}
response = requests.put('{}/{}'.format(API_URL, "SendMsg"), json=mes)
print(response.json())
