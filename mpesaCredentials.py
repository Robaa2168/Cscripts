import base64
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import rsa, padding
from cryptography import x509


def load_public_key_from_certificate(cert_path):
    """Loading a public key from a given certificate."""
    with open(cert_path, 'rb') as cert_file:
        cert = x509.load_pem_x509_certificate(cert_file.read(), default_backend())
        return cert.public_key()


def encrypt_with_mpesa_public_key(initiator_password, public_key_path):
    # Check if it's a certificate and load public key accordingly
    if public_key_path.endswith('.cer'):
        public_key = load_public_key_from_certificate(public_key_path)
    else:
        with open(public_key_path, 'rb') as key_file:
            public_key = serialization.load_pem_public_key(
                key_file.read(),
                backend=default_backend()
            )
    # Encrypting the initiator password using RSA with PKCS #1.5 padding
    encrypted_password = public_key.encrypt(
        initiator_password.encode(),
        padding.PKCS1v15()
    )
    # Converting the encrypted byte array into a base64 encoded string
    security_credential = base64.b64encode(encrypted_password).decode()

    return security_credential


if __name__ == "__main__":
    #actual initiator password and public key path
    initiator_password = "Lahaja2171#"
    public_key_path = r"C:\Users\Koech\Downloads\ProductionCertificate.cer"

    security_credential = encrypt_with_mpesa_public_key(initiator_password, public_key_path)
    print("Security Credential:", security_credential)
