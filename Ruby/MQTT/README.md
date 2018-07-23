# A topic-based, asynchronous MQTT handler
That's right, finally there's a *mostly* stable, connection-loss resistant MQTT handler out there that runs fully asynchronously.
What's even nicer is that you can attach (and detach!) callbacks to *any* topic you want - including wildcards!
Any topic can have as many callbacks as you want too, so don't worry about that!

## That's great, but how to get it started?
The main code is simple:
```ruby
require 'mqtt/sub_handler'

myClient = MQTT::Client.new(ADDRESS_OR_PARAMETERS); # See the "mqtt" gem for possible options!
mqttSubHandler = MQTT::SubHandler.new(myClient);    # Create a handler with a mqtt class

mqttSubHandler = MQTT::SubHandler.new('mqtts://Password:Username@Address') # Or use a string!
mqttSubHandler = MQTT.Eclipse(); # Or use the quick shortcut to 'iot.eclipse.org' for testing!
```
### Subscribing

```ruby
mySub = mqttSubHandler.subscribe_to "Any/Topic/You/Want" do |data, [topicMatch]|
	puts "I got some data: #{data}";
end
# Yup, it's that simple. The callback will be stored, and will run whenever data is received.

# You need to get rid of your subscription?
# Sure thing!

mqttSubHandler.unregister_subscription(mySub); # That'll remove the callback, and unsubscribe!
# Don't worry about breaking other subscriptions. The code checks if any other callbacks are attached to the topic in question!
```

#### Wildcard subscriptions
Wildcard subscriptions are also accepted with this code, and, may I say, are quite useful too!
```ruby
mqttSubHandler.subscribe_to "A/Wildcard/+/Topic/+" do |data, topicList|
	puts "I got some data: #{data}"
	puts "The first + was: #{topicList[0]}"
	puts "The second one was: #{topicList[1]}"
end
```
Note: This also works with the "#" wildcard. Every following topic "branch" becomes a new array element!

### Publishing
```ruby
# Pushing gets easy, too:
mqttSubHandler.publish_to "Any/Topic/You/Want", theData, [qos: 1, retain: false]
# Right now, the code ONLY SUPPORTS QOS 0, AND WILL BLOCK!
# This is courtesy of the MQTT-Gem though, and nothing I can fix for now.
```

## But ... Wait?
Yep, you can wait for data, too!

```ruby
mqttSubHandler.wait_for "The/Waiting/Topic", [timeout: seconds or nil] do |data, [topicMatch]|
	# Confirm and process data here. The rest of the code will wait!
	# Return true when you found what you need, or set an optional timeout, and the main code will continue after that.
end
```

The same goes for the end of the code, if you have nothing else to run but want to keep listening:
```ruby
mqttSubHandler.lockAndListen();
# This here also traps SIGINT, so you get a clean exit!
```
