#include <string>
#include <iostream>
#include <string.h>
#include <assert.h>

#include <mosquitto.h>

#include "config.hpp"
#include "cmd.hpp"
#include "mqtt.hpp"

using namespace std;

extern string ID;
static struct mosquitto *g_mosq = NULL;

static string mqtt_id_topic()
{
    return "redeye/camera/" + ID;
}

static void mqtt_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
    int i;

    printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
    for(i=1; i<qos_count; i++){
        printf(", %d", granted_qos[i]);
    }
    printf("\n");
}

static void mqtt_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
    clog << "MQTT" << str << endl;
}

static void mqtt_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
    int i;
    if( result ) {
        fprintf(stderr, "Connect failed\n");
        return;
    }
    /* Subscribe to broker information topics on successful connect. */
    string t = "redeye/camera/" + ID;
    mosquitto_subscribe(mosq, NULL, t.c_str(), 2);
    mqtt_publish("redeye/announce/camera", ID.c_str());
}

static void mqtt_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
    bool match = 0;
    printf("MQTT Message topic: %s - %d - %s\n", msg->topic, msg->payloadlen, (char *) msg->payload);

    if ( msg->topic == mqtt_id_topic() ) {
        cout << "MQTT CMD sent to us: " << (char *) msg->payload << endl;

        cmd_runner( (char *) msg->payload );

        return;
    }
}

int mqtt_publish(string topic, string msg)
{
    int mid;
    return mosquitto_publish( g_mosq, NULL, topic.c_str(), msg.length(), msg.c_str(), 0, false );
}

void* mqtt_loop(void *p)
{
    int i;
    int keepalive = 60;
    bool clean_session = true;

    mosquitto_lib_init();
    g_mosq = mosquitto_new(NULL, clean_session, NULL);
    if(!g_mosq){
        cerr << "MQTT New Error: Out of memory." << endl;
        return NULL;
    }
    mosquitto_log_callback_set(g_mosq, mqtt_log_callback);
    mosquitto_connect_callback_set(g_mosq, mqtt_connect_callback);
    mosquitto_message_callback_set(g_mosq, mqtt_message_callback);
    mosquitto_subscribe_callback_set(g_mosq, mqtt_subscribe_callback);

    if(mosquitto_connect(g_mosq, "10.24.10.10", 1883, keepalive)){
        cerr << "MQTT Error: Failed to connect." << endl;
        return NULL;
    }

    mosquitto_loop_forever(g_mosq, -1, 1);

    mosquitto_destroy(g_mosq);
    mosquitto_lib_cleanup();
    return NULL;
}

