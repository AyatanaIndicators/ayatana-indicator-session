
void populate_usb_scanners (GHashTable* scanners)
{
  GList* epson = NULL;
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

# Canon DR-2080C
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1601", ENV{libsane_matched}="yes"
# Canon CR-180 | Canon CR-180II
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1602", ENV{libsane_matched}="yes"
# Canon DR-9080C
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1603", ENV{libsane_matched}="yes"
# Canon DR-7080C
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1604", ENV{libsane_matched}="yes"
# Canon DR-5010C
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1606", ENV{libsane_matched}="yes"
# Canon DR-6080
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1607", ENV{libsane_matched}="yes"
# Canon DR-2580C
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1608", ENV{libsane_matched}="yes"
# Canon DR-3080CII
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1609", ENV{libsane_matched}="yes"
# Canon DR-2050C | Canon DR-2050SP
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="160a", ENV{libsane_matched}="yes"
# Canon DR-7580
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="160b", ENV{libsane_matched}="yes"
# Canon PIXMA MP750
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1706", ENV{libsane_matched}="yes"
# Canon PIXMA MP780
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1707", ENV{libsane_matched}="yes"
# Canon PIXMA MP760
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1708", ENV{libsane_matched}="yes"
# Canon PIXMA MP150
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1709", ENV{libsane_matched}="yes"
# Canon PIXMA MP170
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="170a", ENV{libsane_matched}="yes"
# Canon PIXMA MP450
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="170b", ENV{libsane_matched}="yes"
# Canon PIXMA MP500
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="170c", ENV{libsane_matched}="yes"
# Canon PIXMA MP800
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="170d", ENV{libsane_matched}="yes"
# Canon PIXMA MP800R
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="170e", ENV{libsane_matched}="yes"
# Canon PIXMA MP530
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1712", ENV{libsane_matched}="yes"
# Canon PIXMA MP830
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1713", ENV{libsane_matched}="yes"
# Canon PIXMA MP160
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1714", ENV{libsane_matched}="yes"
# Canon PIXMA MP180
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1715", ENV{libsane_matched}="yes"
# Canon PIXMA MP460
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1716", ENV{libsane_matched}="yes"
# Canon PIXMA MP510
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1717", ENV{libsane_matched}="yes"
# Canon PIXMA MP600
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1718", ENV{libsane_matched}="yes"
# Canon PIXMA MP600R
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1719", ENV{libsane_matched}="yes"
# Canon PIXMA MP810
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="171a", ENV{libsane_matched}="yes"
# Canon PIXMA MP960
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="171b", ENV{libsane_matched}="yes"
# Canon PIXMA MX7600
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="171c", ENV{libsane_matched}="yes"
# Canon PIXMA MP210
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1721", ENV{libsane_matched}="yes"
# Canon PIXMA MP220
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1722", ENV{libsane_matched}="yes"
# Canon PIXMA MP470
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1723", ENV{libsane_matched}="yes"
# Canon PIXMA MP520
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1724", ENV{libsane_matched}="yes"
# Canon PIXMA MP610
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1725", ENV{libsane_matched}="yes"
# Canon PIXMA MP970
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1726", ENV{libsane_matched}="yes"
# Canon PIXMA MX300
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1727", ENV{libsane_matched}="yes"
# Canon PIXMA MX310
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1728", ENV{libsane_matched}="yes"
# Canon PIXMA MX700
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1729", ENV{libsane_matched}="yes"
# Canon PIXMA MP140
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="172b", ENV{libsane_matched}="yes"
# Canon PIXMA MX850
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="172c", ENV{libsane_matched}="yes"
# Canon PIXMA MP980
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="172d", ENV{libsane_matched}="yes"
# Canon PIXMA MP630
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="172e", ENV{libsane_matched}="yes"
# Canon PIXMA MP620
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="172f", ENV{libsane_matched}="yes"
# Canon PIXMA MP540
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1730", ENV{libsane_matched}="yes"
# Canon PIXMA MP480
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1731", ENV{libsane_matched}="yes"
# Canon PIXMA MP240
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1732", ENV{libsane_matched}="yes"
# Canon PIXMA MP260
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1733", ENV{libsane_matched}="yes"
# Canon PIXMA MP190
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1734", ENV{libsane_matched}="yes"
# Canon PIXMA MX860
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1735", ENV{libsane_matched}="yes"
# Canon PIXMA MX320
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1736", ENV{libsane_matched}="yes"
# Canon PIXMA MX330
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1737", ENV{libsane_matched}="yes"
# Canon PIXMA MP250
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="173a", ENV{libsane_matched}="yes"
# Canon PIXMA MP270
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="173b", ENV{libsane_matched}="yes"
# Canon PIXMA MP490
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="173c", ENV{libsane_matched}="yes"
# Canon PIXMA MP550
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="173d", ENV{libsane_matched}="yes"
# Canon PIXMA MP560
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="173e", ENV{libsane_matched}="yes"
# Canon PIXMA MP640
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="173f", ENV{libsane_matched}="yes"
# Canon PIXMA MP990
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1740", ENV{libsane_matched}="yes"
# Canon PIXMA MX340
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1741", ENV{libsane_matched}="yes"
# Canon PIXMA MX350
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1742", ENV{libsane_matched}="yes"
# Canon PIXMA MX870
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1743", ENV{libsane_matched}="yes"
# Canon CanoScan 8800F
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1901", ENV{libsane_matched}="yes"
# Canon CanoScan LiDE 100
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1904", ENV{libsane_matched}="yes"
# Canon CanoScan LiDE 200
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1905", ENV{libsane_matched}="yes"
# Canon CanoScan LiDE 110
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="1909", ENV{libsane_matched}="yes"
# Canon CanoScan LiDE 210
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="190a", ENV{libsane_matched}="yes"
# Canon CanoScan fb630u | Canon CanoScan fb636u
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="2204", ENV{libsane_matched}="yes"
# Canon CanoScan N650U/N656U
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="2206", ENV{libsane_matched}="yes"
# Canon CanoScan N1220U
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="2207", ENV{libsane_matched}="yes"
# Canon CanoScan D660U
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="2208", ENV{libsane_matched}="yes"
# Canon CanoScan N670U/N676U/LiDE20
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="220d", ENV{libsane_matched}="yes"
# Canon CanoScan N1240U/LiDE30
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="220e", ENV{libsane_matched}="yes"
# Canon CanoScan LiDE 35 | Canon CanoScan LiDE 40 | Canon CanoScan LiDE 50
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="2213", ENV{libsane_matched}="yes"
# Canon CanoScan LiDE 60
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="221c", ENV{libsane_matched}="yes"
# Canon CanoScan LiDE25
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="2220", ENV{libsane_matched}="yes"
# Canon DR-1210C
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="2222", ENV{libsane_matched}="yes"
# Canon PIXMA MP730
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="262f", ENV{libsane_matched}="yes"
# Canon PIXMA MP700
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="2630", ENV{libsane_matched}="yes"
# Canon PIXMA MP360
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="263c", ENV{libsane_matched}="yes"
# Canon PIXMA MP370
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="263d", ENV{libsane_matched}="yes"
# Canon PIXMA MP390
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="263e", ENV{libsane_matched}="yes"
# Canon PIXMA MP375R
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="263f", ENV{libsane_matched}="yes"
# Canon PIXMA MP740
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="264c", ENV{libsane_matched}="yes"
# Canon PIXMA MP710
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="264d", ENV{libsane_matched}="yes"
# Canon imageCLASS MF5630
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="264e", ENV{libsane_matched}="yes"
# Canon laserBase MF5650
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="264f", ENV{libsane_matched}="yes"
# Canon imageCLASS MF8170c
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="2659", ENV{libsane_matched}="yes"
# Canon imageCLASS MF5730
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="265d", ENV{libsane_matched}="yes"
# Canon imageCLASS MF5750
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="265e", ENV{libsane_matched}="yes"
# Canon imageCLASS MF5770
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="265f", ENV{libsane_matched}="yes"
# Canon imageCLASS MF3110
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="2660", ENV{libsane_matched}="yes"
# Canon imageCLASS MF3240
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="2684", ENV{libsane_matched}="yes"
# Canon imageCLASS MF6500 series
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="2686", ENV{libsane_matched}="yes"
# Canon imageCLASS MF4120 | Canon imageCLASS MF4122 | Canon imageCLASS MF4140
# Canon imageCLASS MF4150
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="26a3", ENV{libsane_matched}="yes"
# Canon imageCLASS MF4690
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="26b0", ENV{libsane_matched}="yes"
# Canon imageCLASS MF4010 | Canon imageCLASS MF4018
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="26b4", ENV{libsane_matched}="yes"
# Canon imageCLASS MF4270
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="26b5", ENV{libsane_matched}="yes"
# Canon imageCLASS MF4370dn | Canon imageCLASS MF4380dn
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="26ec", ENV{libsane_matched}="yes"
# Canon imageCLASS D480
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="26ed", ENV{libsane_matched}="yes"
# Canon I-SENSYS MF4320d | Canon I-SENSYS MF4330d | Canon imageCLASS MF4350d
ATTRS{idVendor}=="04a9", ATTRS{idProduct}=="26ee", ENV{libsane_matched}="yes"
# Nikon LS 40 ED | Nikon LS 40 ED | Nikon Coolspan IV
ATTRS{idVendor}=="04b0", ATTRS{idProduct}=="4000", ENV{libsane_matched}="yes"
# Nikon LS 50 ED | Nikon Coolscan V ED | Nikon LS 50 ED
# Nikon Coolscan V ED
ATTRS{idVendor}=="04b0", ATTRS{idProduct}=="4001", ENV{libsane_matched}="yes"
# Nikon Super Coolscan LS-5000 ED | Nikon Super Coolscan LS-5000 ED
ATTRS{idVendor}=="04b0", ATTRS{idProduct}=="4002", ENV{libsane_matched}="yes"
# Epson Perfection 636U | Epson GT-7000U | Epson Perfection 636U
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0101", ENV{libsane_matched}="yes"
# Epson Perfection 610 | Epson GT-6600U | Epson Perfection 610
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0103", ENV{libsane_matched}="yes"
# Epson Perfection 1200U | Epson Perfection 1200Photo | Epson GT-7600U
# Epson GT-7600UF | Epson Perfection 1200U | Epson Perfection 1200U PHOTO
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0104", ENV{libsane_matched}="yes"
# Epson Stylus Scan 2000
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0105", ENV{libsane_matched}="yes"
# Epson Stylus Scan 2500
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0106", ENV{libsane_matched}="yes"
# Epson Expression 1600 | Epson ES-2000 | Epson Expression 1600
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0107", ENV{libsane_matched}="yes"
# Epson ES-8500 | Epson Expression 1640XL
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0109", ENV{libsane_matched}="yes"
# Epson Perfection 1640 | Epson GT-8700 | Epson GT-8700F
# Epson Perfection 1640SU | Epson Perfection 1640SU PHOTO
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="010a", ENV{libsane_matched}="yes"
# Epson Perfection 1240 | Epson GT-7700U | Epson Perfection 1240U
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="010b", ENV{libsane_matched}="yes"
# Epson Perfection 640 | Epson GT-6700U | Epson Perfection 640U
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="010c", ENV{libsane_matched}="yes"
# Epson Expression 1680 | Epson ES-2200 | Epson Expression 1680
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="010e", ENV{libsane_matched}="yes"
# Epson Perfection 1250 | Epson Perfection 1250Photo
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="010f", ENV{libsane_matched}="yes"
# Epson Perfection 1650 | Epson GT-8200U | Epson GT-8200UF
# Epson Perfection 1650 | Epson Perfection 1650 PHOTO
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0110", ENV{libsane_matched}="yes"
# Epson Perfection 2450 | Epson GT-9700F | Epson Perfection 2450 PHOTO
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0112", ENV{libsane_matched}="yes"
# Epson Perfection 660
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0114", ENV{libsane_matched}="yes"
# Epson Perfection 2400 | Epson GT-9300UF | Epson Perfection 2400 PHOTO
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="011b", ENV{libsane_matched}="yes"
# Epson Perfection 3200 | Epson GT-9800F | Epson Perfection 3200 PHOTO
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="011c", ENV{libsane_matched}="yes"
# Epson Perfection 1260 | Epson Perfection 1260Photo
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="011d", ENV{libsane_matched}="yes"
# Epson Perfection 1660 | Epson GT-8300UF | Epson Perfection 1660 PHOTO
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="011e", ENV{libsane_matched}="yes"
# Epson Perfection 1670
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="011f", ENV{libsane_matched}="yes"
# Epson Perfection 1270
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0120", ENV{libsane_matched}="yes"
# Epson Perfection 2480 | Epson Perfection 2580
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0121", ENV{libsane_matched}="yes"
# Epson Perfection 3490 | Epson Perfection 3590
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0122", ENV{libsane_matched}="yes"
# Epson ES-7000H | Epson GT-15000
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0126", ENV{libsane_matched}="yes"
# Epson Perfection 4870 | Epson GT-X700 | Epson Perfection 4870 PHOTO
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0128", ENV{libsane_matched}="yes"
# Epson ES-10000G | Epson Expression 10000XL
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0129", ENV{libsane_matched}="yes"
# Epson Perfection 4990 | Epson GT-X800 | Epson Perfection 4990 PHOTO
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="012a", ENV{libsane_matched}="yes"
# Epson ES-H300 | Epson GT-2500
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="012b", ENV{libsane_matched}="yes"
# Epson V700 | Epson V750 | Epson GT-X900
# Epson Perfection V700 Photo | Epson Perfection V750 Photo
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="012c", ENV{libsane_matched}="yes"
# Epson GT-X970
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0135", ENV{libsane_matched}="yes"
# Epson CX-5200 | Epson CX-5400 | Epson CC-600PX
# Epson Stylus CX5100 | Epson Stylus CX5200
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0801", ENV{libsane_matched}="yes"
# Epson CX-3200 | Epson CC-570L | Epson Stylus CX3100
# Epson Stylus CX3200
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0802", ENV{libsane_matched}="yes"
# Epson CX-6300 | Epson CX-6400 | Epson Stylus CX6300
# Epson Stylus CX6400
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0805", ENV{libsane_matched}="yes"
# Epson RX-600 | Epson PM-A850 | Epson Stylus Photo RX600
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0806", ENV{libsane_matched}="yes"
# Epson RX-500 | Epson Stylus Photo RX500 | Epson Stylus Photo RX510
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0807", ENV{libsane_matched}="yes"
# Epson CX-5400 | Epson Stylus CX5300 | Epson Stylus CX5400
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0808", ENV{libsane_matched}="yes"
# Epson Stylus CX-1500
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="080c", ENV{libsane_matched}="yes"
# Epson CX-4600 | Epson Stylus CX4500 | Epson Stylus CX4600
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="080d", ENV{libsane_matched}="yes"
# Epson CX-3600 | Epson CX-3650 | Epson PX-A550
# Epson Stylus CX3500 | Epson Stylus CX3600 | Epson Stylus CX3650
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="080e", ENV{libsane_matched}="yes"
# Epson RX-425 | Epson Stylus Photo RX420 | Epson Stylus Photo RX425
# Epson Stylus Photo RX430
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="080f", ENV{libsane_matched}="yes"
# Epson RX-700 | Epson PM-A900 | Epson Stylus Photo RX700
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0810", ENV{libsane_matched}="yes"
# Epson RX-620 | Epson PM-A870 | Epson Stylus Photo RX620
# Epson Stylus Photo RX630
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0811", ENV{libsane_matched}="yes"
# Epson CX-6500 | Epson CX-6600 | Epson Stylus CX6500
# Epson Stylus CX6600
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0813", ENV{libsane_matched}="yes"
# Epson PM-A700
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0814", ENV{libsane_matched}="yes"
# Epson AcuLaser CX11 | Epson AcuLaser CX11NF | Epson AcuLaser CX11
# Epson AcuLaser CX11NF | Epson LP-A500
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0815", ENV{libsane_matched}="yes"
# Epson LP-M5500 | Epson LP-M5500F
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0817", ENV{libsane_matched}="yes"
# Epson DX-3850 | Epson CX-3700 | Epson CX-3800
# Epson DX-3800 | Epson Stylus CX3700 | Epson Stylus CX3800
# Epson Stylus DX3800
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0818", ENV{libsane_matched}="yes"
# Epson CX-4800 | Epson PX-A650 | Epson Stylus CX4700
# Epson Stylus CX4800 | Epson Stylus DX4800 | Epson Stylus DX4850
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0819", ENV{libsane_matched}="yes"
# Epson PM-A750 | Epson Stylus Photo RX520 | Epson Stylus Photo RX530
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="081a", ENV{libsane_matched}="yes"
# Epson PM-A890 | Epson Stylus Photo RX640 | Epson Stylus Photo RX650
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="081c", ENV{libsane_matched}="yes"
# Epson PM-A950
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="081d", ENV{libsane_matched}="yes"
# Epson Stylus CX7700 | Epson Stylus CX7800
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="081f", ENV{libsane_matched}="yes"
# Epson CX-4200 | Epson Stylus CX4100 | Epson Stylus CX4200
# Epson Stylus DX4200
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0820", ENV{libsane_matched}="yes"
# Epson PM-A820 | Epson Stylus Photo RX560 | Epson Stylus Photo RX580
# Epson Stylus Photo RX590
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0827", ENV{libsane_matched}="yes"
# Epson PM-A970
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0828", ENV{libsane_matched}="yes"
# Epson PM-T990
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0829", ENV{libsane_matched}="yes"
# Epson PM-A920
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="082a", ENV{libsane_matched}="yes"
# Epson CX-5000 | Epson DX-5000 | Epson DX-5050
# Epson Stylus CX4900 | Epson Stylus CX5000 | Epson Stylus DX5000
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="082b", ENV{libsane_matched}="yes"
# Epson DX-6000 | Epson PX-A720 | Epson Stylus CX5900
# Epson Stylus CX6000 | Epson Stylus DX6000
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="082e", ENV{libsane_matched}="yes"
# Epson DX-4050 | Epson PX-A620 | Epson Stylus CX3900
# Epson Stylus DX4000
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="082f", ENV{libsane_matched}="yes"
# Epson ME 200 | Epson Stylus CX2800 | Epson Stylus CX2900
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0830", ENV{libsane_matched}="yes"
# Epson LP-M5600
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0833", ENV{libsane_matched}="yes"
# Epson LP-M6000
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0834", ENV{libsane_matched}="yes"
# Epson AcuLaser CX21
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0835", ENV{libsane_matched}="yes"
# Epson PM-T960
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0836", ENV{libsane_matched}="yes"
# Epson PM-A940 | Epson Stylus Photo RX680 | Epson Stylus Photo RX685
# Epson Stylus Photo RX690
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0837", ENV{libsane_matched}="yes"
# Epson DX-7400 | Epson PX-A640 | Epson Stylus CX7300
# Epson Stylus CX7400 | Epson Stylus DX7400
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0838", ENV{libsane_matched}="yes"
# Epson PX-A740 | Epson Stylus CX8300 | Epson Stylus CX8400
# Epson Stylus DX8400
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0839", ENV{libsane_matched}="yes"
# Epson PX-FA700 | Epson Stylus CX9300F | Epson Stylus CX9400Fax
# Epson Stylus DX9400F
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="083a", ENV{libsane_matched}="yes"
# Epson PM-A840 | Epson PM-A840S | Epson Stylus Photo RX585
# Epson Stylus Photo RX595 | Epson Stylus Photo RX610
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="083c", ENV{libsane_matched}="yes"
# Epson ME 300 | Epson PX-401A | Epson Stylus NX100
# Epson Stylus SX100 | Epson Stylus TX100
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0841", ENV{libsane_matched}="yes"
# Epson LP-M5000
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0843", ENV{libsane_matched}="yes"
# Epson Artisan 800 | Epson EP-901A | Epson EP-901F
# Epson Stylus Photo PX800FW | Epson Stylus Photo TX800FW
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0844", ENV{libsane_matched}="yes"
# Epson Artisan 700 | Epson EP-801A | Epson Stylus Photo PX700W
# Epson Stylus Photo TX700W
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0846", ENV{libsane_matched}="yes"
# Epson ME Office 700FW | Epson PX-601F | Epson Stylus Office BX600FW
# Epson Stylus Office TX600FW | Epson Stylus SX600FW | Epson WorkForce 600
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0847", ENV{libsane_matched}="yes"
# Epson ME Office 600F | Epson Stylus Office BX300F | Epson Stylus Office TX300F
# Epson Stylus NX300
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0848", ENV{libsane_matched}="yes"
# Epson Stylus NX200 | Epson Stylus SX200 | Epson Stylus SX205
# Epson Stylus TX200 | Epson Stylus TX203 | Epson Stylus TX209
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0849", ENV{libsane_matched}="yes"
# Epson PX-501A | Epson Stylus NX400 | Epson Stylus SX400
# Epson Stylus SX405 | Epson Stylus TX400
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="084a", ENV{libsane_matched}="yes"
# Epson WorkForce 500
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="084c", ENV{libsane_matched}="yes"
# Epson PX-402A | Epson Stylus NX110 Series | Epson Stylus SX110 Series
# Epson Stylus TX110 Series
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="084d", ENV{libsane_matched}="yes"
# Epson ME OFFICE 510 | Epson Stylus NX210 Series | Epson Stylus SX210 Series
# Epson Stylus TX210 Series
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="084f", ENV{libsane_matched}="yes"
# Epson Stylus NX410 Series | Epson Stylus SX410 Series | Epson Stylus TX410 Series
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0851", ENV{libsane_matched}="yes"
# Epson ME OFFICE 650FN Series | Epson Stylus Office BX310FN Series | Epson Stylus Office TX510FN Series
# Epson WorkForce 310 Series
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0854", ENV{libsane_matched}="yes"
# Epson PX-502A | Epson Stylus NX510 Series | Epson Stylus SX510W Series
# Epson Stylus TX550W Series
ATTRS{idVendor}=="04b8", ATTRS{idProduct}=="0856", ENV{libsane_matched}="yes"
# Fujitsu fi-4010CU
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="1029", ENV{libsane_matched}="yes"
# Fujitsu fi-4120C
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="1041", ENV{libsane_matched}="yes"
# Fujitsu fi-4220C
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="1042", ENV{libsane_matched}="yes"
# Fujitsu fi-4530C
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="1078", ENV{libsane_matched}="yes"
# Fujitsu fi-5750C
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="1095", ENV{libsane_matched}="yes"
# Fujitsu fi-5110EOX/2
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="1096", ENV{libsane_matched}="yes"
# Fujitsu fi-5110C
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="1097", ENV{libsane_matched}="yes"
# Fujitsu fi-5650C
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="10ad", ENV{libsane_matched}="yes"
# Fujitsu fi-4120C2
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="10ae", ENV{libsane_matched}="yes"
# Fujitsu fi-4220C2
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="10af", ENV{libsane_matched}="yes"
# Fujitsu fi-60F
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="10c7", ENV{libsane_matched}="yes"
# Fujitsu fi-4340C
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="10cf", ENV{libsane_matched}="yes"
# Fujitsu fi-5120C
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="10e0", ENV{libsane_matched}="yes"
# Fujitsu fi-5220C
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="10e1", ENV{libsane_matched}="yes"
# Fujitsu fi-5530C
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="10e2", ENV{libsane_matched}="yes"
# Fujitsu fi-5110EOX3
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="10e6", ENV{libsane_matched}="yes"
# Fujitsu fi-5900C
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="10e7", ENV{libsane_matched}="yes"
# Fujitsu fi-5015C
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="10ef", ENV{libsane_matched}="yes"
# Fujitsu fi-5110EOXM
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="10f2", ENV{libsane_matched}="yes"
# Fujitsu ScanSnap S500
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="10fe", ENV{libsane_matched}="yes"
# Fujitsu ScanSnap S500M
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="1135", ENV{libsane_matched}="yes"
# Fujitsu fi-5530C2
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="114a", ENV{libsane_matched}="yes"
# Fujitsu fi-6140
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="114d", ENV{libsane_matched}="yes"
# Fujitsu fi-6240
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="114e", ENV{libsane_matched}="yes"
# Fujitsu fi-6130
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="114f", ENV{libsane_matched}="yes"
# Fujitsu fi-6230
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="1150", ENV{libsane_matched}="yes"
# Fujitsu ScanSnap S510
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="1155", ENV{libsane_matched}="yes"
# Fujitsu ScanSnap S300
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="1156", ENV{libsane_matched}="yes"
# Fujitsu ScanSnap S510M
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="116f", ENV{libsane_matched}="yes"
# Fujitsu fi-6770
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="1174", ENV{libsane_matched}="yes"
# Fujitsu fi-6770A
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="1175", ENV{libsane_matched}="yes"
# Fujitsu fi-6670
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="1176", ENV{libsane_matched}="yes"
# Fujitsu fi-6670A
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="1177", ENV{libsane_matched}="yes"
# Fujitsu fi-6750S
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="1178", ENV{libsane_matched}="yes"
# Fujitsu ScanSnap S300M
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="117f", ENV{libsane_matched}="yes"
# Fujitsu fi-6800
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="119d", ENV{libsane_matched}="yes"
# Fujitsu fi-6800-CGA
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="119e", ENV{libsane_matched}="yes"
# Fujitsu fi-6900
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="119f", ENV{libsane_matched}="yes"
# Fujitsu fi-6900-CGA
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="11a0", ENV{libsane_matched}="yes"
# Fujitsu ScanSnap S1500 | Fujitsu ScanSnap S1500M
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="11a2", ENV{libsane_matched}="yes"
# Fujitsu ScanSnap S1300
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="11ed", ENV{libsane_matched}="yes"
# Fujitsu fi-6125
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="11ee", ENV{libsane_matched}="yes"
# Fujitsu fi-6225
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="11ef", ENV{libsane_matched}="yes"
# Fujitsu fi-6145
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="11f1", ENV{libsane_matched}="yes"
# Fujitsu fi-6245
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="11f2", ENV{libsane_matched}="yes"
# Fujitsu fi-6135
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="11f3", ENV{libsane_matched}="yes"
# Fujitsu fi-6235
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="11f4", ENV{libsane_matched}="yes"
# Fujitsu fi-6110
ATTRS{idVendor}=="04c5", ATTRS{idProduct}=="11fc", ENV{libsane_matched}="yes"
# Konica e-mini
ATTRS{idVendor}=="04c8", ATTRS{idProduct}=="0722", ENV{libsane_matched}="yes"
# Panasonic KV-S2026C
ATTRS{idVendor}=="04da", ATTRS{idProduct}=="1000", ENV{libsane_matched}="yes"
# Panasonic KV-S2046C
ATTRS{idVendor}=="04da", ATTRS{idProduct}=="1001", ENV{libsane_matched}="yes"
# Panasonic KV-S1025C
ATTRS{idVendor}=="04da", ATTRS{idProduct}=="1006", ENV{libsane_matched}="yes"
# Panasonic KV-S1020C
ATTRS{idVendor}=="04da", ATTRS{idProduct}=="1007", ENV{libsane_matched}="yes"
# Panasonic KV-S2048C
ATTRS{idVendor}=="04da", ATTRS{idProduct}=="1009", ENV{libsane_matched}="yes"
# Panasonic KV-S2028C
ATTRS{idVendor}=="04da", ATTRS{idProduct}=="100a", ENV{libsane_matched}="yes"
# Panasonic KV-SS080
ATTRS{idVendor}=="04da", ATTRS{idProduct}=="100f", ENV{libsane_matched}="yes"
# Panasonic KV-S1045C
ATTRS{idVendor}=="04da", ATTRS{idProduct}=="1010", ENV{libsane_matched}="yes"
# Samsung SCX-4200
ATTRS{idVendor}=="04e8", ATTRS{idProduct}=="341b", ENV{libsane_matched}="yes"
# Samsung SCX4725-FN
ATTRS{idVendor}=="04e8", ATTRS{idProduct}=="341f", ENV{libsane_matched}="yes"
# Samsung SCX-4500
ATTRS{idVendor}=="04e8", ATTRS{idProduct}=="3426", ENV{libsane_matched}="yes"
# Samsung CLX-3170fn | Samsung CLX-3175FW
ATTRS{idVendor}=="04e8", ATTRS{idProduct}=="342a", ENV{libsane_matched}="yes"
# Samsung SCX-4500W
ATTRS{idVendor}=="04e8", ATTRS{idProduct}=="342b", ENV{libsane_matched}="yes"
# Samsung SCX-4824
ATTRS{idVendor}=="04e8", ATTRS{idProduct}=="342c", ENV{libsane_matched}="yes"
# Samsung SCX-4300
ATTRS{idVendor}=="04e8", ATTRS{idProduct}=="342e", ENV{libsane_matched}="yes"
# Samsung SCX-4600
ATTRS{idVendor}=="04e8", ATTRS{idProduct}=="3433", ENV{libsane_matched}="yes"
# Samsung SCX-4623
ATTRS{idVendor}=="04e8", ATTRS{idProduct}=="3434", ENV{libsane_matched}="yes"
# Samsung SCX-4825FN
ATTRS{idVendor}=="04e8", ATTRS{idProduct}=="343c", ENV{libsane_matched}="yes"
# Samsung SCX-4623FW
ATTRS{idVendor}=="04e8", ATTRS{idProduct}=="3440", ENV{libsane_matched}="yes"
# Pentax DSmobile 600
ATTRS{idVendor}=="04f9", ATTRS{idProduct}=="2038", ENV{libsane_matched}="yes"
# Aiptek Aiptek Pencam
ATTRS{idVendor}=="0553", ATTRS{idProduct}=="0202", ENV{libsane_matched}="yes"
# Mustek ScanExpress 1200 CU
ATTRS{idVendor}=="055f", ATTRS{idProduct}=="0001", ENV{libsane_matched}="yes"
# Mustek ScanExpress 600 CU
ATTRS{idVendor}=="055f", ATTRS{idProduct}=="0002", ENV{libsane_matched}="yes"
# Mustek ScanExpress 1200 UB | Trust Compact Scan USB 19200
ATTRS{idVendor}=="055f", ATTRS{idProduct}=="0006", ENV{libsane_matched}="yes"
# Mustek ScanExpress 1200 CU Plus
ATTRS{idVendor}=="055f", ATTRS{idProduct}=="0008", ENV{libsane_matched}="yes"
# Mustek BearPaw 1200 F
ATTRS{idVendor}=="055f", ATTRS{idProduct}=="0010", ENV{libsane_matched}="yes"
# Mustek ScanExpress A3 USB
ATTRS{idVendor}=="055f", ATTRS{idProduct}=="0210", ENV{libsane_matched}="yes"
# Mustek BearPaw 2400 CS | Mustek BearPaw 2400 TA | Trust 240TH Easy Webscan Gold
ATTRS{idVendor}=="055f", ATTRS{idProduct}=="0218", ENV{libsane_matched}="yes"
# Mustek BearPaw 2400 CS Plus | Mustek BearPaw 2400 TA Plus | Mustek Plug-n-Scan 2400 MT
# Mustek Plug-n-Scan 2400 M | Packard Bell Diamond 2450
ATTRS{idVendor}=="055f", ATTRS{idProduct}=="0219", ENV{libsane_matched}="yes"
# Mustek BearPaw 2448 CS Plus | Mustek BearPaw 2448 TA Plus
ATTRS{idVendor}=="055f", ATTRS{idProduct}=="021a", ENV{libsane_matched}="yes"
# Mustek BearPaw 1200 CU Plus | Packard Bell Diamond 1200 Plus
ATTRS{idVendor}=="055f", ATTRS{idProduct}=="021b", ENV{libsane_matched}="yes"
# Mustek BearPaw 1200 CU Plus | Mustek BearPaw 1248 CU | Packard Bell Diamond 1200 Plus
# Trust Direct WebScan 19200
ATTRS{idVendor}=="055f", ATTRS{idProduct}=="021c", ENV{libsane_matched}="yes"
# Mustek BearPaw 2400 CU Plus
ATTRS{idVendor}=="055f", ATTRS{idProduct}=="021d", ENV{libsane_matched}="yes"
# Mustek BearPaw 1200 CS | Mustek BearPaw 1200 TA
ATTRS{idVendor}=="055f", ATTRS{idProduct}=="021e", ENV{libsane_matched}="yes"
# Mustek ScanExpress 1248 UB
ATTRS{idVendor}=="055f", ATTRS{idProduct}=="021f", ENV{libsane_matched}="yes"
# Mustek BearPaw 2448TA Pro
ATTRS{idVendor}=="055f", ATTRS{idProduct}=="0409", ENV{libsane_matched}="yes"
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
