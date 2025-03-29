from functions import (
    aes_encrypt_cbc_pkcs7, aes_decrypt_cbc_pkcs7,
    mac_create_aes_cbc, mac_verify_aes_cbc,
    rsa_sign_pss_sha256, rsa_verify_pss_sha256, rsa_load_key,
    send_get, send_post,
    int_to_bytes, bytes_to_int, int_to_hex, hex_to_int
)

import os
import random
import hashlib
from cryptography.hazmat.primitives.asymmetric import rsa
import json
from cryptography.hazmat.primitives.serialization import load_pem_private_key


UCO = 570943 

with open("private_key.txt", "r") as private_key_file:
    private_key_data = private_key_file.read()

actual_private_key = load_pem_private_key(
    private_key_data.encode() if isinstance(private_key_data, str) else private_key_data,
    password=None
)



# Function to print requests and responses for debugging
def print_step(step_name, request=None, response=None):
    print(f"\n--- {step_name} ---")
    if request:
        print(f"Request: {json.dumps(request, indent=2)}")
    if response:
        print(f"Response: {json.dumps(response, indent=2)}")



#LAME PART






# 1. Get Diffie-Hellman parameters
status, dh_params = send_get('/hw03/auth-server/api/dh')
print_step("Get DH parameters", response=dh_params)
p = hex_to_int(dh_params['modulus_p'])
g = hex_to_int(dh_params['generator'])

# 2. Get AUTH server's RSA public key
status, auth_rsa = send_get('/hw03/auth-server/api/rsa')
print_step("Get AUTH RSA key", response=auth_rsa)
auth_n = hex_to_int(auth_rsa['modulus_n'])
auth_e = hex_to_int(auth_rsa['public_exponent_e'])
auth_public_key = rsa.RSAPublicNumbers(auth_e, auth_n).public_key()

# 3. Generate DH key pair
dh_private = random.randint(2, p-2)
dh_public = pow(g, dh_private, p)
dh_public_hex = int_to_hex(dh_public)
dh_public_bytes = bytes.fromhex(dh_public_hex)

# 4. Sign public key
signature = rsa_sign_pss_sha256(actual_private_key, dh_public_bytes)
signature_hex = signature.hex()

# 5. Perform key agreement
kex_request = {
    "uco": UCO,
    "payload": dh_public_hex,
    "signature": signature_hex
}
status, kex_response = send_post('/hw03/auth-server/api/kex', kex_request)
print_step("Key exchange", kex_request, kex_response)

# 6. Verify server's signature
server_public_key_hex = kex_response['payload']
server_signature_hex = kex_response['signature']
server_public_key_bytes = bytes.fromhex(server_public_key_hex)
server_signature_bytes = bytes.fromhex(server_signature_hex)
verified = rsa_verify_pss_sha256(auth_public_key, server_public_key_bytes, server_signature_bytes)
if not verified:
    print("Server signature verification failed!")
    exit(1)
else:
    print("Server signature verified successfully.")

# 7. Calculate the shared key
server_dh_public = hex_to_int(server_public_key_hex)
dh_shared_secret = pow(server_dh_public, dh_private, p)
dh_shared_secret_bytes = int_to_bytes(dh_shared_secret).rjust(256, b'\x00')
shared_key = hashlib.sha256(dh_shared_secret_bytes).digest()
print(f"Shared key established: {shared_key.hex()[:10]}...")







# 1. Get DATABASE server's RSA public key
status, db_rsa = send_get('/hw03/database-server/api/rsa')
print_step("Get DATABASE RSA key", response=db_rsa)

# 2. Generate a random MAC key
mac_key = os.urandom(32)  # Generate a random 32-byte key
print(f"Generated MAC key: {mac_key.hex()[:10]}...")

# 3. Encrypt the MAC key
encrypted_mac_key = aes_encrypt_cbc_pkcs7(shared_key, mac_key)

# 4. Sign the encrypted MAC key
mac_key_signature = rsa_sign_pss_sha256(actual_private_key, encrypted_mac_key)

# 5. Establish the MAC key
mac_request = {
    "uco": UCO,
    "payload": encrypted_mac_key.hex(),
    "signature": mac_key_signature.hex()
}
status, mac_response = send_post('/hw03/database-server/api/mac', mac_request)
print_step("Establish MAC key", mac_request, mac_response)





#COOL PART



# 1. Create the message
message = f"Give {UCO} 3 p".encode()
print(f"Original message: {message.decode()}")

# 2. Encrypt the message
encrypted_message = aes_encrypt_cbc_pkcs7(shared_key, message)

# 3. Sign the encrypted message
message_signature = rsa_sign_pss_sha256(actual_private_key, encrypted_message)

# 4. Calculate the MAC
uco_bytes = int_to_bytes(UCO)
uco_mac = mac_create_aes_cbc(mac_key, uco_bytes)

# 5. Send the request
points_request = {
    "uco": UCO,
    "payload": encrypted_message.hex(),
    "signature": message_signature.hex(),
    "mac": uco_mac.hex()
}
status, points_response = send_post('/hw03/database-server/api/points', points_request)
print_step("Give points (first part)", points_request, points_response)

# Check your points
status, points = send_get('/hw03/database-server/api/points', {"uco": UCO})
print(f"Your current points: {points['points']}")






#HACKERMAN (COOL)

print("\ncool stuff ...\n")

# The leaked information
leaked_encrypted_msg = bytes.fromhex("116911af5df639774a8cc0badff7068a2d0cdfee53dee0593aed8b08854b48b2")
original_msg = b"Give 445358 3 p"
vashek_uco = 344
vashek_uco_mac = bytes.fromhex("5c7adf789b5b093889e7cda64a091bc0")



#identify which block contains the UCO
block_size = 16
num_blocks = len(leaked_encrypted_msg) // block_size




# Convert leaked message to blocks
blocks = [leaked_encrypted_msg[i:i+block_size] for i in range(0, len(leaked_encrypted_msg), block_size)]




# Make a copy of the blocks
modified_blocks = blocks.copy()



modified_blocks[0] = bytes([
    blocks[0][i] ^ original_msg[i] ^ ord(f"Give {UCO} 3 p"[i]) 
    for i in range(len(original_msg))
])



# Reassemble modified ciphertext
modified_encrypted_msg = b''.join(modified_blocks)


fake_message = f"Give {UCO} 3 p".encode()

payload_fake = aes_encrypt_cbc_pkcs7(shared_key, fake_message)


# Prepare the exploit request
exploit_request = {
    "mac": vashek_uco_mac.hex(),
    "payload": payload_fake.hex(),
    "signature": message_signature.hex(),
    "uco": vashek_uco,  
    
}




#In this world, is the destiny of mankind controlled by some transcendental entity or law? 
# Is it like the hand of God hovering above? At least it is true that man has no control; even over his own will



# Send the exploit request (POST request)
status, response = send_post('/hw03/database-server/api/points', exploit_request)

# Print the response to see if the points were updated
print("Exploit Response:", response)



# Check your points
status, points = send_get('/hw03/database-server/api/points', {"uco": UCO})
print(f"Your current points: {points['points']}")





#im losing my sanity



#I drink 'til I'm drunk,
# Smoke 'til I'm high 
# Castle on the hill, wake up in the sky
# You can't tell me I ain't fly
# I know I'm super fly,
# I know I'm super fly