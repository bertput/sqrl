#ifndef CLIENT_H
#define CLIENT_H

#include "sqrl_client.h"


int client_authenticate(char *in_url);

bool client_onAuthenticationRequired(
    Sqrl_Transaction transaction,
    Sqrl_Credential_Type credentialType );

int client_onProgress( Sqrl_Transaction transaction, int p );

void client_onTransactionComplete( Sqrl_Transaction transaction );

void client_onSaveSuggested( Sqrl_User user );

Sqrl_User client_onSelectUser (Sqrl_Transaction transaction);

void client_onSend(
    Sqrl_Transaction transaction,
    const char *url, size_t url_len,
    const char *payload, size_t payload_len );



#endif
