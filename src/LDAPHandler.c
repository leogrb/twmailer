#include "../include/LDAPHandler.h"

bool userLogin(char *username, char *password)
{
    LDAP *ld;                /* LDAP resource handle */
    LDAPMessage *result, *e; /* LDAP result handle */
    BerElement *ber;         /* array of attributes */
    char *attribute;
    BerValue **vals;

    BerValue *servercredp;
    BerValue cred;
    cred.bv_val = password;
    cred.bv_len = strlen(password);

    int i, rc = 0;

    char *userDN = NULL;
    char *userBind = "";
    char *filter = "";
    sprintf(userBind, "uid=%s,ou=People,dc=technikum-wien,dc=at", username);
    sprintf(filter, "(uid=%s)", username);

    const char *attribs[] = {"uid", "cn", NULL}; /* attribute array for search */

    int ldapversion = LDAP_VERSION3;

    /* setup LDAP connection */
    if (ldap_initialize(&ld, LDAP_URI) != LDAP_SUCCESS)
    {
        fprintf(stderr, "ldap_init failed");
        return false;
    }

    printf("connected to LDAP server %s\n", LDAP_URI);

    if ((rc = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ldapversion)) != LDAP_SUCCESS)
    {
        fprintf(stderr, "ldap_set_option(PROTOCOL_VERSION): %s\n", ldap_err2string(rc));
        ldap_unbind_ext_s(ld, NULL, NULL);
        return false;
    }

    if ((rc = ldap_start_tls_s(ld, NULL, NULL)) != LDAP_SUCCESS)
    {
        fprintf(stderr, "ldap_start_tls_s(): %s\n", ldap_err2string(rc));
        ldap_unbind_ext_s(ld, NULL, NULL);
        return false;
    }

    /* anonymous bind */
    // TODO instead of user bind, use anonym from #define
    rc = ldap_sasl_bind_s(ld, userBind, LDAP_SASL_SIMPLE, &cred, NULL, NULL, &servercredp);
    if (rc != LDAP_SUCCESS)
    {
        fprintf(stderr, "LDAP bind error: %s\n", ldap_err2string(rc));
        ldap_unbind_ext_s(ld, NULL, NULL);
        return false;
    }
    else
    {
        printf("Bind was successful\n");
    }

    rc = ldap_search_ext_s(ld, SEARCHBASE, SCOPE, filter, (char **)attribs, 0, NULL, NULL, NULL, 500, &result);

    if (rc != LDAP_SUCCESS)
    {
        fprintf(stderr, "LDAP search error: %s\n", ldap_err2string(rc));
        ldap_unbind_ext_s(ld, NULL, NULL);
        return false;
    }

    // int totalResult = ldap_count_entries(ld, result);

    for (e = ldap_first_entry(ld, result); e != NULL; e = ldap_next_entry(ld, e))
    {
        for (attribute = ldap_first_attribute(ld, e, &ber); attribute != NULL; attribute = ldap_next_attribute(ld, e, ber))
        {
            if ((vals = ldap_get_values_len(ld, e, attribute)) != NULL)
            {
                for (i = 0; i < ldap_count_values_len(vals); i++)
                {
                    if (strcmp(attribute, "uid") == 0 && strcmp(vals[i]->bv_val, username))
                    {
                        userDN = ldap_get_dn(ld, e);
                    }
                }
                ldap_value_free_len(vals);
            }
            /* free memory used to store the attribute */
            ldap_memfree(attribute);
        }
        /* free memory used to store the value structure */
        if (ber != NULL)
        {
            ber_free(ber, 0);
        }
        printf("\n");
    }
    /* free memory used for result */
    ldap_msgfree(result);

    if (userDN != NULL)
    {
        rc = ldap_sasl_bind_s(ld, userDN, LDAP_SASL_SIMPLE, &cred, NULL, NULL, &servercredp);
        if (rc != LDAP_SUCCESS)
        {
            fprintf(stderr, "LDAP bind error: %s\n", ldap_err2string(rc));
            ldap_unbind_ext_s(ld, NULL, NULL);
            return false;
        }
        printf("Login was successful\n");
        ldap_unbind_ext_s(ld, NULL, NULL);
        return true;
    }

    ldap_unbind_ext_s(ld, NULL, NULL);
    return false;
}