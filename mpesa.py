import base64
from Crypto.PublicKey import RSA
from Crypto.Cipher import PKCS1_v1_5
from Crypto.Util.Padding import pad
from Crypto.Hash import SHA
import os

def encrypt_with_public_key(password, public_key_path):
    # Read the public key from the certificate file
    with open(public_key_path, 'r') as file:
        public_key = RSA.import_key(file.read())

    # Encrypt the password using RSA with PKCS#1.5 padding
    cipher = PKCS1_v1_5.new(public_key)
    encrypted_password = cipher.encrypt(password.encode())

    # Encode the encrypted data to base64
    base64_encoded = base64.b64encode(encrypted_password).decode()
    return base64_encoded

# Path to the public key certificate
public_key_path = r"C:\Users\Koech\Downloads\ProductionCertificate.cer"

# Unencrypted password
password = "Pahaja40@"

# Generate the security credentials
security_credentials = encrypt_with_public_key(password, public_key_path)
print("Security Credentials:", security_credentials)
