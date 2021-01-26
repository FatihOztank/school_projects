import random
from Crypto.Hash import SHA3_256,HMAC, SHA256
from ecpy.curves import Curve, Point
from Crypto.Cipher import AES
import hmac
import hashlib

def sign_msg(msg,priv_key,curve):
    # signs the msg with given curve and priv key
    n = curve.order
    P = curve.generator
    k = random.randint(1, n - 2)
    R = k * P
    r = R.x % n

    h = SHA3_256.new(str(msg).encode("utf-8") + r.to_bytes((r.bit_length() + 7) // 8, byteorder='big'))
    h = (int.from_bytes(h.digest(), byteorder='big')) % n

    s = (priv_key * h + k) % n

    return h,s

# decrypts the number encoded ciphertext with given aes mode and given key = k
def decrypt(ctxt, aes_mode,k,ret_with_dec = True):
    ctext = ctxt.to_bytes((ctxt.bit_length() + 7) // 8, byteorder='big')
    cipher = AES.new(k.digest(), aes_mode, nonce=ctext[0:8])
    dtext = cipher.decrypt(ctext[8:])
    if ret_with_dec:  # returns the dtext as string by decoding dtext w.r.t utf-8
        return dtext.decode('utf-8')
    else:  # returns dtext as byte array
        return dtext


def decryptAndAuth_p2(msg,curve,eph_keys):  # msg = msg["MSG"]
    # eph_keys is the list of all ephemeral keys.
    T = Point(msg["QBJ.X"], msg["QBJ.Y"], curve) * eph_keys[int(msg["KEYID"])]
    U = (str(T.x) + str(T.y) + "NoNeedToRunAndHide")
    # https://www.youtube.com/watch?v=W6oQUDFV2C0&ab_channel=MatthewF.

    K_enc = SHA3_256.new(U.encode("utf-8"))  # enc. session key
    K_mac = SHA3_256.new(K_enc.digest())  # mac key
    # print(K_enc.digest())
    # print(K_mac.digest())

    # message as byte array
    byteMsg = msg["MSG"].to_bytes((msg["MSG"].bit_length() + 7) // 8, byteorder='big')

    # mac is the last 32 bytes of the message
    big_mac = byteMsg[len(byteMsg) - 32:]  # yes, big_mac is mac of the message.
    cip = int.from_bytes(byteMsg[:len(byteMsg) - 32], byteorder='big') # cip is the ciphertext in number format

    dec_arr = decrypt(cip, AES.MODE_CTR, K_enc)
    #print(dec_arr)
    msg = byteMsg[8:len(byteMsg) - 32]  # first 8 byte is nonce
    h = HMAC.new(K_mac.digest(), digestmod=SHA256)
    h.update(msg)
    try:
        # return the decrypted string if msg is authentic
        h.verify(big_mac)
        print("The message '%s' is authentic" % msg)
        return dec_arr
    except ValueError:
        # return none if auth. fails
        print("The message or the key is wrong")
