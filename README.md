# TJ2MID
Technos Japan (GB/GBC) to MIDI converter

This tool converts music (and sound effects) from Game Boy and Game Boy Color games using Technos Japan's sound engine to MIDI format. This sound engine is used not only in games by Technos, but also other companies.

It works with ROM images. To use it, you must specify the name of the ROM followed by the number of the bank containing the sound data (in hex).
For games that contain multiple banks of music, you must run the program multiple times specifying where each different bank is located. However, in order to prevent files from being overwritten, the MIDI files from the previous bank must either be moved to a separate folder or renamed.

Note that for many games, there are "empty" tracks. This is normal.

Examples:
* TJ2MID "Double Dragon (U) [!].gb" 3
* TJ2MID "Nintendo World Cup (U) [!].gb" 3
* TJ2MID "Dig Dug (E) [!].gb" 2
* TJ2MID "Cross Hunter - Monster Hunter Version (J) [C][!].gbc" 2
* TJ2MID "Cross Hunter - Monster Hunter Version (J) [C][!].gbc" 3


This tool was based on my own reverse-engineering of the sound engine, partially based on disassembly of Double Dragon's sound code.

Also included is another program, TJ2TXT, which prints out information about the song data from each game. This is essentially a prototype of TJ2MID.

Supported games:
  * 3 Choume no Tama: Tama and Friends: 3 Choume Obake Panic
  * Adventure Island II
  * Azarashi no Sentai: Dokidori Daisakusen!
  * Beach Volleyball
  * Bikkuri Nekketsu Shikiroku!: Dokodomo Kin Medal
  * Blade
  * Bonk's Adventure/B.C. Kid
  * Bouken: Dondoki Shima
  * Cross Hunter: Monster Hunter Version
  * Cross Hunter: Treasure Hunter Version
  * Cross Hunter: X Hunter Version
  * Dig Dug
  * DokiDoki Densetsu: Mahoujin Guruguru
  * Double Dragon
  * Double Dragon II
  * Dragon Ball Z: Gokuu Gekitouden
  * Dragon Ball Z: Gokuu Hishouden
  * GB Pachislot Hissyouhou Jr.
  * Gyouten Ningen Batseelor: Doctor Guy no Yabou
  * Jida Igeki
  * Jinsei Game
  * Klax (GB, JP)
  * Marmalade Boy
  * Mickey's Dangerous Chase
  * Mighty Morphin Power Rangers
  * Mini 4 Boy
  * Mini 4 Boy II: Final Evolution
  * Momotarou Densetsu 1-2
  * Monster Truck
  * Nakayoshi Cooking Series 3: Tanoshii Obentou
  * Nakayoshi Cooking Series 4: Tanoshii Dessert
  * Nakayoshi Cooking Series 5: Tanoshii Cake
  * Nakayoshi Pet Series 1: Kawaii Hamster
  * Nakayoshi Pet Series 2: Kawaii Usagi
  * Nakayoshi Pet Series 3: Kawaii Koinu
  * Nakayoshi Pet Series 4: Kawaii Koneko
  * Nakayoshi Pet Series 5: Kawaii Hamster 2
  * Nekketsu Koukou Dodgeball-bu
  * New SD Gundam
  * Nintendo World Cup
  * Ojarumaru: Mitsunegai Jinja no Ennichi de Ojaru!
  * Pocket Pro Wrestling: Perfect Wrestler
  * Purikura Pocket 3: Talent Debut Daisakusen
  * Snowboard Champion
  * Tamagotchi
  * Tamagotchi 2
  * Tokio Senki
  * Ultraman Ball
  * Undoukai
  * X-Men: Mutant Wars
  * Xtreme Wheels
  * Zen-Nippon Pro Wrestling Jet

## To do:
  * Support for the NES version of the sound engine
  * GBS file support
