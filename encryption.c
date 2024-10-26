/*
 ============================================================================
 Name        : encryption.c
 Author      : Polytech
 Version     :
 Copyright   : Your copyright notice
 Description :
 ============================================================================
 */

#include "encryption.h"

uint8_t encrypt_message(const uint8_t *key, uint8_t key_size, uint8_t *msg, uint8_t msg_size)
{
    uint8_t ret = 0;

  /* ... Complété ... */

    // Ajout de la logique de chiffrement
    for(uint8_t i = 0; i < msg_size; i++)
    {
        msg[i] ^= key[i % key_size];
    }

    ret = 1; // Indique que le chiffrement s'est effectué avec succès

    return ret;
  /* ............ */
}

uint8_t decrypt_message(const uint8_t *key, uint8_t key_size, uint8_t *msg, uint8_t msg_size)
{
    // Le chiffrement étant symétrique, le déchiffrement est identique
    return encrypt_message(key, key_size, msg, msg_size);
}
