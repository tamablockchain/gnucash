/********************************************************************\
 * gnc-hbci-kvp.c -- hbci kvp handling                              *
 * Copyright (C) 2002 Christian Stimming                            *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
\********************************************************************/

#include "gnc-hbci-kvp.h"

#define HBCI_KEY "hbci"
#define HBCI_ACCOUNT_ID "account-id"
#define HBCI_BANK_CODE "bank-code"
#define HBCI_COUNTRY_CODE "country-code"
#define HBCI_TRANS_RETRIEVAL "trans-retrieval"

/* Account */
char *gnc_hbci_get_account_accountid (Account *a)
{
  kvp_frame *frame = gnc_hbci_get_account_kvp (a);
  kvp_value *value = kvp_frame_get_slot (frame, HBCI_ACCOUNT_ID);
  return kvp_value_get_string (value);
}
char *gnc_hbci_get_account_bankcode (Account *a)
{
  kvp_frame *frame = gnc_hbci_get_account_kvp (a);
  kvp_value *value = kvp_frame_get_slot (frame, HBCI_BANK_CODE);
  return kvp_value_get_string (value);
}
gint gnc_hbci_get_account_countrycode (Account *a)
{
  kvp_frame *frame = gnc_hbci_get_account_kvp (a);
  kvp_value *value = kvp_frame_get_slot (frame, HBCI_COUNTRY_CODE);
  return kvp_value_get_gint64 (value);
}
Timespec gnc_hbci_get_account_trans_retrieval (Account *a)
{
  kvp_frame *frame = gnc_hbci_get_account_kvp (a);
  kvp_value *value = kvp_frame_get_slot (frame, HBCI_TRANS_RETRIEVAL);
  return kvp_value_get_timespec (value);
}
void gnc_hbci_set_account_accountid (Account *a, const char *id)
{
  kvp_frame *frame = gnc_hbci_get_account_kvp (a);
  kvp_value *value = kvp_value_new_string (id);
  xaccAccountBeginEdit(a);
  kvp_frame_set_slot_nc (frame, HBCI_ACCOUNT_ID, value);
  xaccAccountCommitEdit (a);
}
void gnc_hbci_set_account_bankcode (Account *a, const char *code)
{
  kvp_frame *frame = gnc_hbci_get_account_kvp (a);
  kvp_value *value = kvp_value_new_string (code);
  xaccAccountBeginEdit (a);
  kvp_frame_set_slot_nc (frame, HBCI_BANK_CODE, value);
  xaccAccountCommitEdit (a);
}
void gnc_hbci_set_account_countrycode (Account *a, gint code)
{
  kvp_frame *frame = gnc_hbci_get_account_kvp (a);
  kvp_value *value = kvp_value_new_gint64 (code);
  xaccAccountBeginEdit (a);
  kvp_frame_set_slot_nc (frame, HBCI_COUNTRY_CODE, value);
  xaccAccountCommitEdit (a);
}
void gnc_hbci_set_account_trans_retrieval (Account *a, Timespec time)
{
  kvp_frame *frame = gnc_hbci_get_account_kvp (a);
  kvp_value *value = kvp_value_new_timespec (time);
  xaccAccountBeginEdit (a);
  kvp_frame_set_slot_nc (frame, HBCI_TRANS_RETRIEVAL, value);
  xaccAccountCommitEdit (a);
}



/* GNCBook */
#define HBCI_CONFIGFILE "config-filename"
#define HBCI_TEMPLATES "template-list"

char *gnc_hbci_get_book_configfile (GNCBook *b)
{
  kvp_frame *frame = gnc_hbci_get_book_kvp (b);
  kvp_value *value = kvp_frame_get_slot (frame, HBCI_CONFIGFILE);
  return kvp_value_get_string (value);
}
void gnc_hbci_set_book_configfile (GNCBook *b, const char *filename)
{
  kvp_frame *frame = gnc_hbci_get_book_kvp (b);
  kvp_value *value = kvp_value_new_string (filename);
  kvp_frame_set_slot_nc (frame, HBCI_CONFIGFILE, value);
  gnc_book_kvp_changed (b);
}
GList *gnc_hbci_get_book_template_list (GNCBook *b)
{
  kvp_frame *frame = gnc_hbci_get_book_kvp (b);
  kvp_value *value = kvp_frame_get_slot (frame, HBCI_TEMPLATES);
  return kvp_value_get_glist (value);
}
void gnc_hbci_set_book_template_list (GNCBook *b, GList *template_list)
{
  kvp_frame *frame = gnc_hbci_get_book_kvp (b);
  kvp_value *value = kvp_value_new_glist_nc (template_list);
  kvp_frame_set_slot_nc (frame, HBCI_TEMPLATES, value);
  gnc_book_kvp_changed (b);
}


/* lowlevel */
/* getters  for kvp frame in book */
kvp_frame *gnc_hbci_get_book_kvp (GNCBook *b)
{
  kvp_frame *toplevel = gnc_book_get_slots (b);
  return kvp_frame_get_frame (toplevel, HBCI_KEY, NULL);
}



/* kvp frame in Account */
kvp_frame *gnc_hbci_get_account_kvp (Account *a)
{
  kvp_frame *toplevel = xaccAccountGetSlots (a);
  return kvp_frame_get_frame (toplevel, HBCI_KEY, NULL);
}