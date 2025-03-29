# These functions require the following non-stdlib packages: cryptography, requests
# Note: You might also want to use functions from the hashlib package and the pow() function

import hashlib
import secrets
from typing import Dict, Optional, Tuple

import requests

from cryptography.hazmat.primitives.asymmetric import (
    padding as padding_asymmetric,
    rsa,
    utils,
)
from cryptography.hazmat.primitives.ciphers import modes, algorithms, Cipher
from cryptography.hazmat.primitives import (
    padding as padding_symmetric,
    hashes,
    serialization,
)
from cryptography.exceptions import InvalidSignature


def aes_encrypt_cbc_pkcs7(key: bytes, plaintext: bytes) -> bytes:
    """
    Encrypt `data` using AES-CBC with PKCS7 padding.

    :param key: AES symmetric key (128, 192 or 256 bits long)
    :param plaintext: the plaintext to encrypt

    :return: the ciphertext (with IV prepended)

    Example:

    >>> import secrets
    >>> key = secrets.token_bytes(32)
    >>> ciphertext = aes_encrypt_cbc_pkcs7(key=key, plaintext=b"some data to encrypt")
    """
    padder = padding_symmetric.PKCS7(algorithms.AES.block_size).padder()
    padded = padder.update(plaintext) + padder.finalize()
    iv = secrets.token_bytes(algorithms.AES.block_size // 8)

    cipher = Cipher(algorithms.AES(key), modes.CBC(iv))
    encryptor = cipher.encryptor()
    ciphertext = encryptor.update(padded) + encryptor.finalize()
    return iv + ciphertext


def aes_decrypt_cbc_pkcs7(key: bytes, ciphertext: bytes) -> bytes:
    """
    Decrypt `data` using AES-CBC with PKCS7 padding.

    :param key: AES symmetric key (128, 192 or 256 bits long)
    :param ciphertext: the cipertext to decrypt

    :return: the plaintext

    Example:

    >>> import secrets
    >>> key = secrets.token_bytes(32)
    >>> msg = b"some data to encrypt"
    >>> ciphertext = aes_encrypt_cbc_pkcs7(key=key, plaintext=msg)
    >>> assert msg == aes_decrypt_cbc_pkcs7(key=key, ciphertext=ciphertext)
    """
    iv = ciphertext[: algorithms.AES.block_size // 8]
    ciphertext = ciphertext[algorithms.AES.block_size // 8 :]

    cipher = Cipher(algorithms.AES(key), modes.CBC(iv))
    decryptor = cipher.decryptor()
    plaintext = decryptor.update(ciphertext)
    unpadder = padding_symmetric.PKCS7(algorithms.AES.block_size).unpadder()

    return unpadder.update(plaintext) + unpadder.finalize()


def mac_create_aes_cbc(key: bytes, data: bytes) -> bytes:
    """
    Calculate Message Authentication Code of `data` (using AES-CBC), i.e.
    encrypt `data` using `key` and AES-CBC with initialization vector
    equal to zero bytes.

    :param key: AES symmetric key
    :param data: the data that will be MAC'ed

    :return: 16 bytes long MAC value

    Example:

    >>> import secrets
    >>> key = secrets.token_bytes(32)
    >>> mac = mac_create_aes_cbc(key=key, data=b"some data to MAC")
    """
    padder = padding_symmetric.PKCS7(128).padder()
    padded_data = padder.update(data) + padder.finalize()

    # the initialization vector is fixed to zero bytes
    iv = b"\x00" * 16
    cipher = Cipher(algorithms.AES(key=key), mode=modes.CBC(iv))

    encryptor = cipher.encryptor()
    ciphertext = encryptor.update(padded_data) + encryptor.finalize()

    # MAC is only the last block, i.e. 16 bytes
    mac = ciphertext[-16:]
    return mac


def mac_verify_aes_cbc(key: bytes, data: bytes, mac: bytes) -> bool:
    """
    Verify that the MAC (using AES-CBC) of `data` matches `mac`.

    :param key: AES symmetric key
    :param data: the data that will be MAC'ed
    :param mac: the value against which we verify

    :return: True if the verification succeeds, False otherwise

    Example:

    >>> import secrets
    >>> key = secrets.token_bytes(32)
    >>> data = b"some data to MAC"
    >>> mac = mac_create_aes_cbc(key=key, data=data)
    >>> assert mac_verify_aes_cbc(key=key, data=data, mac=mac)
    """
    return mac == mac_create_aes_cbc(key=key, data=data)


def rsa_sign_pss_sha256(private_key: rsa.RSAPrivateKey, data: bytes) -> bytes:
    """
    Use RSA `private_key` to digitally sign of `data`.

    :param private_key: RSA private key
    :param data: the data to be signed

    :return: the bytes of the signature

    Example:

    >>> from cryptography.hazmat.primitives.asymmetric import rsa
    >>> private_key = rsa.generate_private_key(public_exponent=65537, key_size=2048)
    >>> public_key = private_key.public_key()
    >>> signature = rsa_sign_pss_sha256(private_key=private_key, data=b"a contract contents")
    """
    prehashed_msg = hashlib.sha256(data).digest()
    signature = private_key.sign(
        prehashed_msg,
        padding_asymmetric.PSS(
            mgf=padding_asymmetric.MGF1(hashes.SHA256()),
            salt_length=padding_asymmetric.PSS.MAX_LENGTH,
        ),
        utils.Prehashed(hashes.SHA256()),
    )
    return signature


def rsa_verify_pss_sha256(public_key: rsa.RSAPublicKey, data: bytes, signature: bytes) -> bool:
    """
    Verify that the `signature` of `data` was signed using the RSAPrivateKey
    corresponding to `public_key`.

    :param public_key: the RSAPublicKey to be used for the verification
    :param data: the data that were signed
    :param signature: the bytes of the signature

    Example:

    >>> from cryptography.hazmat.primitives.asymmetric import rsa
    >>> private_key = rsa.generate_private_key(public_exponent=65537, key_size=2048)
    >>> public_key = private_key.public_key()
    >>> data = b"the contract contents"
    >>> signature = rsa_sign_pss_sha256(private_key=private_key, data=data)
    >>> assert rsa_verify_pss_sha256(public_key=public_key, data=data, signature=signature)
    """
    prehashed_msg = hashlib.sha256(data).digest()
    try:
        public_key.verify(
            signature,
            prehashed_msg,
            padding_asymmetric.PSS(
                mgf=padding_asymmetric.MGF1(hashes.SHA256()),
                salt_length=padding_asymmetric.PSS.MAX_LENGTH,
            ),
            utils.Prehashed(hashes.SHA256()),
        )
        return True
    except InvalidSignature:
        return False


def rsa_load_key(pem: bytes) -> rsa.RSAPrivateKey:
    """
    Load an RSA private_key in PEM format.

    :param pem: the key in PEM format, including the b"-----BEGIN RSA PRIVATE KEY-----" and
                b"-----END RSA PRIVATE KEY-----" markers
    :return: the loaded private key
    """
    return serialization.load_pem_private_key(pem, None)


def send_get(endpoint: str, params: Optional[Dict[str, str]] = None) -> Tuple[int, Dict[str, str]]:
    """
    Send a GET request to the `endpoint` with query `params`.

    :param endpoint: the endpoint url
    :param params: the query parameters to include, can be `None`

    :return: a tuple of the HTTP status code of the response and the JSON response as a dictionary

    Example:

    >>> status_code, data = send_get("/hw03/auth-server/api/dh")
    """
    resp = requests.get("https://pv080.fi.muni.cz" + endpoint, params=params)
    data = resp.json()
    if resp.status_code >= 500:
        print("This should not be usually happening, please report it.")
    return resp.status_code, data


def send_post(endpoint: str, data: Optional[Dict[str, str]] = None) -> Tuple[int, Dict[str, str]]:
    """
    Send a POST request to the `endpoint` with JSON `data`.

    :param endpoint: the endpoint url
    :param data: the JSON to include in the body

    :return: a tuple of the HTTP status code of the response and the JSON response as a dictionary

    Example:

    >>> status_code, data = send_post("/hw03/auth-server/api/kex", data={"payload": "12", "signature": "34", "uco": 123})
    """
    resp = requests.post("https://pv080.fi.muni.cz" + endpoint, json=data)
    data = resp.json()
    if resp.status_code >= 500:
        print("This should not be usually happening, please report it.")
    return resp.status_code, data


def int_to_bytes(integer: int) -> bytes:
    return integer.to_bytes((integer.bit_length() + 7) // 8, byteorder="big")


def bytes_to_int(data: bytes) -> int:
    return int.from_bytes(data, byteorder="big")


def int_to_hex(integer: int) -> str:
    return int_to_bytes(integer).hex()


def hex_to_int(hexa: str) -> int:
    return bytes_to_int(bytes.fromhex(hexa))
