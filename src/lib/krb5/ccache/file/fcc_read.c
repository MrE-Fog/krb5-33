/*
 * $Source$
 * $Author$
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <krb5/mit-copyright.h>.
 *
 * This file contains the source code for reading variables from a
 * credentials cache.  These are not library-exported functions.
 */

#if !defined(lint) && !defined(SABER)
static char fcc_read_c[] = "$Id$";
#endif /* !lint && !SABER */

#include <krb5/copyright.h>
#include "fcc.h"

/* XXX Deal with kret return values */
#define CHECK(ret) if (ret != KRB5_OK) return ret;
     
/* XXX Doesn't deal if < sizeof(o) bytes are written XXX */
#define krb5_fcc_read(i,b,l) (read(((krb5_fcc_data *)i->data)->fd,b,l) == -1 \
			      ? errno : KRB5_OK)

/*
 * FOR ALL OF THE FOLLOWING FUNCTIONS:
 *
 * Requires:
 * id is open and set to read at the appropriate place in the file
 *
 * Effects:
 * Fills in the second argument with data of the appropriate type from
 * the file.  In some cases, the functions have to allocate space for
 * variable length fields; therefore, krb5_destroy_<type> must be
 * called for each filled in structure.
 *
 * Errors:
 * system errors (read errors)
 * KRB5_NOMEM
 */

krb5_error_code
krb5_fcc_read_principal(id, princ)
   krb5_ccache id;
   krb5_principal *princ;
{
     krb5_error_code kret;
     krb5_int32 length;
     int i;

     /* Read the number of components */
     kret = krb5_fcc_read_int32(id, &length);
     CHECK(kret);

     /*
      * The # of levels of indirection is confusing.  A krb5_principal
      * is an array of pointers to krb5_data.  princ is a pointer to
      * an array of pointers to krb5_data.  (*princ)[i] a pointer to
      * krb5_data.
      */

     /* Make *princ able to hold length pointers to krb5_data structs */
     *princ = (krb5_principal) malloc(sizeof(krb5_data *)*length);
     if (*princ == NULL)
	  return KRB5_NOMEM;

     /* XXX Memory leak XXX */
     for (i=0; i < length; i++) {
	  (*princ)[i] = (krb5_data *) malloc(sizeof(krb5_data));
	  if ((*princ)[i] == NULL)
	       return KRB5_NOMEM;
	  
	  kret = krb5_fcc_read_data(id, (*princ)[i]);
	  CHECK(kret);
     }

     return KRB5_OK;
}

krb5_error_code
krb5_fcc_read_keyblock(id, keyblock)
   krb5_ccache id;
   krb5_keyblock *keyblock;
{
     krb5_error_code kret;
     char *temp; /* HACK! */
     int ret;

     /*
      * XXX The third field should be a char *, not a char[1].
      * Alternatively, I am *VERY* confused.
      */

     kret = krb5_fcc_read_keytype(id, &keyblock->keytype);
     CHECK(kret);
     kret = krb5_fcc_read_int(id, &keyblock->length);
     CHECK(kret);

     temp = (char *) malloc(keyblock->length*sizeof(krb5_octet));
     if (temp == NULL)
	  return KRB5_NOMEM;
     
     ret = read(((krb5_fcc_data *) id->data)->fd, temp,
		(keyblock->length)*sizeof(krb5_octet));

     /*
     keyblock->contents = (char *) malloc(keyblock->length*sizeof(krb5_octet));
     if (keyblock->contents == NULL)
	  return KRB5_NOMEM;
     
     ret = read(((krb5_fcc_data *) id->data)->fd, keyblock->contents,
		(keyblock->length)*sizeof(krb5_octet));
     */

     if (ret < 0)
	  return errno;
     else
	  return KRB5_OK;
}

krb5_error_code
krb5_fcc_read_data(id, data)
   krb5_ccache id;
   krb5_data *data;
{
     krb5_error_code kret;
     int ret;

     kret = krb5_fcc_read_int(id, &data->length);
     CHECK(kret);

     data->data = (char *) malloc(data->length);
     if (data->data == NULL)
	  return KRB5_NOMEM;

     ret = read(((krb5_fcc_data *) id->data)->fd, data->data, data->length);
     if (ret == -1)
	  return errno;

     return KRB5_OK;
}

krb5_error_code
krb5_fcc_read_int32(id, i)
   krb5_ccache id;
   krb5_int32 *i;
{
     return krb5_fcc_read(id, i, sizeof(krb5_int32));
}

krb5_error_code
krb5_fcc_read_keytype(id, k)
   krb5_ccache id;
   krb5_keytype *k;
{
     return krb5_fcc_read(id, k, sizeof(krb5_keytype));
}

krb5_error_code
krb5_fcc_read_int(id, i)
   krb5_ccache id;
   int *i;
{
     return krb5_fcc_read(id, i, sizeof(int));
}

krb5_error_code
krb5_fcc_read_bool(id, b)
   krb5_ccache id;
   krb5_boolean *b;
{
     return krb5_fcc_read(id, b, sizeof(krb5_boolean));
}

krb5_error_code
krb5_fcc_read_times(id, t)
   krb5_ccache id;
   krb5_ticket_times *t;
{
     return krb5_fcc_read(id, t, sizeof(krb5_ticket_times));
}

krb5_error_code
krb5_fcc_read_flags(id, f)
   krb5_ccache id;
   krb5_flags *f;
{
     return krb5_fcc_read(id, f, sizeof(krb5_flags));
}
