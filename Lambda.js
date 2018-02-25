'use strict';
var stateLight;
var https = require('https');
const Alexa = require("alexa-sdk");
var AWS = require('aws-sdk');

var config = {};
config.IOT_BROKER_ENDPOINT      = "a3g1fmc9jba095.iot.us-east-1.amazonaws.com".toLowerCase();;
config.IOT_BROKER_REGION        = "us-east-1";
config.IOT_THING_NAME           = "SciJoyYun3Thing";

AWS.config.region = config.IOT_BROKER_REGION;

//Initializing client for IoT
var iotData = new AWS.IotData({
	endpoint: config.IOT_BROKER_ENDPOINT,
	 apiVersion: '2015-05-28'
});


exports.handler = function(event, context) {
		const alexa = Alexa.handler(event, context);
    alexa.dynamoDBTableName = 'SensorTracking';
		alexa.registerHandlers(handlers);
		alexa.execute();
};


const handlers = {
	'LaunchRequest': function () {

    this.emit(':ask', 'Please name a sensor you would like to log, for example say force sensor.', 'Say see plot to have any active plots sent to the Alexa app.');
    this.emit(':responseReady');

		},

	'SensorIntent': function () {
    var filledSlots = delegateSlotCollection.call(this);
		console.log(this.event.request.intent);

    var self = this;
    var currentSensor = this.event.request.intent.slots.sensor.resolutions.resolutionsPerAuthority[0].values[0].value.id;
    this.attributes['currentSensorDB'] = currentSensor;
    var sensorSampleRate = this.event.request.intent.slots.sampleRate.value;
	    var SR0 = sensorSampleRate.slice(2,-1);   // shows 23
	    var MSH = sensorSampleRate.slice(-1);
	    var sampleRatePi;
	    if(MSH == "S"){
	      sampleRatePi = SR0 * 1000;
	    }
	    else if (MSH == "M"){
	      sampleRatePi = SR0 * 60000;
	    }
	     else if (MSH == "H"){
	      sampleRatePi = SR0 * 3600000;
	    }

    var sensorDuration = this.event.request.intent.slots.sensorDuration.value;
    var SD0 = sensorDuration.slice(2,-1);   // shows 23
    var MSHD = sensorDuration.slice(-1);
	    var sensorDurationPi;
	    if(MSHD == "S"){
	      sensorDurationPi = SD0 * 1000;
	    }
	    else if (MSHD == "M"){
	      sensorDurationPi = SD0 * 60000;
	    }
	     else if (MSHD == "H"){
	      sensorDurationPi = SD0 * 3600000;
	    }
    var sensorThreshold = this.event.request.intent.slots.threshold.value;
    var sensorThresholdMM = this.event.request.intent.slots.minMax.resolutions.resolutionsPerAuthority[0].values[0].value.id;

			//if (sensorThresholdMM == 0 || 1){
      var payloadObj={ "state":
                        { "desired":{
                            "Sensor" : currentSensor,
                            "SampleRate": sampleRatePi,
                            "SensorDuration" : sensorDurationPi,
                            "ThresholdMM": sensorThresholdMM,
                            "Threshold" : sensorThreshold
                        }
                     }
    	};

    var paramsUpdate = {
                payload : JSON.stringify(payloadObj),
                thingName : config.IOT_THING_NAME,
  };

//}

    iotData.updateThingShadow(paramsUpdate, function(err, data) {

      if (err) {
            self.response.speak('Error');
            console.log(err, err.stack); // an error occurred
            self.emit(':responseReady');
      }

      else{
            console.log(data);           // successful response
            self.emit(':responseReady');
      }
    });



    //Threshold, new sensor, actutor or exit?
    var actuatorYN = this.event.request.intent.slots.actuatorYN.value;
    if (actuatorYN === "yes") {
      this.emit(':ask', 'Say use motor, use solenoid, or use LED', 'Say use and then a name of an actuator');
      this.emit(':responseReady');
    } else {
      this.emit(':ask', 'Please say new sensor or quit.', 'Say new sensor to add another sensor and quit to exit');
      this.emit(':responseReady');

      }
	},

	'ActuatorIntent': function () {
    var filledSlots = delegateSlotCollection.call(this);
    console.log(this.event.request.intent);

    var self = this;
    var currentSensor = this.attributes['currentSensorDB'];

    var currentActuator = this.event.request.intent.slots.actuator.resolutions.resolutionsPerAuthority[0].values[0].value.id;
    var triggerMinMax = this.event.request.intent.slots.triggerMinMax.resolutions.resolutionsPerAuthority[0].values[0].value.id;
    var trigger = this.event.request.intent.slots.trigger.value;
    var actuatorDuration = this.event.request.intent.slots.actuatorDuration.value;
        var AD0 = actuatorDuration.slice(2,-1);   // shows 23
        var MSHAD = actuatorDuration.slice(-1);
        var actuatorDurationPi;
        if(MSHAD == "S"){
          actuatorDurationPi = AD0 * 1000;
        }
        else if (MSHAD == "M"){
          actuatorDurationPi = AD0 * 60000;
        }
         else if (MSHAD == "H"){
          actuatorDurationPi = AD0 * 3600000;
				}

      var payloadObj={ "state":
                        { "desired":{
                            "Actuator": currentActuator,
                            "Trigger": trigger,
                            "TriggerMM" : triggerMinMax,
                            "ActuatorDuration": actuatorDurationPi
                        }}};

    var paramsUpdate = {
                payload : JSON.stringify(payloadObj),
                thingName : config.IOT_THING_NAME,
               };

    iotData.updateThingShadow(paramsUpdate, function(err, data) {

      if (err) {
            self.response.speak('Error');
            console.log(err, err.stack); // an error occurred
            self.emit(':responseReady');
      }

      else{
            console.log(data);           // successful response
            self.emit(':responseReady');
      }
    }
    );


    //Threshold, new sensor, actutor or exit?
    var newSensorYN = this.event.request.intent.slots.newSensorYN.value;
    if (newSensorYN === "yes") {
      this.emit(':ask', 'Say new sensor to log another sensor', 'Say new sensor.');
      this.emit(':responseReady');
    } else {
      this.emit(':ask', 'Thank you, goodbye', 'Goodbye');
      this.emit(':responseReady');

      }
	},

	'AMAZON.HelpIntent': function () {
			const speechOutput = this.t('HELP_MESSAGE');
			const reprompt = this.t('HELP_MESSAGE');
			this.emit(':ask', speechOutput, reprompt);
	},
	'AMAZON.CancelIntent': function () {
			this.emit(':tell', this.t('STOP_MESSAGE'));

	},

	'AMAZON.StopIntent': function () {
			this.emit(':tell', this.t('STOP_MESSAGE'));
	},

	'Unhandled': function() {
			this.emit(':responseReady');
	}
};

function delegateSlotCollection(){
  console.log("in delegateSlotCollection");
  console.log("current dialogState: "+this.event.request.dialogState);
    if (this.event.request.dialogState === "STARTED") {
      console.log("in Beginning");
      var updatedIntent=this.event.request.intent;
      //optionally pre-fill slots: update the intent object with slot values for which
      //you have defaults, then return Dialog.Delegate with this updated intent
      // in the updatedIntent property
      this.emit(":delegate", updatedIntent);
     var sampleRateSlot = this.event.request.intent.slots.sampleRate.value;
     this.emit(':ask', sampleRateSlot, 'working yet?');
     this.emit(':responseReady');

    } else if (this.event.request.dialogState !== "COMPLETED") {
      console.log("in not completed");
      // return a Dialog.Delegate directive with no updatedIntent property.
      this.emit(":delegate");
    } else {
      console.log("in completed");
      console.log("returning: "+ JSON.stringify(this.event.request.intent));
      // Dialog is now complete and all required slots should be filled,
      // so call your normal intent handler.
      return this.event.request.intent;
    }
}
