# Matériel

## Carte cible

| Paramètre | Valeur |
| --------- | ------ |
| SoC | **ESP32-H2** |
| Radio | IEEE 802.15.4 (Zigbee) + BLE |
| Flash | 2 Mo |
| Cible IDF | `esp32h2` |

Configuré via `idf.py set-target esp32h2` et `CONFIG_IDF_TARGET="esp32h2"` dans `sdkconfig`.

Le README Espressif mentionne aussi ESP32-C6 — ce projet est configuré pour **H2**.

## Câblage LED

| Signal | GPIO | Détail |
| ------ | ---- | ------ |
| WS2812 DATA | **GPIO 8** | 1 LED adressable |

Défini dans `main/light_driver.h` :

```c
#define CONFIG_EXAMPLE_STRIP_LED_GPIO   8
#define CONFIG_EXAMPLE_STRIP_LED_NUMBER 1
```

Driver : `espressif/led_strip` — protocole WS2812 via périphérique **RMT** à 10 MHz.

Comportement :

- **On** : pixel blanc `(255, 255, 255)`
- **Off** : `(0, 0, 0)`
- **Clignotement** : 2 cycles (4 toggles) après join réseau, puis off

## Câblage TIC Linky

Carte de référence : **ESP32-H2-DevKitM-1** ([doc Espressif](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32h2/esp32-h2-devkitm-1/user_guide.html)).

### Correspondance TX / RX (connecteur J3)

Sur le silkscreen de la carte, les broches **TX** et **RX** correspondent aux GPIO utilisés par le firmware :

| Broche silkscreen (J3) | GPIO | Fonction hardware | Dans le firmware | Rôle |
| ---------------------- | ---- | ----------------- | ---------------- | ---- |
| **RX** | **GPIO 23** | `U0RXD` | `UART0` — RX seul | **Réception TIC Linky** |
| **TX** | **GPIO 24** | `U0TXD` | `UART1` — TX seul | **Émission simulation** (dev) |

Défini dans `main/esp_zb_uart.h` :

```c
#define UART_RX_NUM UART_NUM_0   // lecture TIC
#define UART_TX_NUM UART_NUM_1   // simulation
#define RXD_PIN 23
#define TXD_PIN 24
```

**Point de vue ESP :**

- **RX (GPIO 23)** — l'ESP **écoute** → câbler la **sortie TIC du Linky** ici
- **TX (GPIO 24)** — l'ESP **émet** → utilisé par `teleinfo_generator_task` (simulateur)

**Point de vue Linky :**

- Sortie TIC du compteur → broche **RX** de l'ESP (GPIO 23)
- Masse Linky → **GND** de l'ESP

> La console USB (flash / `idf.py monitor`) passe par **USB Serial/JTAG** (GPIO 26/27), pas par GPIO 23/24.

Paramètres UART : **1200 bauds, 7E1** — voir [teleinfo-tic.md](teleinfo-tic.md).

### Branchement Linky

Le compteur Linky expose une sortie TIC sur ses bornes. Vérifier :

- Niveau de tension compatible avec l'ESP32-H2 (3.3 V TTL)
- Câblage conforme au mode TIC (pas le même câblage que l'ancien téléinfo 9600 bauds)
- Masse commune entre Linky et ESP32

### Test sans Linky (boucle locale)

Le simulateur émet sur **UART1 TX (GPIO 24)** mais la lecture se fait sur **UART0 RX (GPIO 23)**. Sans Linky, les trames simulées n'arrivent pas au parseur sauf si :

- un jumper relie **TX → RX** (GPIO 24 → GPIO 23), ou
- `HOOK_TELEINFO` est mis à `1` dans `esp_zb_uart.h`

Voir [teleinfo-tic.md](teleinfo-tic.md#mode-simulation) pour le détail.

## Brochage ESP32-H2-DevKitM-1

### Connecteur J3 (TX / RX / LED / USB)

Pins du projet en **gras** :

```
                    ESP32-H2-DevKitM-1
                    ┌─────────────────┐
         J3         │                 │
    ┌───────────┐   │                 │
    │ 1  GND    ├───┤ GND             │
    │ 2  TX     ├───┤ GPIO 24 (U0TXD) │──► simulation TIC (UART1 TX)
    │ 3  RX     ├───┤ GPIO 23 (U0RXD) │◄── sortie TIC Linky
    │ 4  10     ├───┤ GPIO 10         │
    │ 5  11     ├───┤ GPIO 11         │
    │ 6  25     ├───┤ GPIO 25         │
    │ 7  12     ├───┤ GPIO 12         │
    │ 8  8      ├───┤ GPIO 8  (LOG)   │──► LED WS2812 onboard
    │ 9  22     ├───┤ GPIO 22         │
    │10  GND    ├───┤ GND             │
    │11  9      ├───┤ GPIO 9  (BOOT)  │
    │12  GND    ├───┤ GND             │
    │13  27     ├───┤ GPIO 27 (USB_D+)│
    │14  26     ├───┤ GPIO 26 (USB_D-)│──► USB-C (flash + monitor)
    │15  GND    ├───┤ GND             │
    └───────────┘   │                 │
                    │  Antenne Zigbee │
                    └─────────────────┘
```

### Connecteur J1 (référence)

```
         J1
    ┌───────────┐
    │ 1  3V3    │
    │ 2  RST    │
    │ 3  0      │  GPIO 0
    │ 4  1      │  GPIO 1
    │ 5  2      │  GPIO 2
    │ 6  3      │  GPIO 3
    │ 7  13     │  GPIO 13 (XTAL 32k)
    │ 8  14     │  GPIO 14 (XTAL 32k)
    │ 9  4      │  GPIO 4
    │10  5      │  GPIO 5
    │11  GND    │
    │12  5V     │
    │13  GND    │
    └───────────┘
```

## USB / programmation

- Console série : **USB Serial/JTAG** intégré
- Pas besoin de convertisseur UART externe sur la plupart des devkits ESP32-H2
- Port typique macOS : `/dev/tty.usbmodem*` ou `/dev/cu.usbmodem*`
- Port configuré dans `.vscode/settings.json` : `/dev/tty.usbmodemFD3401`

## Bouton BOOT

Non utilisé dans le firmware actuel.

## Alimentation

Alimentation via USB pour le développement. Le Linky alimente sa propre sortie TIC — ne pas alimenter le compteur depuis l'ESP32.

## Cartes compatibles

Toute carte **ESP32-H2** avec :

- Antenne IEEE 802.15.4
- GPIO 8 accessible pour la LED
- GPIO 23 et 24 accessibles pour l'UART TIC
- 2 Mo flash minimum

## Schéma câblage Linky + projet

```
  Linky                          ESP32-H2 (J3)
 ┌────────┐                    ┌──────────────┐
 │ TIC    │───────────────────►│ RX  GPIO 23  │  lecture UART0
 │(sortie)│                    │              │
 │ GND    │───────────────────►│ GND          │
 └────────┘                    │              │
                               │ TX  GPIO 24  │──► (optionnel : boucle test
                               │              │     GPIO 24 → GPIO 23)
                               │ 8   GPIO 8   │──► LED WS2812 (onboard)
                               │ USB-C        │──► PC
                               └──────────────┘
                               Antenne Zigbee ◄──► réseau
```

## Fichier `simulation_linky.c` (référence)

Programme de test non compilé qui utilise des pins différentes :

| Signal | GPIO |
| ------ | ---- |
| TX | GPIO 17 |
| RX | GPIO 16 |

Ne pas confondre avec les pins du firmware principal (23/24).
