
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
                       g_list_copy(hp));

  GList* mustek = NULL;

  mustek = g_list_append (mustek, g_strdup ("1000"));
  mustek = g_list_append (mustek, g_strdup ("1001"));

  g_hash_table_insert (scanners,
                       g_strdup("03f0"),
                       g_list_copy(mustek));

  GList* kodak = NULL;

  kodak = g_list_append (kodak, g_strdup ("6001"));
  kodak = g_list_append (kodak, g_strdup ("6002"));
  kodak = g_list_append (kodak, g_strdup ("6003"));
  kodak = g_list_append (kodak, g_strdup ("6004"));
  kodak = g_list_append (kodak, g_strdup ("6005"));

  g_hash_table_insert (scanners,
                       g_strdup("040a"),
                       g_list_copy(kodak));
                                              
  GList* creative = NULL;
  
  creative = g_list_append (creative, g_strdup ("4007"));

  g_hash_table_insert (scanners,
                       g_strdup("041e"),
                       g_list_copy(creative));
  
  GList* lexmark = NULL;
  
  lexmark = g_list_append (lexmark, g_strdup("002d"));
  lexmark = g_list_append (lexmark, g_strdup("0060"));
  lexmark = g_list_append (lexmark, g_strdup("007c"));
  lexmark = g_list_append (lexmark, g_strdup("007d"));

  g_hash_table_insert (scanners,
                       g_strdup("043d"),
                       g_list_copy(lexmark));
  

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
                       g_list_copy(genius));

  GList* medion = NULL;
  medion = g_list_append (medion, g_strdup("0377"));
  g_hash_table_insert (scanners,
                       g_strdup("0461"),
                       g_list_copy(medion));

  GList* trust = NULL;
  trust = g_list_append (trust, g_strdup("1000"));
  trust = g_list_append (trust, g_strdup("1002"));
  g_hash_table_insert (scanners,
                       g_strdup("047b"),
                       g_list_copy(trust));
                         
  GList* kyocera = NULL;
  kyocera = g_list_append (kyocera, g_strdup("0335"));
  g_hash_table_insert (scanners,
                       g_strdup("0482"),
                       g_list_copy(kyocera));
  
  GList* compaq = NULL;
  compaq = g_list_append (compaq, g_strdup("001a"));
  g_hash_table_insert (scanners,
                       g_strdup("049f"),
                       g_list_copy(compaq));
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
                       g_list_copy(benq));

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
                       g_list_copy(visioneer));
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
                       g_list_copy(canon));
  
  GList* nikon = NULL;
  nikon = g_list_append (nikon, g_strdup ("4000"));
  nikon = g_list_append (nikon, g_strdup ("4001"));
  nikon = g_list_append (nikon, g_strdup ("4002"));
  g_hash_table_insert (scanners,
                       g_strdup("04b0"),
                       g_list_copy(nikon));

  GList* epson = NULL;
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
                       g_list_copy(epson));

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
                       g_list_copy(fujitsu));
  GList* konica = NULL;
  konica = g_list_append (konica, g_strdup ("0722"));
  g_hash_table_insert (scanners,
                       g_strdup("04c8"),
                       g_list_copy(konica));
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
                       g_list_copy(panasonic));

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
                       g_list_copy(samsung));
  
  GList* pentax;
  pentax = g_list_append (pentax, g_strdup ("2038"));
  g_hash_table_insert (scanners,
                       g_strdup("04f9"),
                       g_list_copy(pentax));

  GList* apitek;
  apitek = g_list_append (apitek, g_strdup ("0202"));
  g_hash_table_insert (scanners,
                       g_strdup("0553"),
                       g_list_copy(apitek));

  GList* mustek;
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
                       g_list_copy(mustek));

# Artec/Ultima Ultima 2000 | Artec/Ultima Ultima 2000 e+ | Boeder Sm@rtScan Slim Edition
# Fujitsu 1200CUS | Googlegear 2000 | Medion/Lifetec/Tevion/Cytron MD 4394
# Medion/Lifetec/Tevion/Cytron MD/LT 9375 | Medion/Lifetec/Tevion/Cytron MD/LT 9385 | Medion/Lifetec/Tevion/Cytron LT 9452
# Medion/Lifetec/Tevion/Cytron MD 9458 | Mustek BearPaw 1200 CU | Mustek BearPaw 2400 CU
# Mustek ScanExpress 1200 UB Plus | Mustek ScanExpress 2400 USB | Mustek ScanMagic 1200 UB Plus
# Packard Bell Diamond 1200 | Trust Compact Scan USB 19200 | Trust Flat Scan USB 19200
ATTRS{idVendor}=="05d8", ATTRS{idProduct}=="4002", ENV{libsane_matched}="yes"
# Artec/Ultima E+ 48U | Medion/Lifetec/Tevion/Cytron MD9693 | Medion/Lifetec/Tevion/Cytron MD9705
# Medion/Lifetec/Tevion/Cytron MD4394 | Microstar MR 9791
ATTRS{idVendor}=="05d8", ATTRS{idProduct}=="4003", ENV{libsane_matched}="yes"
# Artec/Ultima E+ Pro
ATTRS{idVendor}=="05d8", ATTRS{idProduct}=="4004", ENV{libsane_matched}="yes"
# Memorex MEM 48U
ATTRS{idVendor}=="05d8", ATTRS{idProduct}=="4005", ENV{libsane_matched}="yes"
# Trust Easy Webscan 19200
ATTRS{idVendor}=="05d8", ATTRS{idProduct}=="4006", ENV{libsane_matched}="yes"
# Trust 240H Easy Webscan Gold
ATTRS{idVendor}=="05d8", ATTRS{idProduct}=="4007", ENV{libsane_matched}="yes"
# UMAX AstraSlim SE
ATTRS{idVendor}=="05d8", ATTRS{idProduct}=="4009", ENV{libsane_matched}="yes"
# UMAX AstraSlim 1200 SE
ATTRS{idVendor}=="05d8", ATTRS{idProduct}=="4010", ENV{libsane_matched}="yes"
# Yakumo Scan50
ATTRS{idVendor}=="05d8", ATTRS{idProduct}=="4011", ENV{libsane_matched}="yes"
# Microtek ScanMaker X6USB
ATTRS{idVendor}=="05da", ATTRS{idProduct}=="0099", ENV{libsane_matched}="yes"
# Microtek SlimScan C6
ATTRS{idVendor}=="05da", ATTRS{idProduct}=="009a", ENV{libsane_matched}="yes"
# Microtek ScanMaker V6USL
ATTRS{idVendor}=="05da", ATTRS{idProduct}=="00a3", ENV{libsane_matched}="yes"
# Microtek ScanMaker V6UPL
ATTRS{idVendor}=="05da", ATTRS{idProduct}=="00b6", ENV{libsane_matched}="yes"
# Microtek ScanMaker 4800
ATTRS{idVendor}=="05da", ATTRS{idProduct}=="30cf", ENV{libsane_matched}="yes"
# Microtek ScanMaker 3840
ATTRS{idVendor}=="05da", ATTRS{idProduct}=="30d4", ENV{libsane_matched}="yes"
# Microtek ScanMaker 3600
ATTRS{idVendor}=="05da", ATTRS{idProduct}=="40b3", ENV{libsane_matched}="yes"
# Microtek ScanMaker 3700
ATTRS{idVendor}=="05da", ATTRS{idProduct}=="40b8", ENV{libsane_matched}="yes"
# Microtek ScanMaker 3600
ATTRS{idVendor}=="05da", ATTRS{idProduct}=="40ca", ENV{libsane_matched}="yes"
# Microtek ScanMaker 3700
ATTRS{idVendor}=="05da", ATTRS{idProduct}=="40cb", ENV{libsane_matched}="yes"
# Microtek ScanMaker 3750
ATTRS{idVendor}=="05da", ATTRS{idProduct}=="40dd", ENV{libsane_matched}="yes"
# Microtek ScanMaker 3600
ATTRS{idVendor}=="05da", ATTRS{idProduct}=="40ff", ENV{libsane_matched}="yes"
# Microtek ScanMaker V6USL
ATTRS{idVendor}=="05da", ATTRS{idProduct}=="80a3", ENV{libsane_matched}="yes"
# iVina 1200U
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0268", ENV{libsane_matched}="yes"
# Minolta Dimage Scan Dual II
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="026a", ENV{libsane_matched}="yes"
# Avision AV600U
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a13", ENV{libsane_matched}="yes"
# Minolta-QMS SC-110
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a15", ENV{libsane_matched}="yes"
# Avision DS610CU Scancopier | Minolta-QMS SC-215 | OKI S700 Scancopier
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a16", ENV{libsane_matched}="yes"
# Avision AV600U Plus
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a18", ENV{libsane_matched}="yes"
# Avision AV610
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a19", ENV{libsane_matched}="yes"
# Avision AV220
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a23", ENV{libsane_matched}="yes"
# Avision AV210
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a24", ENV{libsane_matched}="yes"
# Avision AV210
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a25", ENV{libsane_matched}="yes"
# Avision AV120
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a27", ENV{libsane_matched}="yes"
# Avision AV220C2
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a2a", ENV{libsane_matched}="yes"
# Avision AV220D2
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a2b", ENV{libsane_matched}="yes"
# Avision AV220+
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a2c", ENV{libsane_matched}="yes"
# Avision AV220C2-G
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a2d", ENV{libsane_matched}="yes"
# Avision AV220C2-B
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a2e", ENV{libsane_matched}="yes"
# Avision AV210C2-G
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a2f", ENV{libsane_matched}="yes"
# Avision AV122
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a33", ENV{libsane_matched}="yes"
# Avision AV210C2
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a3a", ENV{libsane_matched}="yes"
# Avision AV121
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a3c", ENV{libsane_matched}="yes"
# Avision AV8300
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a40", ENV{libsane_matched}="yes"
# Avision AM3000 Series
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a41", ENV{libsane_matched}="yes"
# Avision @V5100
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a45", ENV{libsane_matched}="yes"
# Avision AV8050U
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a4d", ENV{libsane_matched}="yes"
# Avision AV3200SU
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a4e", ENV{libsane_matched}="yes"
# Avision AV3730SU
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a4f", ENV{libsane_matched}="yes"
# Avision AV610C2
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a5e", ENV{libsane_matched}="yes"
# Avision IT8300
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a61", ENV{libsane_matched}="yes"
# Avision AV3750SU
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a65", ENV{libsane_matched}="yes"
# Avision AV3850SU
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a66", ENV{libsane_matched}="yes"
# Avision AV8350
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a68", ENV{libsane_matched}="yes"
# Avision FB6080E
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a82", ENV{libsane_matched}="yes"
# Avision FB2080E
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a84", ENV{libsane_matched}="yes"
# Avision AV122 C2
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a93", ENV{libsane_matched}="yes"
# Avision AV220-G
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0a94", ENV{libsane_matched}="yes"
# Avision @V2500
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="0aa1", ENV{libsane_matched}="yes"
# Avision AV210D2+
ATTRS{idVendor}=="0638", ATTRS{idProduct}=="1a35", ENV{libsane_matched}="yes"
# Minolta Elite II
ATTRS{idVendor}=="0686", ATTRS{idProduct}=="4004", ENV{libsane_matched}="yes"
# Minolta Dimage Scan Dual III
ATTRS{idVendor}=="0686", ATTRS{idProduct}=="400d", ENV{libsane_matched}="yes"
# Minolta Dimage Scan Elite 5400
ATTRS{idVendor}=="0686", ATTRS{idProduct}=="400e", ENV{libsane_matched}="yes"
# AGFA SnapScan 1212U
ATTRS{idVendor}=="06bd", ATTRS{idProduct}=="0001", ENV{libsane_matched}="yes"
# AGFA SnapScan 1236u
ATTRS{idVendor}=="06bd", ATTRS{idProduct}=="0002", ENV{libsane_matched}="yes"
# Agfa Snapscan Touch
ATTRS{idVendor}=="06bd", ATTRS{idProduct}=="0100", ENV{libsane_matched}="yes"
# AGFA SnapScan 1212U_2
ATTRS{idVendor}=="06bd", ATTRS{idProduct}=="2061", ENV{libsane_matched}="yes"
# AGFA SnapScan e40
ATTRS{idVendor}=="06bd", ATTRS{idProduct}=="208d", ENV{libsane_matched}="yes"
# AGFA SnapScan e50
ATTRS{idVendor}=="06bd", ATTRS{idProduct}=="208f", ENV{libsane_matched}="yes"
# AGFA SnapScan e20
ATTRS{idVendor}=="06bd", ATTRS{idProduct}=="2091", ENV{libsane_matched}="yes"
# AGFA SnapScan e10
ATTRS{idVendor}=="06bd", ATTRS{idProduct}=="2093", ENV{libsane_matched}="yes"
# AGFA SnapScan e25
ATTRS{idVendor}=="06bd", ATTRS{idProduct}=="2095", ENV{libsane_matched}="yes"
# AGFA SnapScan e26
ATTRS{idVendor}=="06bd", ATTRS{idProduct}=="2097", ENV{libsane_matched}="yes"
# AGFA SnapScan e52
ATTRS{idVendor}=="06bd", ATTRS{idProduct}=="20fd", ENV{libsane_matched}="yes"
# AGFA SnapScan e42
ATTRS{idVendor}=="06bd", ATTRS{idProduct}=="20ff", ENV{libsane_matched}="yes"
# UMAX Astra 4900
ATTRS{idVendor}=="06dc", ATTRS{idProduct}=="0020", ENV{libsane_matched}="yes"
# Plustek OpticPro U12 | Plustek OpticPro UT12 | Plustek OpticPro 1212U
# RevScan RevScan Orange R48Ti | Genius ColorPage Vivid III USB
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="0001", ENV{libsane_matched}="yes"
# Plustek OpticPro U12
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="0010", ENV{libsane_matched}="yes"
# Plustek OpticPro U24
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="0011", ENV{libsane_matched}="yes"
# Plustek OpticPro UT12
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="0013", ENV{libsane_matched}="yes"
# Plustek OpticPro U24
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="0015", ENV{libsane_matched}="yes"
# Plustek OpticPro UT12 | Plustek OpticPro UT16 | Plustek OpticPro UT24
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="0017", ENV{libsane_matched}="yes"
# Plustek OpticPro 1248U | RevScan 19200i
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="0400", ENV{libsane_matched}="yes"
# Plustek OpticPro 1248U
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="0401", ENV{libsane_matched}="yes"
# Plustek OpticPro U16B
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="0402", ENV{libsane_matched}="yes"
# Plustek OpticPro U16B+ | Plustek OpticPro UT16B
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="0403", ENV{libsane_matched}="yes"
# Nortek MyScan 1200 | Plustek OpticPro S12 | Plustek OpticPro ST12
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="040b", ENV{libsane_matched}="yes"
# Plustek OpticPro S24
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="040e", ENV{libsane_matched}="yes"
# NeatReceipts Scanalizer Professional 2.5 | Plustek OpticSlim M12
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="0412", ENV{libsane_matched}="yes"
# Plustek OpticSlim 1200
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="0413", ENV{libsane_matched}="yes"
# Plustek OpticSlim 2400
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="0422", ENV{libsane_matched}="yes"
# Plustek OpticSlim 2400 plus
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="0454", ENV{libsane_matched}="yes"
# Plustek Iriscan Express 2
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="045f", ENV{libsane_matched}="yes"
# NeatReceipts Mobile Scanner
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="0462", ENV{libsane_matched}="yes"
# Plustek OpticBook 3600
ATTRS{idVendor}=="07b3", ATTRS{idProduct}=="0900", ENV{libsane_matched}="yes"
# Corex 600c
ATTRS{idVendor}=="08f0", ATTRS{idProduct}=="0002", ENV{libsane_matched}="yes"
# Corex 800c
ATTRS{idVendor}=="08f0", ATTRS{idProduct}=="0005", ENV{libsane_matched}="yes"
# Xerox Phaser 6110MFP
ATTRS{idVendor}=="0924", ATTRS{idProduct}=="3d5d", ENV{libsane_matched}="yes"
# Xerox Phaser 3200MFP
ATTRS{idVendor}=="0924", ATTRS{idProduct}=="3da4", ENV{libsane_matched}="yes"
# Xerox WorkCentre 4118 Series
ATTRS{idVendor}=="0924", ATTRS{idProduct}=="420c", ENV{libsane_matched}="yes"
# Xerox WorkCentre 3119 Series
ATTRS{idVendor}=="0924", ATTRS{idProduct}=="4265", ENV{libsane_matched}="yes"
# Xerox WorkCentre 3210
ATTRS{idVendor}=="0924", ATTRS{idProduct}=="4293", ENV{libsane_matched}="yes"
# Xerox WorkCentre 3220
ATTRS{idVendor}=="0924", ATTRS{idProduct}=="4294", ENV{libsane_matched}="yes"
# Pentax DSmobile 600
ATTRS{idVendor}=="0a17", ATTRS{idProduct}=="3210", ENV{libsane_matched}="yes"
# Portable Peripheral Co., Ltd. Q-Scan USB001 (A4 portable scanner)
ATTRS{idVendor}=="0a53", ATTRS{idProduct}=="1000", ENV{libsane_matched}="yes"
# Syscan TravelScan 460/464 | Ambir Visigo A4
ATTRS{idVendor}=="0a82", ATTRS{idProduct}=="4600", ENV{libsane_matched}="yes"
# Syscan DocketPort 465
ATTRS{idVendor}=="0a82", ATTRS{idProduct}=="4802", ENV{libsane_matched}="yes"
# Syscan DocketPort 665
ATTRS{idVendor}=="0a82", ATTRS{idProduct}=="4803", ENV{libsane_matched}="yes"
# Syscan DocketPort 685/ Ambir DS685
ATTRS{idVendor}=="0a82", ATTRS{idProduct}=="480c", ENV{libsane_matched}="yes"
# Syscan DocketPort 485
ATTRS{idVendor}=="0a82", ATTRS{idProduct}=="4810", ENV{libsane_matched}="yes"
# Syscan TravelScan 662
ATTRS{idVendor}=="0a82", ATTRS{idProduct}=="6620", ENV{libsane_matched}="yes"
# Canon CR-55
ATTRS{idVendor}=="1083", ATTRS{idProduct}=="160c", ENV{libsane_matched}="yes"
# Canon DR-1210C
ATTRS{idVendor}=="1083", ATTRS{idProduct}=="160f", ENV{libsane_matched}="yes"
# Canon DR-4010C
ATTRS{idVendor}=="1083", ATTRS{idProduct}=="1614", ENV{libsane_matched}="yes"
# Canon DR-2510C
ATTRS{idVendor}=="1083", ATTRS{idProduct}=="1617", ENV{libsane_matched}="yes"
# Canon DR-X10C
ATTRS{idVendor}=="1083", ATTRS{idProduct}=="1618", ENV{libsane_matched}="yes"
# Canon CR-25
ATTRS{idVendor}=="1083", ATTRS{idProduct}=="161a", ENV{libsane_matched}="yes"
# Canon DR-2010C
ATTRS{idVendor}=="1083", ATTRS{idProduct}=="161b", ENV{libsane_matched}="yes"
# Canon DR-3010C
ATTRS{idVendor}=="1083", ATTRS{idProduct}=="161d", ENV{libsane_matched}="yes"
# Canon DR-7090C
ATTRS{idVendor}=="1083", ATTRS{idProduct}=="1620", ENV{libsane_matched}="yes"
# Canon DR-9050C
ATTRS{idVendor}=="1083", ATTRS{idProduct}=="1622", ENV{libsane_matched}="yes"
# Canon DR-7550C
ATTRS{idVendor}=="1083", ATTRS{idProduct}=="1623", ENV{libsane_matched}="yes"
# Canon DR-6050C
ATTRS{idVendor}=="1083", ATTRS{idProduct}=="1624", ENV{libsane_matched}="yes"
# Canon DR-6010C
ATTRS{idVendor}=="1083", ATTRS{idProduct}=="1626", ENV{libsane_matched}="yes"
# Canon CR-190i
ATTRS{idVendor}=="1083", ATTRS{idProduct}=="162b", ENV{libsane_matched}="yes"
# Canon DR-6030C
ATTRS{idVendor}=="1083", ATTRS{idProduct}=="1638", ENV{libsane_matched}="yes"
# Canon CR-135i
ATTRS{idVendor}=="1083", ATTRS{idProduct}=="1639", ENV{libsane_matched}="yes"
# Digital Dream l' espion XS
ATTRS{idVendor}=="1183", ATTRS{idProduct}=="0001", ENV{libsane_matched}="yes"
# KONICA MINOLTA magicolor 1690MF
ATTRS{idVendor}=="132b", ATTRS{idProduct}=="2089", ENV{libsane_matched}="yes"
# UMAX Astra 1220U
ATTRS{idVendor}=="1606", ATTRS{idProduct}=="0010", ENV{libsane_matched}="yes"
# UMAX Astra 1600U | UMAX Astra 2000U
ATTRS{idVendor}=="1606", ATTRS{idProduct}=="0030", ENV{libsane_matched}="yes"
# Umax UMAX 3400
ATTRS{idVendor}=="1606", ATTRS{idProduct}=="0050", ENV{libsane_matched}="yes"
# Umax UMAX 3400 | Umax UMAX Astranet ia101 | Umax UMAX 3450
ATTRS{idVendor}=="1606", ATTRS{idProduct}=="0060", ENV{libsane_matched}="yes"
# UMAX Astra 4400 | UMAX Astra 4450
ATTRS{idVendor}=="1606", ATTRS{idProduct}=="0070", ENV{libsane_matched}="yes"
# UMAX Astra 2100U
ATTRS{idVendor}=="1606", ATTRS{idProduct}=="0130", ENV{libsane_matched}="yes"
# Umax UMAX 5400
ATTRS{idVendor}=="1606", ATTRS{idProduct}=="0160", ENV{libsane_matched}="yes"
# UMAX Astra 2200 (SU)
ATTRS{idVendor}=="1606", ATTRS{idProduct}=="0230", ENV{libsane_matched}="yes"
# DCT DocketPort 487
ATTRS{idVendor}=="1dcc", ATTRS{idProduct}=="4810", ENV{libsane_matched}="yes"
# Dell A920
ATTRS{idVendor}=="413c", ATTRS{idProduct}=="5105", ENV{libsane_matched}="yes"
# Dell Dell MFP Laser Printer 1815dn
ATTRS{idVendor}=="413c", ATTRS{idProduct}=="5124", ENV{libsane_matched}="yes"
# Dell 1600n
ATTRS{idVendor}=="413c", ATTRS{idProduct}=="5250", ENV{libsane_matched}="yes"
  
}
