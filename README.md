# Spotter
Discord bot written in C++ designed to notify about certain social network events by using their API.

## Features
### Spotter part (regular bot)
- twitch stream notification system with minimal delay (few seconds)
- website changes checker - checks whether specified website contains specified text (can be used to check product availability in silicon shortage times)
- fully managed through discord commands with multi-server support
### Personal part (self bot)
- twitter notification system (checks for new tweets)
- monitoring discord messages for those from certain array of people

## Why
Twitch has its own notification system, however it's for some reason most of the time delayed by 10-30 minutes. The goal of this was to get notifications as soon as streams start - which was achieved. At first, I had wanted to add instagram and twitter API as well, but after several exhausting minutes trying to figure out why I need to setup 2 independent facebook pages and apply for business account on 3 different places, I gave up on the idea and declared instagram's built-in notification system relatively decent. Twitter API is on the other hand way easier to setup, however there is rate limit of 900 request per 15 minutes with no option to combine requests whatsoever. If the bot would become even slightly popular, these notifications could and would be delayed just like are official twitch notifications - this breaks the whole idea why I decided to make this in the first place.

## Setup (using visual studio)
1. git clone --recurse-submodules https://github.com/rikodot/spotter.git
2. open folder in visual studio
3. wait for cmake magic to finish
4. set values in config.h
5. right click CMakeLists.txt -> Set as Startup Item
6. build
7. profit ??
- (errors? make sure you have required tools, for help check - [sleepydiscord docs](https://yourwaifu.dev/sleepy-discord/docs))

## Future updates
I may deploy the bot publicly to be used as it was meant to. I may or may not implement more API functionalities as well as including different social networks or other functionalities in general. I will keep updating this repository to ensure its functionality as long as I personally use the bot.

## Disclaimer
- Personal part may be used as a regular bot or not used at all. I, however, want to use it as self bot so I can utilize its whole potential. In such case, as I mentioned in the code, according to [Discord ToS](https://discord.com/terms), self bots are forbidden and may result in your account being terminated - [official article](https://support.discord.com/hc/articles/115002192352).
- I have written most of this while bored in school using vim through ssh on phone. My apologies for terrible way of *(not)* catching exceptions.

## Credits
[SleepyDiscord](https://github.com/yourWaifu/sleepy-discord) - incredible C++ discord library  
[NlohmannJson](https://github.com/nlohmann/json) - best C++ json library out there  
[Libcurl](https://github.com/curl/curl) - multiprotocol C transfer library

## Preview
![discord preview](https://i.imgur.com/dkALwBz.png)
![console preview](https://i.imgur.com/0T9pddx.png)
