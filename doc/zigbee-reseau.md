# Zigbee — réseau et intégration

## Rôle de l'appareil

| Paramètre | Valeur |
| --------- | ------ |
| Rôle | **End Device** (`ESP_ZB_DEVICE_TYPE_ED`) |
| Config Kconfig | `CONFIG_ZB_ZED=y` |
| Keep alive | 3000 ms |
| ED aging timeout | 64 min |
| Install code | Désactivé (`install_code_policy = false`) |
| Canaux | Tous (`ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK`) |

Un **End Device** ne route pas le trafic et peut passer en sommeil (selon config stack). C'est adapté à un capteur de mesure comme le Linky.

## Identité sur le réseau

| Paramètre | Valeur |
| --------- | ------ |
| Endpoint | `10` |
| Profile | Home Automation (`0x0104`) |
| Device ID | Temperature Sensor (`ESP_ZB_HA_TEMPERATURE_SENSOR_DEVICE_ID`) |
| Fabricant (Basic) | `Shift` |
| Modèle (Basic) | `linky` |
| Date code | `20240515` |

> Le device ID `Temperature Sensor` est un héritage de l'exemple Espressif. Les mesures sont exposées via **Analog Output** et **Analog Value**, pas via le cluster Temperature Measurement. À adapter si l'intégration Z2M ne reconnaît pas correctement l'appareil.

## Clusters exposés

| Cluster | ID | Attribut | Mesure |
| ------- | -- | -------- | ------ |
| Basic | `0x0000` | Manufacturer, Model, versions | Identification |
| Identify | `0x0003` | identify_time | Identification visuelle |
| Analog Output | `0x000D` | present_value (`0x0055`) | Courant `IINST` (A) |
| Analog Value | `0x000E` | present_value (`0x0055`) | Puissance `PAPP` (VA) |

Publication dans `send_data_task()` :

```c
esp_zb_zcl_set_attribute_val(HA_ESP_LINKY_ENDPOINT,
    ESP_ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT, ...,
    ESP_ZB_ZCL_ATTR_ANALOG_OUTPUT_PRESENT_VALUE_ID, &current, false);

esp_zb_zcl_set_attribute_val(HA_ESP_LINKY_ENDPOINT,
    ESP_ZB_ZCL_CLUSTER_ID_ANALOG_VALUE, ...,
    ESP_ZB_ZCL_ATTR_ANALOG_VALUE_PRESENT_VALUE_ID, &power, false);
```

Mise à jour toutes les **5 secondes** si les valeurs sont > 0.

## Commissioning (rejoindre un réseau)

Séquence au démarrage :

1. **ZDO Config Ready** — stack initialisée
2. **Device First Start** (usine) → `ESP_ZB_BDB_MODE_NETWORK_STEERING`
3. **Steering** — scan des réseaux, tentative de join
4. Si succès → log PAN ID, canal, short address + clignotements LED
5. Si échec → retry steering après 1 seconde

Au **reboot** (déjà sur un réseau) :

1. Init LED différée (`deferred_driver_init`)
2. Vérification statut commissioning
3. Si pas sur réseau → re-steering
4. Si sur réseau → `start_blinking()`

En cas de **perte de lien** (`ESP_ZB_NWK_SIGNAL_NO_ACTIVE_LINKS_LEFT`) :

- Re-steering automatique
- Clignotements LED

## Intégration Zigbee2MQTT

### Procédure

1. Coordinateur en **permit join** (Zigbee2MQTT : `permit_join: true`)
2. Alimenter ou reset l'ESP32-H2
3. L'appareil fait du network steering et rejoint le réseau
4. Il apparaît dans Z2M avec modèle `linky` / fabricant `Shift`
5. Les attributs Analog Output / Analog Value devraient exposer courant et puissance

### Si l'appareil ne rejoint pas

- Vérifier que permit join est actif
- `./activate -p PORT erase-flash` puis re-flash (efface NVS Zigbee)
- Rapprocher l'appareil du coordinateur
- Vérifier les canaux Zigbee (l'appareil scanne tous les canaux 11–26)

### Si l'appareil était sur un autre réseau

Le steering ne rejoint pas automatiquement un autre PAN sans factory reset :

```bash
./activate -p PORT erase-flash
./activate -p PORT flash
```

## Logs utiles

```
I ESP_ZB_TIC_LINKY: Zigbee stack initialized
I ESP_ZB_TIC_LINKY: First start
I ESP_ZB_TIC_LINKY: Start network steering
I ESP_ZB_TIC_LINKY: Joined network successfully (Extended PAN ID: ..., PAN ID: 0x..., Channel:...)
I ESP_ZB_TIC_LINKY: Données mises à jour via Zigbee : PAPP=2500.0 VA, IINST=12.0 A
```

En cas de perte de lien :

```
W ESP_ZB_TIC_LINKY: Connection lost - attempting to rejoin network
```

## Différences avec relay-zigbee

| | ha_tic_linky | relay-zigbee |
| - | ------------ | ------------ |
| Rôle | End Device | Router |
| Cluster principal | Analog Output / Value | On/Off (`0x0006`) |
| Endpoint | 10 | 12 |
| Modèle | `linky` | `lightOnOff` |
| Données | Mesures TIC | État on/off |
