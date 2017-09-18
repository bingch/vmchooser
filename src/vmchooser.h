
#include <lber.h>
#include <ldap.h>

/* ldap parameters */
const gchar *ldap_uri, *bind_dn, *bind_pass, *base, *filter_pre;
/* local user credential */
const gchar *localuser, *localuser_hash;

/* widget handler */
GtkWidget *wgWarn, *wgBanner, *wgFootnote,*wgUsername,
		  *wgID, *wgPass, *wgChooser, *btLogin;

static int load_conf(const char *conf_file)
{
	GKeyFile *keyfile;
	GKeyFileFlags flags;
	GError *error = NULL;
	
	keyfile = g_key_file_new ();
	flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;
	if (!g_key_file_load_from_file (keyfile, conf_file, flags, &error))
	{
		g_error (G_STRINGIFY_ARG(error->message));
		return -1;
	}
	/* construct vm chooser combobox */
	const gchar *vms = g_key_file_get_string(keyfile, "vm", "VMs", NULL);
	gchar **cblist = g_strsplit(vms,":",-1);
	int columns_num =  g_strv_length(cblist);
	int i;
	for( i=0; i< columns_num ; i++ ){
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(wgChooser),cblist[i]);
	}
	
	gtk_combo_box_set_active(GTK_COMBO_BOX(wgChooser),0);
	g_strfreev(cblist);
	/* set banner, username, footnote */ 
	const char *banner = g_key_file_get_string(keyfile,
	                                           "misc", "BANNER", NULL);
	gtk_label_set_label(GTK_LABEL(wgBanner), banner);
	const char *username = g_key_file_get_string(keyfile,
	                                           "misc", "USERNAME", NULL);
	gtk_label_set_label(GTK_LABEL(wgUsername), username);
	const char *footnote = g_key_file_get_string(keyfile,
	                                             "misc", "FOOTNOTE", NULL);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW
	                                                 (wgFootnote));
	gtk_text_buffer_set_text(buffer,footnote, -1);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(wgFootnote), buffer);
	/* ldap parameters */
	ldap_uri = g_key_file_get_string(keyfile, "ldap", "LDAP_URI", NULL);
	bind_dn = g_key_file_get_string(keyfile, "ldap", "BIND_DN", NULL);
	bind_pass = g_key_file_get_string(keyfile, "ldap", "BIND_PASS", NULL);
	base = g_key_file_get_string(keyfile, "ldap", "BASE", NULL);
	filter_pre = g_key_file_get_string(keyfile, "ldap", "FILTER_PRE", NULL);
	/* local user credential */
	localuser = g_key_file_get_string(keyfile, "local user", "NAME", NULL);
	localuser_hash = g_key_file_get_string(keyfile, "local user", "HASH", NULL);

	g_key_file_free (keyfile);
	return 0;
}


static void unbind (LDAP *ld)
{
	int ret = ldap_unbind_ext(ld, NULL, NULL);
	if (ret != LDAP_SUCCESS){
		g_error("ldap_unbind_s error: %s\n",ldap_err2string(ret));
	} /*else {
		g_message("ldap_unbind_s OK");
	}*/
}

static gboolean check_ret (int ret, int criterion, const gchar* errmsg)
{
	if (ret == criterion){
		return TRUE;
	} else {
		g_warning ( "%s failed: %s", errmsg, ldap_err2string(ret));
		return FALSE;
	}
}

static gboolean ldap_check_pass (const gchar* id, const gchar* pass)
{
	int ret;
	LDAP *ld;
	LDAPMessage* msg = NULL;
	LDAPMessage* entry = NULL;
	gchar* dn = NULL;
	gchar* filter = NULL;
	gboolean result = FALSE;
	/* FIXME hard-coded ldap opts */ 
	int desired_version = LDAP_VERSION3;
	int reqcert = LDAP_OPT_X_TLS_DEMAND;
	BerValue cred;
	
	ret = ldap_set_option(NULL, LDAP_OPT_X_TLS_REQUIRE_CERT, &reqcert);
	if (! check_ret(ret, LDAP_OPT_SUCCESS, 
	                "ldap_set_option cert" )) goto cleanup;
	ret = ldap_initialize(&ld, ldap_uri);
	if (! check_ret(ret, LDAP_SUCCESS, "ldap_initialize")) goto cleanup;

	//set the LDAP version to be 3
	ret = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &desired_version);
	if (! check_ret(ret, LDAP_OPT_SUCCESS, 
	                "ldap_set_option version")) goto cleanup;

	cred.bv_len = strlen(bind_pass);
    cred.bv_val = strdup(bind_pass);
	ret = ldap_sasl_bind_s( ld, bind_dn, LDAP_SASL_SIMPLE, &cred,
	                       NULL, NULL, NULL );
	cred.bv_len = 0;
	cred.bv_val = NULL;
	if (! check_ret(ret, LDAP_SUCCESS, "ldap_sasl_bind_s: search")) goto cleanup;
	filter = g_strjoin("=", filter_pre, id, NULL);
	ret = ldap_search_ext_s(ld, base, LDAP_SCOPE_SUBTREE, filter, NULL, 0,
	                        NULL, NULL, NULL, 0, &msg);
	g_free(filter);
	if (! check_ret(ret, LDAP_SUCCESS, "ldap_search_s")) goto cleanup;
	ret = ldap_count_entries(ld, msg);
	if (!ret == 1) goto cleanup;
	/* Iterate through the returned entries */
	for (entry = ldap_first_entry(ld, msg); entry != NULL;
	     entry = ldap_next_entry(ld, entry)){
			 if ((dn = ldap_get_dn(ld, entry)) != NULL){
				 //printf("Returned dn: %s\n", dn);
				 cred.bv_len = strlen(pass);
				 cred.bv_val = strdup(pass);
				 ret = ldap_sasl_bind_s( ld, dn, LDAP_SASL_SIMPLE,
				                        &cred, NULL, NULL, NULL );
				 cred.bv_len = 0;		
				 cred.bv_val = NULL;	 
				 if (check_ret(ret, LDAP_SUCCESS, "ldap_sasl_bind_s: user")){
					 result = TRUE;
				 }
			 }
	}
cleanup:
	// clean up
	if (dn != NULL) ldap_memfree(dn);
	if (entry != NULL) ldap_msgfree(entry);
	if (msg != NULL) ldap_msgfree(msg);
	unbind(ld);
	return result;
}

static gboolean check_pass ( const gchar* id, const gchar* pass)
{
	
	if (strcmp(id, localuser) == 0 )
	{
		gchar *shasum;
		shasum = g_compute_checksum_for_string (G_CHECKSUM_SHA1, pass, -1);
		//g_message(shasum);
	    if (strcmp(shasum, localuser_hash) == 0)
		{
			g_free(shasum);
			return TRUE;
		} else {
			g_free(shasum);
			return FALSE;
		}
	} else {

		gboolean ret = ldap_check_pass(id, pass);
		return ret;
	}
}