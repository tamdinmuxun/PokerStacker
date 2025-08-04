# Telegram Bot PokerStacker

PokerStacker keeps track of poker chips for you. Bot was written in C++ with tgbot-cpp library and libpqxx for PostreSQL database. PokerStacker for now supports only unlimited Texas Holdem variation.

# Key Features of the Poker Bot
## 🎮 Automatic Game Guidance

* No rules memorization needed – The bot tells you when to act and suggests valid moves (check, fold, raise, etc.) in real-time.

* Context-aware prompts like:
    "Your turn! Options: CHECK, RAISE 50, or FOLD"

## 💰 Smart Pot Management

* Auto-calculates pots – Handles main/side pots and split pots flawlessly.

* Instantly distributes winnings – Even in complex all-in scenarios with multiple players.

## ⏱️ Game Flow Automation

Auto-ends hands when:

* Only one player hasn’t folded ♠️♥️♦️♣️

* Only one player hasn’t gone all-in 💥

* Detects showdowns and prompts players to reveal cards.

## 🚦 Turn Enforcement

* Prevents invalid actions – Blocks out-of-turn moves or illegal bets.

* Enforces betting rules – Validates raises according to blind structure.

📊 Real-time Updates

* Live chip counts – Tracks every player’s stack after each action.

# Roadmap

* Counter mode (game with one device)

* Telegram username as default name

* Save templates of games

* Inline keyboards for more user friendly interface

* Dynamic message editing – update the game interface without spamming the chat

* Customization of blinds, time limits.

* New variations of poker
