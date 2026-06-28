# Architecture du code

## Arborescence

```
ha_tic_linky/
├── main/
│   ├── esp_zb_tic_linky.c   # Point d'entrée Zigbee, commissioning, envoi mesures
│   ├── esp_zb_tic_linky.h   # Config Zigbee (endpoint, radio, rôle ED)
│   ├── esp_zb_uart.c        # UART TIC, parsing, simulation interne
│   ├── esp_zb_uart.h        # Pins UART, structure TeleinfoValues
│   ├── light_driver.c       # Pilote LED WS2812 (RMT)
│   ├── light_driver.h       # GPIO LED, API on/off
│   ├── blinker.c            # Clignotements LED au join réseau
│   ├── blinker.h
│   ├── esp_battery.c        # Mesure ADC batterie (non utilisé actuellement)
│   ├── esp_battery.h
│   ├── simulation_linky.c   # Simulateur TIC autonome (non compilé)
│   ├── CMakeLists.txt       # Enregistrement des sources
│   └── idf_component.yml    # Dépendances Component Manager
├── managed_components/      # Libs téléchargées (voir managed-components.md)
├── doc/                     # Documentation technique
├── activate                 # Script activation ESP-IDF
├── partitions.csv
├── sdkconfig.defaults
└── CMakeLists.txt           # project(HA_TIC_LINKY)
```

## Flux d'exécution

```
app_main()
  ├── nvs_flash_init()
  ├── esp_zb_platform_config()     # config radio IEEE 802.15.4
  ├── uart_init()                  # UART TIC 1200 7E1
  └── xTaskCreate(esp_zb_task)     # tâche FreeRTOS Zigbee (prio 5)
        ├── esp_zb_init()          # rôle End Device
        ├── création clusters ZCL (Basic, Identify, Analog Output, Analog Value)
        ├── esp_zb_device_register()
        ├── xTaskCreate(send_data_task)        # prio 4, toutes les 5 s
        ├── xTaskCreate(teleinfo_generator_task) # prio 3, toutes les 3 s
        └── esp_zb_main_loop_iteration()       # boucle infinie

Parallèle : esp_zb_app_signal_handler()
  ├── SKIP_STARTUP → init commissioning BDB
  ├── DEVICE_FIRST_START → network steering
  ├── DEVICE_REBOOT → re-steering + start_blinking()
  ├── STEERING OK → start_blinking()
  └── NO_ACTIVE_LINKS_LEFT → re-steering + start_blinking()

send_data_task() (toutes les 5 s)
  ├── teleinfo_measure() → lit IINST et PAPP
  └── esp_zb_zcl_set_attribute_val() → publie courant et puissance
```

## Tâches FreeRTOS

| Tâche | Priorité | Stack | Rôle |
| ----- | -------- | ----- | ---- |
| `Zigbee_main` | 5 | 4096 | Stack Zigbee + boucle principale |
| `send_data_task` | 4 | 8096 | Lit TIC, publie sur Zigbee toutes les 5 s |
| `teleinfo_gen` | 3 | 8096 | Génère des trames TIC simulées toutes les 3 s |

Synchronisation :

- `teleinfo_mutex` — accès partagé aux données TIC (UART)
- `zigbee_lock` (spinlock) — accès aux attributs ZCL Zigbee

## Fichiers détaillés

### `esp_zb_tic_linky.c`

Cœur applicatif Zigbee + orchestration des mesures.

| Fonction | Rôle |
| -------- | ---- |
| `app_main()` | Init NVS, radio, UART, lance la tâche Zigbee |
| `esp_zb_task()` | Configure et démarre la stack Zigbee + tâches données |
| `esp_zb_app_signal_handler()` | Événements réseau (join, reboot, perte lien) |
| `send_data_task()` | Lit `IINST` / `PAPP`, met à jour les clusters Analog |
| `deferred_driver_init()` | Init LED après reboot sur réseau |

Clusters enregistrés sur l'endpoint `HA_ESP_LINKY_ENDPOINT` (10) :

- **Basic** — fabricant `Shift`, modèle `linky`
- **Identify** — identification (temps = 0)
- **Analog Output** — courant instantané `IINST` (A)
- **Analog Value** — puissance apparente `PAPP` (VA)

Type d'appareil déclaré : `ESP_ZB_HA_TEMPERATURE_SENSOR_DEVICE_ID` (héritage de l'exemple — à revoir si besoin d'un device ID plus adapté).

### `esp_zb_uart.c`

Couche TIC / Téléinfo.

| Fonction | Rôle |
| -------- | ---- |
| `uart_init()` | Configure UART0 (RX) et UART1 (TX) en 1200 7E1 |
| `parse_teleinfo()` | Extrait `IINST` et `PAPP` d'une trame |
| `teleinfo_measure()` | Lit l'UART RX ou utilise le buffer simulé |
| `simulated_teleinfo_frame()` | Génère une trame TIC de test |
| `teleinfo_generator_task()` | Envoie périodiquement des trames simulées sur UART1 TX |

Voir [teleinfo-tic.md](teleinfo-tic.md) pour le détail du protocole.

### `light_driver.c`

Abstraction matérielle de la LED.

- Driver : `espressif/led_strip` (composant managed)
- Protocole : WS2812 via **RMT**
- GPIO : `8` (défini dans `light_driver.h`)
- Couleur par défaut : blanc `(255, 255, 255)`

### `blinker.c`

| Fonction | Rôle |
| -------- | ---- |
| `start_blinking()` | Démarre un timer 500 ms |
| `toggle_light_cb()` | Alterne la LED, stop après 2 clignotements (4 toggles) |

Appelé après jonction réseau réussie ou reconnexion.

### `esp_battery.c` (désactivé)

Code présent pour mesurer la tension via ADC1 channel 6 (GPIO 34 sur ESP32 classique — **non adapté tel quel à ESP32-H2**). L'init et le cluster Analog Input sont commentés dans `esp_zb_tic_linky.c`.

### `simulation_linky.c` (non compilé)

Projet autonome de test avec `app_main()` propre. Utilise GPIO 16/17. Non inclus dans `main/CMakeLists.txt` — sert de référence pour le format des trames TIC.

## Graphe de dépendances

```
esp_zb_tic_linky.c
  ├── esp_zb_uart.h
  ├── light_driver.h
  ├── blinker.h
  └── esp_zigbee_core.h

esp_zb_uart.c
  └── esp_zb_uart.h

blinker.c
  └── light_driver.h

light_driver.c
  └── led_strip.h (managed)
```

## Points d'attention

| Sujet | Détail |
| ----- | ------ |
| Simulation | `teleinfo_generator_task` tourne mais n'alimente pas le RX sans boucle GPIO 24→23 ou `HOOK_TELEINFO=1` |
| `HOOK_TELEINFO` | Macro à `0` — voir [teleinfo-tic.md](teleinfo-tic.md#mode-simulation) |
| Device ID Zigbee | `TEMPERATURE_SENSOR` — peut limiter l'intégration dans certains coordinateurs |
| ADC batterie | Code mort pour l'instant sur ESP32-H2 |
