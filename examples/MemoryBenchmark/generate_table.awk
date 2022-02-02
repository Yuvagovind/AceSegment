#!/usr/bin/gawk -f
#
# Usage: generate_table.awk < ${board}.txt
#
# Takes the file generated by collect.sh and generates an ASCII
# table that can be inserted into the README.md.

BEGIN {
  labels[0] = "baseline"
  labels[1] = "DirectModule";
  labels[2] = "DirectFast4Module";
  labels[3] = "Hybrid(HardSpi)";
  labels[4] = "Hybrid(HardSpiFast)";
  labels[5] = "Hybrid(SimpleSpi)";
  labels[6] = "Hybrid(SimpleSpiFast)";
  labels[7] = "Hc595(HardSpi)";
  labels[8] = "Hc595(HardSpiFast)";
  labels[9] = "Hc595(SimpleSpi)";
  labels[10] = "Hc595(SimpleSpiFast)";
  labels[11] = "Tm1637(SimpleTmi1637)";
  labels[12] = "Tm1637(SimpleTmi1637Fast)";
  labels[13] = "Tm1638(SimpleTmi1638)";
  labels[14] = "Tm1638(SimpleTmi1638Fast)";
  labels[15] = "Max7219(HardSpi)";
  labels[16] = "Max7219(HardSpiFast)";
  labels[17] = "Max7219(SimpleSpi)";
  labels[18] = "Max7219(SimpleSpiFast)";
  labels[19] = "Ht16k33(TwoWire)";
  labels[20] = "Ht16k33(SimpleWire)";
  labels[21] = "Ht16k33(SimpleWireFast)";
  record_index = 0
}
{
  u[record_index]["flash"] = $2
  u[record_index]["ram"] = $4
  record_index++
}
END {
  NUM_ENTRIES = record_index

  # Calculate the flash and memory deltas from baseline
  base_flash = u[0]["flash"]
  base_ram = u[0]["ram"]
  for (i = 0; i < NUM_ENTRIES; i++) {
    if (u[i]["flash"] != "-1") {
      u[i]["d_flash"] = u[i]["flash"] - base_flash
      u[i]["d_ram"] = u[i]["ram"] - base_ram
    } else {
      u[i]["d_flash"] = -1
      u[i]["d_ram"] = -1
    }
  }

  printf("+--------------------------------------------------------------+\n")
  printf("| functionality                   |  flash/  ram |       delta |\n")
  for (i = 0 ; i < NUM_ENTRIES; i++) {
    if (u[i]["flash"] == "-1") continue

    name = labels[i]
    if (name ~ /baseline/ \
        || name ~ /DirectModule/ \
        || name ~ /Hybrid\(HardSpi\)/ \
        || name ~ /Hc595\(HardSpi\)/ \
        || name ~ /Tm1637\(SimpleTmi1637\)/ \
        || name ~ /Tm1638\(SimpleTmi1638\)/ \
        || name ~ /Max7219\(HardSpi\)/ \
        || name ~ /Ht16k33\(TwoWire\)/) {
      printf(\
        "|---------------------------------+--------------+-------------|\n")
    }
    printf("| %-31s | %6d/%5d | %5d/%5d |\n",
        name, u[i]["flash"], u[i]["ram"], u[i]["d_flash"], u[i]["d_ram"])
  }
  printf("+--------------------------------------------------------------+\n")
}
