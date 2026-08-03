# Connecting Heaven's Gate Live to Lichess (Lichess Bot Integration Guide)

> **Goal**: Get an official, public Glicko-2 Elo rating on [Lichess.org](https://lichess.org) by hosting Heaven's Gate as a 24/7 online chess bot.

---

## 1. Prerequisites

1. A **Lichess Account** (create a fresh account dedicated to your bot, e.g. `HeavensGateBot`).
2. **Python 3.9+** installed on your system.
3. Your compiled `heavensgate.exe` executable.

---

## 2. Step-by-Step Setup Guide

### Step 1: Create a Lichess API Token
1. Log in to your bot account on [Lichess.org](https://lichess.org).
2. Go to **Settings** $\rightarrow$ **Personal Access Tokens** (`https://lichess.org/account/oauth/token`).
3. Click **"Generate new token"**.
4. Check the scope: **"Play games with the bot API"** (`bot:play`).
5. Copy the generated API token string.

### Step 2: Upgrade Account to Bot Account
Run this command in terminal (replace `YOUR_TOKEN`):
```bash
curl -X POST https://lichess.org/api/bot/account/upgrade -H "Authorization: Bearer YOUR_TOKEN"
```
*(Your Lichess profile will now display a purple **BOT** badge!)*

### Step 3: Clone official `lichess-bot` Bridge
```bash
git clone https://github.com/lichess-bot-devs/lichess-bot.git
cd lichess-bot
pip install -r requirements.txt
```

### Step 4: Configure `config.yml`
Create or edit `config.yml` inside the `lichess-bot` directory:

```yaml
token: "YOUR_LICHESS_BOT_TOKEN_HERE"
url: "https://lichess.org/"

engine:
  dir: "C:/Users/abhin/heavensgate"
  name: "heavensgate.exe"
  protocol: "uci"
  uci_options:
    Hash: 64
  variants:
    - standard

challenge:
  min_initial: 10
  max_initial: 300
  min_increment: 0
  max_increment: 10
  time_controls:
    - bullet
    - blitz
    - rapid
  variants:
    - standard
```

### Step 5: Launch the Bot!
```bash
python lichess-bot.py
```

Your bot will automatically accept challenges on Lichess, make moves in real-time, and calculate an **official Lichess Glicko-2 Rating** after playing 20-30 rated games against other bots and humans!
