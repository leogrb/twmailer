#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ldap.h>

#define LDAP_URI "ldap://ldap.technikum-wien.at:389"
#define SEARCHBASE "dc=technikum-wien,dc=at"
#define SCOPE LDAP_SCOPE_SUBTREE
#define BIND_USER "uid=,ou=People,dc=technikum-wien,dc=at" /* anonymous bind with user and pw empty */
#define BIND_PW ""

bool userLogin(char *username, char *password);