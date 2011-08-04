void populate_scsi_scanners (GHashTable* scanners)
{
  GList* epson = NULL;
  epson = g_list_append (epson, g_strdup ("GT-9700"));
  epson = g_list_append (epson, g_strdup ("GT-9800"));
  epson = g_list_append (epson, g_strdup ("Perfection1200"));
  epson = g_list_append (epson, g_strdup ("Perfection636"));
  epson = g_list_append (epson, g_strdup ("SCANNER GT-7000"));
  g_hash_table_insert (scanners,
                       g_strdup("EPSON"),
                       epson);


  GList* hp = NULL;
  hp = g_list_append (hp, g_strdup ("C1130A"));
  hp = g_list_append (hp, g_strdup ("C1750A"));
  hp = g_list_append (hp, g_strdup ("C1790A"));
  hp = g_list_append (hp, g_strdup ("C2500A"));
  hp = g_list_append (hp, g_strdup ("C2520A"));
  hp = g_list_append (hp, g_strdup ("C5110A"));
  hp = g_list_append (hp, g_strdup ("C6270A"));
  hp = g_list_append (hp, g_strdup ("C7670A"));
  g_hash_table_insert (scanners,
                       g_strdup("HP"),
                       hp);
}



void populate_usb_scanners (GHashTable* scanners)
{
  GList* hp = NULL;

  hp = g_list_append (hp, g_strdup ("0101"));
  hp = g_list_append (hp, g_strdup ("0105"));
  hp = g_list_append (hp, g_strdup ("0201"));
  hp = g_list_append (hp, g_strdup ("0205"));
  hp = g_list_append (hp, g_strdup ("0305"));
  hp = g_list_append (hp, g_strdup ("0401"));
  hp = g_list_append (hp, g_strdup ("0405"));
  hp = g_list_append (hp, g_strdup ("0505"));
  hp = g_list_append (hp, g_strdup ("0601"));
  hp = g_list_append (hp, g_strdup ("0605"));
  hp = g_list_append (hp, g_strdup ("0701"));
  hp = g_list_append (hp, g_strdup ("0705"));
  hp = g_list_append (hp, g_strdup ("0801"));
  hp = g_list_append (hp, g_strdup ("0805"));
  hp = g_list_append (hp, g_strdup ("0901"));
  hp = g_list_append (hp, g_strdup ("0a01"));
  hp = g_list_append (hp, g_strdup ("0b01"));
  hp = g_list_append (hp, g_strdup ("1005"));
  hp = g_list_append (hp, g_strdup ("1105"));
  hp = g_list_append (hp, g_strdup ("1205"));
  hp = g_list_append (hp, g_strdup ("1305"));
  hp = g_list_append (hp, g_strdup ("1405"));
  hp = g_list_append (hp, g_strdup ("1705"));
  hp = g_list_append (hp, g_strdup ("1805"));
  hp = g_list_append (hp, g_strdup ("2005"));
  hp = g_list_append (hp, g_strdup ("2205"));
  hp = g_list_append (hp, g_strdup ("2305"));
  hp = g_list_append (hp, g_strdup ("2405"));
  hp = g_list_append (hp, g_strdup ("2605"));
  hp = g_list_append (hp, g_strdup ("2805"));
  hp = g_list_append (hp, g_strdup ("3805"));
  hp = g_list_append (hp, g_strdup ("3905"));
  hp = g_list_append (hp, g_strdup ("3B17"));
  hp = g_list_append (hp, g_strdup ("4105"));
  hp = g_list_append (hp, g_strdup ("4205"));
  hp = g_list_append (hp, g_strdup ("4305"));
  hp = g_list_append (hp, g_strdup ("4505"));
  hp = g_list_append (hp, g_strdup ("4605"));
  hp = g_list_append (hp, g_strdup ("5617"));
  hp = g_list_append (hp, g_strdup ("5717"));
  
  g_hash_table_insert (scanners,
                       g_strdup("03f0"),
                       hp);

  GList* mustek_2 = NULL;
  mustek_2 = g_list_append (mustek_2, g_strdup ("1000"));
  mustek_2 = g_list_append (mustek_2, g_strdup ("1001"));
  g_hash_table_insert (scanners,
                       g_strdup("0400"),
                       mustek_2);

  GList* kodak = NULL;
  kodak = g_list_append (kodak, g_strdup ("6001"));
  kodak = g_list_append (kodak, g_strdup ("6002"));
  kodak = g_list_append (kodak, g_strdup ("6003"));
  kodak = g_list_append (kodak, g_strdup ("6004"));
  kodak = g_list_append (kodak, g_strdup ("6005"));
  g_hash_table_insert (scanners,
                       g_strdup("040a"),
                       kodak);
                                              
  GList* creative = NULL;
  
  creative = g_list_append (creative, g_strdup ("4007"));

  g_hash_table_insert (scanners,
                       g_strdup("041e"),
                       creative);
  
  GList* lexmark = NULL;
  
  lexmark = g_list_append (lexmark, g_strdup("002d"));
  lexmark = g_list_append (lexmark, g_strdup("0060"));
  lexmark = g_list_append (lexmark, g_strdup("007c"));
  lexmark = g_list_append (lexmark, g_strdup("007d"));

  g_hash_table_insert (scanners,
                       g_strdup("043d"),
                       lexmark);
  

  GList* genius = NULL;
  genius = g_list_append (genius, g_strdup("2004"));
  genius = g_list_append (genius, g_strdup("2007"));
  genius = g_list_append (genius, g_strdup("2008"));
  genius = g_list_append (genius, g_strdup("2009"));
  genius = g_list_append (genius, g_strdup("2011"));
  genius = g_list_append (genius, g_strdup("2013"));
  genius = g_list_append (genius, g_strdup("2014"));
  genius = g_list_append (genius, g_strdup("2015"));
  genius = g_list_append (genius, g_strdup("2016"));
  genius = g_list_append (genius, g_strdup("2017"));
  genius = g_list_append (genius, g_strdup("201a"));
  genius = g_list_append (genius, g_strdup("201b"));
  genius = g_list_append (genius, g_strdup("201d"));
  genius = g_list_append (genius, g_strdup("201e"));
  genius = g_list_append (genius, g_strdup("201f"));
  genius = g_list_append (genius, g_strdup("20c1"));
  g_hash_table_insert (scanners,
                       g_strdup("0458"),
                       genius);

  GList* medion = NULL;
  medion = g_list_append (medion, g_strdup("0377"));
  g_hash_table_insert (scanners,
                       g_strdup("0461"),
                       medion);

  GList* trust = NULL;
  trust = g_list_append (trust, g_strdup("1000"));
  trust = g_list_append (trust, g_strdup("1002"));
  g_hash_table_insert (scanners,
                       g_strdup("047b"),
                       trust);
                         
  GList* kyocera = NULL;
  kyocera = g_list_append (kyocera, g_strdup("0335"));
  g_hash_table_insert (scanners,
                       g_strdup("0482"),
                       kyocera);
  
  GList* compaq = NULL;
  compaq = g_list_append (compaq, g_strdup("001a"));
  g_hash_table_insert (scanners,
                       g_strdup("049f"),
                       compaq);
  GList* benq = NULL;
  benq = g_list_append (benq, g_strdup("1a20"));
  benq = g_list_append (benq, g_strdup("1a2a"));
  benq = g_list_append (benq, g_strdup("2022"));
  benq = g_list_append (benq, g_strdup("2040"));
  benq = g_list_append (benq, g_strdup("2060"));
  benq = g_list_append (benq, g_strdup("207e"));
  benq = g_list_append (benq, g_strdup("20b0"));
  benq = g_list_append (benq, g_strdup("20be"));
  benq = g_list_append (benq, g_strdup("20c0"));
  benq = g_list_append (benq, g_strdup("20de"));
  benq = g_list_append (benq, g_strdup("20f8"));
  benq = g_list_append (benq, g_strdup("20fc"));
  benq = g_list_append (benq, g_strdup("20fe"));
  benq = g_list_append (benq, g_strdup("2137"));
  benq = g_list_append (benq, g_strdup("2211"));
  g_hash_table_insert (scanners,
                       g_strdup("04a5"),
                       benq);

  GList* visioneer = NULL;
  visioneer = g_list_append (visioneer, g_strdup("0229"));
  visioneer = g_list_append (visioneer, g_strdup("0390"));
  visioneer = g_list_append (visioneer, g_strdup("0420"));
  visioneer = g_list_append (visioneer, g_strdup("0421"));
  visioneer = g_list_append (visioneer, g_strdup("0422"));
  visioneer = g_list_append (visioneer, g_strdup("0423"));
  visioneer = g_list_append (visioneer, g_strdup("0424"));
  visioneer = g_list_append (visioneer, g_strdup("0426"));
  visioneer = g_list_append (visioneer, g_strdup("0427"));
  visioneer = g_list_append (visioneer, g_strdup("0444"));
  visioneer = g_list_append (visioneer, g_strdup("0446"));
  visioneer = g_list_append (visioneer, g_strdup("0447"));
  visioneer = g_list_append (visioneer, g_strdup("0448"));
  visioneer = g_list_append (visioneer, g_strdup("0449"));
  visioneer = g_list_append (visioneer, g_strdup("044c"));
  visioneer = g_list_append (visioneer, g_strdup("0474"));
  visioneer = g_list_append (visioneer, g_strdup("0475"));
  visioneer = g_list_append (visioneer, g_strdup("0477"));
  visioneer = g_list_append (visioneer, g_strdup("0478"));
  visioneer = g_list_append (visioneer, g_strdup("0479"));
  visioneer = g_list_append (visioneer, g_strdup("047a"));
  visioneer = g_list_append (visioneer, g_strdup("047b"));
  visioneer = g_list_append (visioneer, g_strdup("047c"));
  visioneer = g_list_append (visioneer, g_strdup("048c"));
  visioneer = g_list_append (visioneer, g_strdup("048d"));
  visioneer = g_list_append (visioneer, g_strdup("048e"));
  visioneer = g_list_append (visioneer, g_strdup("048f"));
  visioneer = g_list_append (visioneer, g_strdup("0490"));
  visioneer = g_list_append (visioneer, g_strdup("0491"));
  visioneer = g_list_append (visioneer, g_strdup("0492"));
  visioneer = g_list_append (visioneer, g_strdup("0493"));
  visioneer = g_list_append (visioneer, g_strdup("0494"));
  visioneer = g_list_append (visioneer, g_strdup("0495"));
  visioneer = g_list_append (visioneer, g_strdup("0497"));
  visioneer = g_list_append (visioneer, g_strdup("0498"));
  visioneer = g_list_append (visioneer, g_strdup("0499"));
  visioneer = g_list_append (visioneer, g_strdup("049a"));
  visioneer = g_list_append (visioneer, g_strdup("049b"));
  visioneer = g_list_append (visioneer, g_strdup("049c"));
  visioneer = g_list_append (visioneer, g_strdup("049d"));
  visioneer = g_list_append (visioneer, g_strdup("04a7"));
  visioneer = g_list_append (visioneer, g_strdup("04ac"));
  g_hash_table_insert (scanners,
                       g_strdup("04a7"),
                       visioneer);
  GList* canon = NULL;
  canon = g_list_append (canon, g_strdup("1601"));
  canon = g_list_append (canon, g_strdup("1602"));
  canon = g_list_append (canon, g_strdup("1603"));
  canon = g_list_append (canon, g_strdup("1604"));
  canon = g_list_append (canon, g_strdup("1606"));
  canon = g_list_append (canon, g_strdup("1607"));
  canon = g_list_append (canon, g_strdup("1608"));
  canon = g_list_append (canon, g_strdup("1609"));
  canon = g_list_append (canon, g_strdup("160a"));
  canon = g_list_append (canon, g_strdup("160b"));
  canon = g_list_append (canon, g_strdup("1706"));
  canon = g_list_append (canon, g_strdup("1707"));
  canon = g_list_append (canon, g_strdup("1708"));
  canon = g_list_append (canon, g_strdup("1709"));
  canon = g_list_append (canon, g_strdup("170a"));
  canon = g_list_append (canon, g_strdup("170b"));
  canon = g_list_append (canon, g_strdup("170c"));
  canon = g_list_append (canon, g_strdup("170d"));
  canon = g_list_append (canon, g_strdup("170e"));
  canon = g_list_append (canon, g_strdup("1712"));
  canon = g_list_append (canon, g_strdup("1713"));
  canon = g_list_append (canon, g_strdup("1714"));
  canon = g_list_append (canon, g_strdup("1715"));
  canon = g_list_append (canon, g_strdup("1716"));
  canon = g_list_append (canon, g_strdup("1717"));
  canon = g_list_append (canon, g_strdup("1718"));
  canon = g_list_append (canon, g_strdup("1719"));
  canon = g_list_append (canon, g_strdup("171a"));
  canon = g_list_append (canon, g_strdup("171b"));
  canon = g_list_append (canon, g_strdup("171c"));
  canon = g_list_append (canon, g_strdup("1721"));
  canon = g_list_append (canon, g_strdup("1722"));
  canon = g_list_append (canon, g_strdup("1723"));
  canon = g_list_append (canon, g_strdup("1724"));
  canon = g_list_append (canon, g_strdup("1725"));
  canon = g_list_append (canon, g_strdup("1726"));
  canon = g_list_append (canon, g_strdup("1727"));
  canon = g_list_append (canon, g_strdup("1728"));
  canon = g_list_append (canon, g_strdup("1729"));
  canon = g_list_append (canon, g_strdup("172b"));
  canon = g_list_append (canon, g_strdup("172c"));
  canon = g_list_append (canon, g_strdup("172d"));
  canon = g_list_append (canon, g_strdup("172e"));
  canon = g_list_append (canon, g_strdup("172f"));
  canon = g_list_append (canon, g_strdup("1730"));
  canon = g_list_append (canon, g_strdup("1731"));
  canon = g_list_append (canon, g_strdup("1732"));
  canon = g_list_append (canon, g_strdup("1733"));
  canon = g_list_append (canon, g_strdup("1734"));
  canon = g_list_append (canon, g_strdup("1735"));
  canon = g_list_append (canon, g_strdup("1736"));
  canon = g_list_append (canon, g_strdup("173a"));
  canon = g_list_append (canon, g_strdup("173b"));
  canon = g_list_append (canon, g_strdup("173c"));
  canon = g_list_append (canon, g_strdup("173d"));
  canon = g_list_append (canon, g_strdup("173e"));
  canon = g_list_append (canon, g_strdup("173f"));
  canon = g_list_append (canon, g_strdup("1740"));
  canon = g_list_append (canon, g_strdup("1741"));
  canon = g_list_append (canon, g_strdup("1742"));
  canon = g_list_append (canon, g_strdup("1901"));
  canon = g_list_append (canon, g_strdup("1904"));
  canon = g_list_append (canon, g_strdup("1905"));
  canon = g_list_append (canon, g_strdup("1909"));
  canon = g_list_append (canon, g_strdup("190a"));
  canon = g_list_append (canon, g_strdup("2204"));
  canon = g_list_append (canon, g_strdup("2206"));
  canon = g_list_append (canon, g_strdup("2207"));
  canon = g_list_append (canon, g_strdup("2208"));
  canon = g_list_append (canon, g_strdup("220d"));
  canon = g_list_append (canon, g_strdup("220e"));
  canon = g_list_append (canon, g_strdup("2213"));
  canon = g_list_append (canon, g_strdup("221c"));
  canon = g_list_append (canon, g_strdup("2220"));
  canon = g_list_append (canon, g_strdup("2222"));
  canon = g_list_append (canon, g_strdup("262f"));
  canon = g_list_append (canon, g_strdup("2630"));
  canon = g_list_append (canon, g_strdup("263c"));
  canon = g_list_append (canon, g_strdup("263d"));
  canon = g_list_append (canon, g_strdup("263e"));
  canon = g_list_append (canon, g_strdup("263f"));
  canon = g_list_append (canon, g_strdup("264c"));
  canon = g_list_append (canon, g_strdup("264d"));
  canon = g_list_append (canon, g_strdup("264e"));
  canon = g_list_append (canon, g_strdup("264f"));
  canon = g_list_append (canon, g_strdup("2659"));
  canon = g_list_append (canon, g_strdup("265d"));
  canon = g_list_append (canon, g_strdup("265e"));
  canon = g_list_append (canon, g_strdup("265f"));
  canon = g_list_append (canon, g_strdup("2660"));
  canon = g_list_append (canon, g_strdup("2684"));
  canon = g_list_append (canon, g_strdup("2686"));
  canon = g_list_append (canon, g_strdup("26a3"));
  canon = g_list_append (canon, g_strdup("26b0"));
  canon = g_list_append (canon, g_strdup("26b4"));
  canon = g_list_append (canon, g_strdup("26b5"));
  canon = g_list_append (canon, g_strdup("26ec"));
  canon = g_list_append (canon, g_strdup("26ed"));
  canon = g_list_append (canon, g_strdup("26ee"));
  g_hash_table_insert (scanners,
                       g_strdup("04a9"),
                       canon);
  
  GList* nikon = NULL;
  nikon = g_list_append (nikon, g_strdup ("4000"));
  nikon = g_list_append (nikon, g_strdup ("4001"));
  nikon = g_list_append (nikon, g_strdup ("4002"));
  g_hash_table_insert (scanners,
                       g_strdup("04b0"),
                       nikon);

  GList* epson = NULL;

  // for testing (its a printer not a scanner!)  
  //epson = g_list_append (epson, g_strdup ("0001"));
  
  epson = g_list_append (epson, g_strdup("0101"));
  epson = g_list_append (epson, g_strdup("0103"));
  epson = g_list_append (epson, g_strdup("0104"));
  epson = g_list_append (epson, g_strdup("0105"));
  epson = g_list_append (epson, g_strdup("0106"));
  epson = g_list_append (epson, g_strdup("0107"));
  epson = g_list_append (epson, g_strdup("0109"));
  epson = g_list_append (epson, g_strdup("010a"));
  epson = g_list_append (epson, g_strdup("010b"));
  epson = g_list_append (epson, g_strdup("010c"));
  epson = g_list_append (epson, g_strdup("010e"));
  epson = g_list_append (epson, g_strdup("010f"));
  epson = g_list_append (epson, g_strdup("0110"));
  epson = g_list_append (epson, g_strdup("0112"));
  epson = g_list_append (epson, g_strdup("0114"));
  epson = g_list_append (epson, g_strdup("011b"));
  epson = g_list_append (epson, g_strdup("011c"));
  epson = g_list_append (epson, g_strdup("011d"));
  epson = g_list_append (epson, g_strdup("011e"));
  epson = g_list_append (epson, g_strdup("011f"));
  epson = g_list_append (epson, g_strdup("0120"));
  epson = g_list_append (epson, g_strdup("0121"));
  epson = g_list_append (epson, g_strdup("0122"));
  epson = g_list_append (epson, g_strdup("0126"));
  epson = g_list_append (epson, g_strdup("0128"));
  epson = g_list_append (epson, g_strdup("0129"));
  epson = g_list_append (epson, g_strdup("012a"));
  epson = g_list_append (epson, g_strdup("012b"));
  epson = g_list_append (epson, g_strdup("012c"));
  epson = g_list_append (epson, g_strdup("0135"));
  epson = g_list_append (epson, g_strdup("0801"));
  epson = g_list_append (epson, g_strdup("0802"));
  epson = g_list_append (epson, g_strdup("0805"));
  epson = g_list_append (epson, g_strdup("0806"));
  epson = g_list_append (epson, g_strdup("0807"));
  epson = g_list_append (epson, g_strdup("0808"));
  epson = g_list_append (epson, g_strdup("080c"));
  epson = g_list_append (epson, g_strdup("080d"));
  epson = g_list_append (epson, g_strdup("080e"));
  epson = g_list_append (epson, g_strdup("080f"));
  epson = g_list_append (epson, g_strdup("0810"));
  epson = g_list_append (epson, g_strdup("0811"));
  epson = g_list_append (epson, g_strdup("0813"));
  epson = g_list_append (epson, g_strdup("0814"));
  epson = g_list_append (epson, g_strdup("0815"));
  epson = g_list_append (epson, g_strdup("0817"));
  epson = g_list_append (epson, g_strdup("0818"));
  epson = g_list_append (epson, g_strdup("0819"));
  epson = g_list_append (epson, g_strdup("081a"));
  epson = g_list_append (epson, g_strdup("081c"));
  epson = g_list_append (epson, g_strdup("081d"));
  epson = g_list_append (epson, g_strdup("081f"));
  epson = g_list_append (epson, g_strdup("0820"));
  epson = g_list_append (epson, g_strdup("0827"));
  epson = g_list_append (epson, g_strdup("0828"));
  epson = g_list_append (epson, g_strdup("0829"));
  epson = g_list_append (epson, g_strdup("082a"));
  epson = g_list_append (epson, g_strdup("082b"));
  epson = g_list_append (epson, g_strdup("082e"));
  epson = g_list_append (epson, g_strdup("082f"));
  epson = g_list_append (epson, g_strdup("0830"));
  epson = g_list_append (epson, g_strdup("0833"));
  epson = g_list_append (epson, g_strdup("0834"));
  epson = g_list_append (epson, g_strdup("0835"));
  epson = g_list_append (epson, g_strdup("0836"));
  epson = g_list_append (epson, g_strdup("0837"));
  epson = g_list_append (epson, g_strdup("0838"));
  epson = g_list_append (epson, g_strdup("0839"));
  epson = g_list_append (epson, g_strdup("083a"));
  epson = g_list_append (epson, g_strdup("083c"));
  epson = g_list_append (epson, g_strdup("0841"));
  epson = g_list_append (epson, g_strdup("0843"));
  epson = g_list_append (epson, g_strdup("0844"));
  epson = g_list_append (epson, g_strdup("0846"));
  epson = g_list_append (epson, g_strdup("0847"));
  epson = g_list_append (epson, g_strdup("0848"));
  epson = g_list_append (epson, g_strdup("0849"));
  epson = g_list_append (epson, g_strdup("084a"));
  epson = g_list_append (epson, g_strdup("084c"));
  epson = g_list_append (epson, g_strdup("084d"));
  epson = g_list_append (epson, g_strdup("084f"));
  epson = g_list_append (epson, g_strdup("0851"));
  epson = g_list_append (epson, g_strdup("0854"));
  epson = g_list_append (epson, g_strdup("0856"));
  g_hash_table_insert (scanners,
                       g_strdup("04b8"),
                       epson);

  GList* fujitsu = NULL;
  fujitsu = g_list_append (fujitsu, g_strdup ("1029"));
  fujitsu = g_list_append (fujitsu, g_strdup ("1041"));
  fujitsu = g_list_append (fujitsu, g_strdup ("1042"));
  fujitsu = g_list_append (fujitsu, g_strdup ("1078"));
  fujitsu = g_list_append (fujitsu, g_strdup ("1095"));
  fujitsu = g_list_append (fujitsu, g_strdup ("1096"));
  fujitsu = g_list_append (fujitsu, g_strdup ("1097"));
  fujitsu = g_list_append (fujitsu, g_strdup ("10ad"));
  fujitsu = g_list_append (fujitsu, g_strdup ("10ae"));
  fujitsu = g_list_append (fujitsu, g_strdup ("10af"));
  fujitsu = g_list_append (fujitsu, g_strdup ("10c7"));
  fujitsu = g_list_append (fujitsu, g_strdup ("10cf"));
  fujitsu = g_list_append (fujitsu, g_strdup ("10e0"));
  fujitsu = g_list_append (fujitsu, g_strdup ("10e1"));
  fujitsu = g_list_append (fujitsu, g_strdup ("10e2"));
  fujitsu = g_list_append (fujitsu, g_strdup ("10e6"));
  fujitsu = g_list_append (fujitsu, g_strdup ("10e7"));
  fujitsu = g_list_append (fujitsu, g_strdup ("10ef"));
  fujitsu = g_list_append (fujitsu, g_strdup ("10f2"));
  fujitsu = g_list_append (fujitsu, g_strdup ("10fe"));
  fujitsu = g_list_append (fujitsu, g_strdup ("1135"));
  fujitsu = g_list_append (fujitsu, g_strdup ("114a"));
  fujitsu = g_list_append (fujitsu, g_strdup ("114d"));
  fujitsu = g_list_append (fujitsu, g_strdup ("114e"));
  fujitsu = g_list_append (fujitsu, g_strdup ("114f"));
  fujitsu = g_list_append (fujitsu, g_strdup ("1150"));
  fujitsu = g_list_append (fujitsu, g_strdup ("1155"));
  fujitsu = g_list_append (fujitsu, g_strdup ("1156"));
  fujitsu = g_list_append (fujitsu, g_strdup ("116f"));
  fujitsu = g_list_append (fujitsu, g_strdup ("1174"));
  fujitsu = g_list_append (fujitsu, g_strdup ("1175"));
  fujitsu = g_list_append (fujitsu, g_strdup ("1176"));
  fujitsu = g_list_append (fujitsu, g_strdup ("1177"));
  fujitsu = g_list_append (fujitsu, g_strdup ("1178"));
  fujitsu = g_list_append (fujitsu, g_strdup ("117f"));
  fujitsu = g_list_append (fujitsu, g_strdup ("119d"));
  fujitsu = g_list_append (fujitsu, g_strdup ("119e"));
  fujitsu = g_list_append (fujitsu, g_strdup ("119f"));
  fujitsu = g_list_append (fujitsu, g_strdup ("11a0"));
  fujitsu = g_list_append (fujitsu, g_strdup ("11a2"));
  fujitsu = g_list_append (fujitsu, g_strdup ("11ed"));
  fujitsu = g_list_append (fujitsu, g_strdup ("11ee"));
  fujitsu = g_list_append (fujitsu, g_strdup ("11ef"));
  fujitsu = g_list_append (fujitsu, g_strdup ("11f1"));
  fujitsu = g_list_append (fujitsu, g_strdup ("11f2"));
  fujitsu = g_list_append (fujitsu, g_strdup ("11f3"));
  fujitsu = g_list_append (fujitsu, g_strdup ("11f4"));
  fujitsu = g_list_append (fujitsu, g_strdup ("11fc"));
  g_hash_table_insert (scanners,
                       g_strdup("04c5"),
                       fujitsu);
  GList* konica = NULL;
  konica = g_list_append (konica, g_strdup ("0722"));
  g_hash_table_insert (scanners,
                       g_strdup("04c8"),
                       konica);
  GList* panasonic = NULL;
  panasonic = g_list_append (panasonic, g_strdup ("1000"));
  panasonic = g_list_append (panasonic, g_strdup ("1001"));
  panasonic = g_list_append (panasonic, g_strdup ("1006"));
  panasonic = g_list_append (panasonic, g_strdup ("1007"));
  panasonic = g_list_append (panasonic, g_strdup ("1009"));
  panasonic = g_list_append (panasonic, g_strdup ("100a"));
  panasonic = g_list_append (panasonic, g_strdup ("100f"));
  panasonic = g_list_append (panasonic, g_strdup ("1010"));
  g_hash_table_insert (scanners,
                       g_strdup("04da"),
                       panasonic);

  GList* samsung = NULL;
  
  samsung = g_list_append (samsung, g_strdup ("341b"));
  samsung = g_list_append (samsung, g_strdup ("341f"));
  samsung = g_list_append (samsung, g_strdup ("3426"));
  samsung = g_list_append (samsung, g_strdup ("342a"));
  samsung = g_list_append (samsung, g_strdup ("342b"));
  samsung = g_list_append (samsung, g_strdup ("342c"));
  samsung = g_list_append (samsung, g_strdup ("3433"));
  samsung = g_list_append (samsung, g_strdup ("3434"));
  samsung = g_list_append (samsung, g_strdup ("343c"));
  samsung = g_list_append (samsung, g_strdup ("3434"));
  g_hash_table_insert (scanners,
                       g_strdup("04e8"),
                       samsung);
  
  GList* pentax = NULL;
  pentax = g_list_append (pentax, g_strdup ("2038"));
  g_hash_table_insert (scanners,
                       g_strdup("04f9"),
                       pentax);

  GList* apitek = NULL;
  apitek = g_list_append (apitek, g_strdup ("0202"));
  g_hash_table_insert (scanners,
                       g_strdup("0553"),
                       apitek);

  GList* mustek = NULL;
  mustek = g_list_append (mustek, g_strdup ("0001"));
  mustek = g_list_append (mustek, g_strdup ("0002"));
  mustek = g_list_append (mustek, g_strdup ("0006"));
  mustek = g_list_append (mustek, g_strdup ("0008"));
  mustek = g_list_append (mustek, g_strdup ("0010"));
  mustek = g_list_append (mustek, g_strdup ("0210"));
  mustek = g_list_append (mustek, g_strdup ("0218"));
  mustek = g_list_append (mustek, g_strdup ("0219"));
  mustek = g_list_append (mustek, g_strdup ("021a"));
  mustek = g_list_append (mustek, g_strdup ("021b"));
  mustek = g_list_append (mustek, g_strdup ("021c"));
  mustek = g_list_append (mustek, g_strdup ("021d"));
  mustek = g_list_append (mustek, g_strdup ("021e"));
  mustek = g_list_append (mustek, g_strdup ("021f"));
  mustek = g_list_append (mustek, g_strdup ("0409"));
  g_hash_table_insert (scanners,
                       g_strdup("055f"),
                       mustek);
  GList* artec = NULL;
  artec = g_list_append (artec, g_strdup ("4002"));
  artec = g_list_append (artec, g_strdup ("4003"));
  artec = g_list_append (artec, g_strdup ("4004"));
  artec = g_list_append (artec, g_strdup ("4005"));
  artec = g_list_append (artec, g_strdup ("4006"));
  artec = g_list_append (artec, g_strdup ("4007"));
  artec = g_list_append (artec, g_strdup ("4009"));
  artec = g_list_append (artec, g_strdup ("4010"));
  artec = g_list_append (artec, g_strdup ("4011"));
  g_hash_table_insert (scanners,
                       g_strdup("05d8"),
                       artec);
  
  GList* microtek = NULL;
  microtek = g_list_append (microtek, g_strdup ("0099"));
  microtek = g_list_append (microtek, g_strdup ("009a"));
  microtek = g_list_append (microtek, g_strdup ("00a3"));
  microtek = g_list_append (microtek, g_strdup ("00b6"));
  microtek = g_list_append (microtek, g_strdup ("30cf"));
  microtek = g_list_append (microtek, g_strdup ("30d4"));
  microtek = g_list_append (microtek, g_strdup ("40b3"));
  microtek = g_list_append (microtek, g_strdup ("40b8"));
  microtek = g_list_append (microtek, g_strdup ("40ca"));
  microtek = g_list_append (microtek, g_strdup ("40cb"));
  microtek = g_list_append (microtek, g_strdup ("40dd"));
  microtek = g_list_append (microtek, g_strdup ("40ff"));
  microtek = g_list_append (microtek, g_strdup ("80a3"));
  g_hash_table_insert (scanners,
                       g_strdup("05da"),
                       microtek);
  
  GList* avision = NULL;
  avision = g_list_append (avision, g_strdup ("0268"));
  avision = g_list_append (avision, g_strdup ("026a"));
  avision = g_list_append (avision, g_strdup ("0a13"));
  avision = g_list_append (avision, g_strdup ("0a15"));
  avision = g_list_append (avision, g_strdup ("0a16"));
  avision = g_list_append (avision, g_strdup ("0a18"));
  avision = g_list_append (avision, g_strdup ("0a19"));
  avision = g_list_append (avision, g_strdup ("0a23"));
  avision = g_list_append (avision, g_strdup ("0a24"));
  avision = g_list_append (avision, g_strdup ("0a25"));
  avision = g_list_append (avision, g_strdup ("0a27"));
  avision = g_list_append (avision, g_strdup ("0a2a"));
  avision = g_list_append (avision, g_strdup ("0a2b"));
  avision = g_list_append (avision, g_strdup ("0a2c"));
  avision = g_list_append (avision, g_strdup ("0a2d"));
  avision = g_list_append (avision, g_strdup ("0a2e"));
  avision = g_list_append (avision, g_strdup ("0a2f"));
  avision = g_list_append (avision, g_strdup ("0a33"));
  avision = g_list_append (avision, g_strdup ("0a3a"));
  avision = g_list_append (avision, g_strdup ("0a3c"));
  avision = g_list_append (avision, g_strdup ("0a40"));
  avision = g_list_append (avision, g_strdup ("0a41"));
  avision = g_list_append (avision, g_strdup ("0a45"));
  avision = g_list_append (avision, g_strdup ("0a4d"));
  avision = g_list_append (avision, g_strdup ("0a4e"));
  avision = g_list_append (avision, g_strdup ("0a4f"));
  avision = g_list_append (avision, g_strdup ("0a5e"));
  avision = g_list_append (avision, g_strdup ("0a61"));
  avision = g_list_append (avision, g_strdup ("0a65"));
  avision = g_list_append (avision, g_strdup ("0a66"));
  avision = g_list_append (avision, g_strdup ("0a68"));
  avision = g_list_append (avision, g_strdup ("0a82"));
  avision = g_list_append (avision, g_strdup ("0a84"));
  avision = g_list_append (avision, g_strdup ("0a93"));
  avision = g_list_append (avision, g_strdup ("0a94"));
  avision = g_list_append (avision, g_strdup ("0aa1"));
  avision = g_list_append (avision, g_strdup ("1a35"));
  g_hash_table_insert (scanners,
                       g_strdup("0638"),
                       avision);
  GList* minolta = NULL;
  minolta = g_list_append (minolta, g_strdup ("4004"));
  minolta = g_list_append (minolta, g_strdup ("400d"));
  minolta = g_list_append (minolta, g_strdup ("400e"));
  g_hash_table_insert (scanners,
                       g_strdup("0686"),
                       minolta);

  GList* agfa = NULL;
  agfa = g_list_append (agfa, g_strdup ("0001"));
  agfa = g_list_append (agfa, g_strdup ("0002"));
  agfa = g_list_append (agfa, g_strdup ("0100"));
  agfa = g_list_append (agfa, g_strdup ("2061"));
  agfa = g_list_append (agfa, g_strdup ("208d"));
  agfa = g_list_append (agfa, g_strdup ("208f"));
  agfa = g_list_append (agfa, g_strdup ("2091"));
  agfa = g_list_append (agfa, g_strdup ("2093"));
  agfa = g_list_append (agfa, g_strdup ("2095"));
  agfa = g_list_append (agfa, g_strdup ("2097"));
  agfa = g_list_append (agfa, g_strdup ("20fd"));
  agfa = g_list_append (agfa, g_strdup ("20ff"));
  g_hash_table_insert (scanners,
                       g_strdup("06bd"),
                       minolta);
  
  GList* umax_2 = NULL;
  umax_2 = g_list_append (umax_2, g_strdup ("0020"));
  g_hash_table_insert (scanners,
                       g_strdup("06dc"),
                       umax_2);

  GList* plustek = NULL;
  
  plustek = g_list_append (plustek, g_strdup ("0001"));
  plustek = g_list_append (plustek, g_strdup ("0010"));
  plustek = g_list_append (plustek, g_strdup ("0011"));
  plustek = g_list_append (plustek, g_strdup ("0013"));
  plustek = g_list_append (plustek, g_strdup ("0015"));
  plustek = g_list_append (plustek, g_strdup ("0017"));
  plustek = g_list_append (plustek, g_strdup ("0400"));
  plustek = g_list_append (plustek, g_strdup ("0401"));
  plustek = g_list_append (plustek, g_strdup ("0402"));
  plustek = g_list_append (plustek, g_strdup ("0403"));
  plustek = g_list_append (plustek, g_strdup ("040b"));
  plustek = g_list_append (plustek, g_strdup ("040e"));
  plustek = g_list_append (plustek, g_strdup ("0412"));
  plustek = g_list_append (plustek, g_strdup ("0413"));
  plustek = g_list_append (plustek, g_strdup ("0422"));
  plustek = g_list_append (plustek, g_strdup ("0454"));
  plustek = g_list_append (plustek, g_strdup ("045f"));
  plustek = g_list_append (plustek, g_strdup ("0462"));
  plustek = g_list_append (plustek, g_strdup ("0900"));
  g_hash_table_insert (scanners,
                       g_strdup("07b3"),
                       plustek);

  GList* corex = NULL;
  corex = g_list_append (corex, g_strdup ("0002"));
  corex = g_list_append (corex, g_strdup ("0005"));
  g_hash_table_insert (scanners,
                       g_strdup("08f0"),
                       corex);
  
  GList* xerox = NULL;
  xerox = g_list_append (xerox, g_strdup ("3d5d"));
  xerox = g_list_append (xerox, g_strdup ("3da4"));
  xerox = g_list_append (xerox, g_strdup ("420c"));
  xerox = g_list_append (xerox, g_strdup ("4265"));
  xerox = g_list_append (xerox, g_strdup ("4293"));
  xerox = g_list_append (xerox, g_strdup ("4294"));
  g_hash_table_insert (scanners,
                       g_strdup("0924"),
                       xerox);
  
  GList* pentax_2 = NULL;
  pentax_2 = g_list_append (pentax_2, g_strdup ("3210"));
  g_hash_table_insert (scanners,
                       g_strdup("0a17"),
                       pentax_2);

  GList* portable = NULL;
  portable = g_list_append (portable, g_strdup ("1000"));
  g_hash_table_insert (scanners,
                       g_strdup("0a53"),
                       portable);
  
  GList* syscan = NULL;
  syscan = g_list_append (syscan, g_strdup ("4600"));
  syscan = g_list_append (syscan, g_strdup ("4802"));
  syscan = g_list_append (syscan, g_strdup ("4803"));
  syscan = g_list_append (syscan, g_strdup ("480c"));
  syscan = g_list_append (syscan, g_strdup ("4810"));
  syscan = g_list_append (syscan, g_strdup ("6620"));
  g_hash_table_insert (scanners,
                       g_strdup("0a82"),
                       syscan);
  
  GList* canon_2 = NULL;
  canon_2 = g_list_append (canon_2, g_strdup ("160c"));  
  canon_2 = g_list_append (canon_2, g_strdup ("160f"));  
  canon_2 = g_list_append (canon_2, g_strdup ("1614"));  
  canon_2 = g_list_append (canon_2, g_strdup ("1617"));  
  canon_2 = g_list_append (canon_2, g_strdup ("1618"));  
  canon_2 = g_list_append (canon_2, g_strdup ("161a"));  
  canon_2 = g_list_append (canon_2, g_strdup ("161b"));  
  canon_2 = g_list_append (canon_2, g_strdup ("161d"));  
  canon_2 = g_list_append (canon_2, g_strdup ("1620"));  
  canon_2 = g_list_append (canon_2, g_strdup ("1622"));  
  canon_2 = g_list_append (canon_2, g_strdup ("1623"));  
  canon_2 = g_list_append (canon_2, g_strdup ("1624"));  
  canon_2 = g_list_append (canon_2, g_strdup ("1626"));  
  canon_2 = g_list_append (canon_2, g_strdup ("162b"));  
  canon_2 = g_list_append (canon_2, g_strdup ("1638"));  
  canon_2 = g_list_append (canon_2, g_strdup ("1639"));  
  g_hash_table_insert (scanners,
                       g_strdup("1083"),
                       canon_2);
  
  GList* digital = NULL;
  digital = g_list_append (digital, g_strdup ("0001"));
  g_hash_table_insert (scanners,
                       g_strdup("1183"),
                       digital);
  
  GList* konica_2 = NULL;
  konica_2 = g_list_append (konica_2, g_strdup ("2089"));
  g_hash_table_insert (scanners,
                       g_strdup("132b"),
                       konica_2);
  
  GList* umax = NULL;
  umax = g_list_append (umax, g_strdup ("0010"));
  umax = g_list_append (umax, g_strdup ("0030"));
  umax = g_list_append (umax, g_strdup ("0050"));
  umax = g_list_append (umax, g_strdup ("0060"));
  umax = g_list_append (umax, g_strdup ("0070"));
  umax = g_list_append (umax, g_strdup ("0130"));
  umax = g_list_append (umax, g_strdup ("0160"));
  umax = g_list_append (umax, g_strdup ("0230"));
  g_hash_table_insert (scanners,
                       g_strdup("1606"),
                       umax);

  GList* docketport = NULL;
  docketport = g_list_append (docketport, g_strdup ("4810"));  
  g_hash_table_insert (scanners,
                       g_strdup("1dcc"),
                       docketport);

  GList* dell = NULL;
  dell = g_list_append (dell, g_strdup ("5105"));
  dell = g_list_append (dell, g_strdup ("5124"));
  dell = g_list_append (dell, g_strdup ("5250"));
  g_hash_table_insert (scanners,
                       g_strdup("413c"),
                       dell);
}
