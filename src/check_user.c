/*
  This program was contributed by Shane Watts
  [modifications by AGM and kukuk]

  You need to add the following (or equivalent) to the
  /etc/pam.d/check_user file:
  # check authorization
  auth       required     pam_unix.so
  account    required     pam_unix.so
 */

#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <stdio.h>
#include <assert.h>


int test_conv(int num_msg, const struct pam_message **msg,
        struct pam_response **resp, void *appdata_ptr){
    const struct pam_message* msg_ptr = *msg;
    struct pam_response * resp_ptr = NULL;
    int x = 0;
    *resp = calloc(sizeof(struct pam_response), num_msg);
    for (x = 0; x < num_msg; x++, msg_ptr++){
        char* resp_str;
        switch (msg_ptr->msg_style){
            case PAM_PROMPT_ECHO_OFF:
            case PAM_PROMPT_ECHO_ON:
                resp_str = getpass(msg_ptr->msg);
                resp[x]->resp= strdup(resp_str);
                break;

            case PAM_ERROR_MSG:
            case PAM_TEXT_INFO:
                printf("PAM: %s\n", msg_ptr->msg);
                break;

            default:
                assert(0);

        }
    }
    return PAM_SUCCESS;
}

static struct pam_conv conv = {
    test_conv,
    NULL
};

int main(int argc, char *argv[])
{
    pam_handle_t *pamh=NULL;
    int retval;
    const char *user="nobody";

    if(argc == 2) {
        user = argv[1];
    }

    if(argc > 2) {
        fprintf(stderr, "Usage: check_user [username]\n");
        exit(1);
    }

    retval = pam_start("check_user", user, &conv, &pamh);

    if (retval == PAM_SUCCESS)
        retval = pam_authenticate(pamh, 0);    /* is user really user? */

    if (retval == PAM_SUCCESS)
        retval = pam_acct_mgmt(pamh, 0);       /* permitted access? */

    /* This is where we have been authorized or not. */

    if (retval == PAM_SUCCESS) {
        fprintf(stdout, "Authenticated\n");
    } else {
        fprintf(stdout, "Not Authenticated\n");
    }

    if (pam_end(pamh,retval) != PAM_SUCCESS) {     /* close Linux-PAM */
        pamh = NULL;
        fprintf(stderr, "check_user: failed to release authenticator\n");
        exit(1);
    }

    return ( retval == PAM_SUCCESS ? 0:1 );       /* indicate success */
}
