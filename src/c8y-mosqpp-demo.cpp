#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <mosquitto.h>

/* Defining macro constants */
#define MQTT_HOST "novanta.us.cumulocity.com"
#define MQTT_PORT 1883
#define MQTT_USERNAME "t457419014/lucas.liberman@novanta.com"
#define MQTT_PASSWORD "<password>"
#define MQTT_TOPIC "s/us"
#define MQTT_CLIENT_ID "CPP-MQTT-Test-Lliberman-00"

// Callback for when the client receives a CONNACK message from the broker.
void on_connect(struct mosquitto *mosq, void *obj, int responce_code)
{
	/* Display connection result using mosquitto_connack_string() to
	 * produce an appropriate string for MQTT v3.x. */
	std::cout << "on_connect: " << mosquitto_connack_string(responce_code) << std::endl;
	if(responce_code != 0){
		/* if connection fails for any reason, disconnect, that way the client
		 * will not be trying to repeatedly connect to the broker. */
		mosquitto_disconnect(mosq);
	}
	std::cout << "Connected to MQTT broker.\n";
}

/* Callback for when the client knows to the best of its abilities that a PUBLISH
 * message has been successfully sent. */
void on_publish(struct mosquitto *mosq, void *obj, int mid)
{
	std::cout << "Message with mid: " << mid << " has been published.\n";
}

std::string get_temperature(void)
{
	/* Prevent too many messages from being published at once */
	sleep(1);
	/* Generate random int between 0 and 100*/
	int temperature = rand()%101;
	return std::to_string(temperature);
}


void publish_temperature(struct mosquitto *mosq)
{
	std::string payload {"211," + get_temperature()};	// Use code 211 for temperature.
	std::cout << payload << std::endl;
	int rc; // response code.


	rc = mosquitto_publish(mosq, NULL, MQTT_TOPIC, sizeof(payload), payload.c_str(), 2, false);

	if(rc != MOSQ_ERR_SUCCESS){
		std::cout << "Error publishing: " << mosquitto_strerror(rc) << std::endl;
	}
}

int main()
{
	struct mosquitto *mosq;
	int rc; // response code


    mosquitto_lib_init();

    // Create a new client
    mosq = mosquitto_new(MQTT_CLIENT_ID, true, NULL);
    if(mosq == NULL){
    	std:: cout << "Error: out of memory.\n";
    	return 1;
    }

    /* Configure callbacks before connecting */
    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_publish_callback_set(mosq, on_publish);

    /* Obtain authorization to C8Y MQTT broker */
    rc = mosquitto_username_pw_set(mosq, MQTT_USERNAME, MQTT_PASSWORD);
    if(rc != MOSQ_ERR_SUCCESS){
    	std::cout << "Error: failed to authenticate user.\n";
    	mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
    	return 1;
    }

    /* Connect to MQTT broker. */
    rc = mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 60);
    if(rc != MOSQ_ERR_SUCCESS){
    	std::cout << "Error: Failed to connect to MQTT broker.\n";
    	mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
    	return 1;
    }

    /* Run the network loop in a background thread.  */
    rc = mosquitto_loop_start(mosq);
    if(rc != MOSQ_ERR_SUCCESS){
    	std::cout << "Error: Failed to loop MQTT connection.\n";
    	mosquitto_destroy(mosq);
    	mosquitto_lib_cleanup();
    	return 1;
    }
    else{
    	while(rc == MOSQ_ERR_SUCCESS){
    	    publish_temperature(mosq);
    	}
    }

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}
