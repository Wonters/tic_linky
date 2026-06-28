# TIC Linky — protocole et implémentation

## Objectif

Lire le signal **TIC** (Télé Information Client) d'un compteur Linky en mode standard, extraire deux grandeurs et les remonter en Zigbee :

| Étiquette TIC | Signification | Unité | Cluster Zigbee |
| ------------- | ------------- | ----- | -------------- |
| `IINST` | Intensité instantanée | A | Analog Output |
| `PAPP` | Puissance apparente instantanée | VA | Analog Value |

Le commentaire en tête de `esp_zb_tic_linky.c` liste d'autres champs TIC potentiels (`ADSC`, `EAST`, `SINSTS`, etc.) — non implémentés pour l'instant.

## Protocole TIC (mode standard)

| Paramètre | Valeur |
| --------- | ------ |
| Débit | **1200 bauds** |
| Format | **7 bits de données, parité paire, 1 stop** (7E1) |
| Début de trame | `0x02` (STX) |
| Fin de trame | `0x03` (ETX) |
| Format ligne | `ETIQUETTE valeur checksum\n` |

Exemple de trame (simulation) :

```
\x02
ADCO 031428097115 B
OPTARIF BASE 0
IINST 12 A
PAPP 2500 VA
MOTDETAT 000000 B
\x03
```

## Câblage UART

Défini dans `main/esp_zb_uart.h` :

| Signal | UART | GPIO | Broche DevKitM-1 | Rôle |
| ------ | ---- | ---- | ---------------- | ---- |
| RX TIC | UART0 | **GPIO 23** | **RX** (J3) | Réception depuis le Linky |
| TX (simu) | UART1 | **GPIO 24** | **TX** (J3) | Émission trames simulées |

Brochage complet de la carte, schémas J1/J3 et câblage Linky : **[materiel.md](materiel.md)**.

## Flux de données

```
                    ┌─────────────────────┐
  Linky TIC ───────►│ UART0 RX (GPIO 23)  │
                    └──────────┬──────────┘
                               │
                    teleinfo_measure()
                               │
                    parse_teleinfo() → IINST, PAPP
                               │
                    ┌──────────▼──────────┐
                    │  teleinfo_mutex     │
                    └──────────┬──────────┘
                               │
                    send_data_task() (5 s)
                               │
                    esp_zb_zcl_set_attribute_val()
                               │
                    Réseau Zigbee (Z2M, etc.)

  teleinfo_generator_task() ──► UART1 TX (GPIO 24)
       (simulation interne, 3 s)
```

## Parsing

Fonction `parse_teleinfo()` dans `esp_zb_uart.c` :

1. Cherche la sous-chaîne `IINST` dans la trame
2. Extrait la valeur jusqu'au `\n`
3. Idem pour `PAPP`
4. Stocke dans `TeleinfoValues` (`char iinst[8]`, `char papp[8]`)

Les valeurs sont converties en `float` via `atoi()` dans `send_data_task()`.

## Mode simulation

Le firmware inclut une tâche de simulation (`teleinfo_generator_task`), mais **elle n'alimente pas automatiquement les mesures** dans la config actuelle.

| Mécanisme | État par défaut | Effet réel |
| --------- | --------------- | ---------- |
| `HOOK_TELEINFO` | `0` dans `esp_zb_uart.h` | N'injecte pas `frame_buffer` dans les mesures |
| `teleinfo_generator_task` | toujours lancée | Émet sur **GPIO 24** (UART1 TX) toutes les 3 s |
| `teleinfo_measure()` | lit **GPIO 23** (UART0 RX) | Ne reçoit rien sans Linky ni boucle TX→RX |
| `simulated_teleinfo_frame` | — | `IINST` 10–14 A, `PAPP` 2300–3200 VA |

### Développer sans Linky

Choisir une des options :

1. **Jumper TX → RX** — relier GPIO 24 à GPIO 23 sur le DevKitM-1 (voir [materiel.md](materiel.md))
2. **`HOOK_TELEINFO = 1`** — forcer l'usage direct de `frame_buffer` sans passer par l'UART RX
3. Désactiver `teleinfo_generator_task` dans `esp_zb_tic_linky.c` si plus nécessaire

### Brancher un vrai Linky

1. Sortie TIC du compteur → broche **RX** (GPIO 23)
2. Masse Linky → **GND**
3. Laisser `HOOK_TELEINFO` à `0`
4. Optionnel : désactiver `teleinfo_generator_task` pour ne plus émettre sur GPIO 24

## Périodicité

| Tâche | Intervalle | Action |
| ----- | ---------- | ------ |
| `teleinfo_generator_task` | 3 s | Émet une trame simulée |
| `send_data_task` | 5 s | Lit les données et publie sur Zigbee |

Les mesures ne sont publiées que si `current > 0` ou `power > 0`.

## Fichier `simulation_linky.c`

Programme autonome (non compilé dans le firmware principal) qui :

- Utilise UART1 sur GPIO 16/17
- Calcule le checksum TIC correct (`calculate_checksum`)
- Envoie des lignes `IINST` et `PAPP` formatées

Utile comme référence pour valider le parsing ou tester le hardware avec un émetteur TIC logiciel.

## Checksum TIC

Chaque ligne suit le format :

```
ETIQUETTE valeur checksum
```

Le checksum est calculé comme `(somme ASCII étiquette + valeur) & 0x3F + 0x20`.

La simulation interne (`simulated_teleinfo_frame`) utilise un format simplifié sans checksum sur chaque ligne — suffisant pour le parsing actuel qui cherche uniquement les étiquettes `IINST` et `PAPP`.

## Évolutions possibles

- Parser d'autres étiquettes (`SINSTS`, `EAST`, `LTARF`, …)
- Désactiver la simulation via Kconfig
- Ajouter le calcul checksum sur les trames simulées
- Bufferiser les trames complètes (STX → ETX) plutôt que lire par chunks UART
