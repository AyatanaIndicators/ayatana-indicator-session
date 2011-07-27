                                             

void populate_usb_scanners (GHashTable* scanners)
{
  GList* epson = NULL;
  // For testing
  epson = g_list_append (epson, g_strdup("0001"));
  epson = g_list_append (epson, g_strdup("0101"));
  epson = g_list_append (epson, g_strdup("0103"));
  epson = g_list_append (epson, g_strdup("0104"));
  epson = g_list_append (epson, g_strdup("0105"));
  epson = g_list_append (epson, g_strdup("0106"));
  epson = g_list_append (epson, g_strdup("0107"));
  epson = g_list_append (epson, g_strdup("0109"));
  epson = g_list_append (epson, g_strdup("010a"));
  epson = g_list_append (epson, g_strdup("010b"));

  g_hash_table_insert (scanners,
                       g_strdup("04b8"),
                       g_list_copy(epson));
}
