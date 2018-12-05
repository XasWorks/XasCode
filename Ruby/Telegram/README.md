
# Xasin's simplifying Telegram gem
This gem is mainly meant for personal use.
Many of the existing Ruby gems for Telegram have all the functionality you want,
but also require a fairly complex setup, and good knowledge of the Telegram API.

My gem doesn't include all functionality, but aims to provide a somewhat simpler interface of
interaction with a small number (or just a single) of users.
I mainly use it for my own smart home system, to easily send and receive messages, but not much more.

## MQTT-Mode (Preferred)
The main mode of using this gem would be via the MQTT Adapter system.
It translates a lot of commonly used functionality into an asynchronous MQTT API, allowing other parts of your code, but also embedded devices like ESPs to very easily interface with the Telegram bot.

*Note:* A MQTT server is not needed! The "virtual" MQTT Server `MQTT::Testing::SubHandler` can be used too!

Setting up the system is fairly easy:

```Ruby
require 'xasin/telegram.rb'
require 'xasin/telegram/MQTT_Adapter.rb'

# Create the HTTP core, which will handle all the communication to the REST api
httpCore = Xasin::Telegram::HTTPCore.new(APIKEY);

# Now create the MQTT interface.
mqttAdapter = Xasin::Telegram::MQTT::Server.new(httpCore, mqtt);
```

This exposes the Telegram Bot's interface functions to the "Telegram/#" tree.

Incoming messages will be published to

## Single-User mode
Another way to use the HTTP-Core is by only looking at a single user.
It discards messages of all users except one, and provides a simple "send" and "on_message" interface:

```Ruby
require 'xasin/telegram.rb'

# The HTTP "core" handles sending raw commands and getting updates, but not much more.
httpCore = Xasin::Telegram::HTTPCore.new(APIKEY);

# The SingleUser class handles the aforementioned sending/receiving of messages.
# Its first argument is the Chat-ID it should use, which is NOT the user ID
# To find it out you have to perform an update request to the Telegram API, send a message to your bot,
# and get the Chat_ID from there.
# I'm working on a better way >.>
singleUser = Xasin::Telegram::SingleUser.new(CHAT_ID, httpCore);


# From then, using the user is simple. Only text is required!
returned_id = singleUser.send_message(TEXT, **args);

# The ID that is returned can then be used to edit or delete the message:
singleUser.edit_message(returned_id, NEW_TEXT);
singleUser.delete_message(returned_id);

# And to receive messages:
singleUser.on_message do |message|
	# Fetch the text from the message first!
	text = message[:text];
end
```
