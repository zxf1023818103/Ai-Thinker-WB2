
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include <FreeRTOS.h>
#include <task.h>

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_shadow_interface.h"
#include "aws_test_cert.h"


static void testdisconnectCallbackHandler(AWS_IoT_Client *pClient, void *data){
	printf("MQTT Disconnect\r\n");
	IoT_Error_t rc = FAILURE;

	if(NULL == pClient) {
		return;
	}

	if(aws_iot_is_autoreconnect_enabled(pClient)) {
		printf("Auto Reconnect is enabled, Reconnecting attempt will start now\r\n");
	} else {
		printf("Auto Reconnect not enabled. Starting manual reconnect...\r\n");
		rc = aws_iot_mqtt_attempt_reconnect(pClient);
		if(NETWORK_RECONNECTED == rc) {
			printf("Manual Reconnect Successful\r\n");
		} else { 
			printf("Manual Reconnect Failed - %d\r\n", rc);
		}
	}
}

void aws_iot_demo_publish(void *arg){
    IoT_Error_t rc = FAILURE;
	char testdata[100];
	int testflag = 0;
	AWS_IoT_Client client;
	IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
	IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

	IoT_Publish_Message_Params paramsQOS0;
	printf("\nAWS IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

	mqttInitParams.enableAutoReconnect = false; 
	mqttInitParams.pHostURL = TEST_MQTT_HOST;
	mqttInitParams.port = TEST_MQTT_PORT;
	mqttInitParams.pRootCALocation = TEST_ROOT_CA_FILENAME;
	mqttInitParams.pDeviceCertLocation = TEST_CERTIFICATE_FILENAME;
	mqttInitParams.pDevicePrivateKeyLocation = TEST_PRIVATE_KEY_FILENAME;
	mqttInitParams.mqttCommandTimeout_ms = 20000;
	mqttInitParams.tlsHandshakeTimeout_ms = 5000;
	mqttInitParams.isSSLHostnameVerify = true;
	mqttInitParams.disconnectHandler = testdisconnectCallbackHandler;
	mqttInitParams.disconnectHandlerData = NULL;

	rc = aws_iot_mqtt_init(&client, &mqttInitParams);
	if(SUCCESS != rc) {
		printf("aws_iot_mqtt_init returned error : %d \r\n", rc);
		goto exit;
	}

	connectParams.keepAliveIntervalInSec = 600;
	connectParams.isCleanSession = true;
	connectParams.MQTTVersion = MQTT_3_1_1;
	connectParams.pClientID = TEST_MQTT_CLIENT_ID;
	connectParams.clientIDLen = (uint16_t) strlen(TEST_MQTT_CLIENT_ID);
	connectParams.isWillMsgPresent = false;

	printf("Connecting...\r\n");
	rc = aws_iot_mqtt_connect(&client, &connectParams);
	if(SUCCESS != rc) {
		printf("Error(%d) connecting to %s:%d \r\n", rc, mqttInitParams.pHostURL, mqttInitParams.port);
		goto exit;
	}
	/*
	 * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
	 *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
	 *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
	 */
	rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
	if(SUCCESS != rc) {
		printf("Unable to set Auto Reconnect to true - %d\r\n", rc);
		goto exit;
	}

	while((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc)) {
		//Max time the yield function will wait for read messages
		rc = aws_iot_mqtt_yield(&client, 100);
		if(NETWORK_ATTEMPTING_RECONNECT == rc) {
			// If the client is attempting to reconnect we will skip the rest of the loop.
			continue;
		}
		printf("-->sleep\r\n");
		vTaskDelay(1000);
		paramsQOS0.qos = QOS0;
		paramsQOS0.isRetained = 0;
		memset(testdata, 0, 100);
		if (!testflag){
			testflag = 1;
			memcpy(testdata, "Light ON", strlen("LIGHT ON"));
		}else{
			testflag = 0;
			memcpy(testdata, "LIGHT OFF", strlen("LIGHT OFF"));
		}
		paramsQOS0.payload = (void *) testdata;
		paramsQOS0.payloadLen = strlen(testdata);
		rc = aws_iot_mqtt_publish(&client, TEST_MYPUBTOPIC, strlen(TEST_MYPUBTOPIC), &paramsQOS0);
		if (rc == MQTT_REQUEST_TIMEOUT_ERROR) {
			printf("publish ack not received.\r\n");
			rc = SUCCESS;
		}
	}
exit:
	printf("\ntest task exit \r\n");
	if(SUCCESS != rc) {
        printf("An error occurred in the loop %d\r\n", rc);
    }
	aws_iot_mqtt_yield(&client, 100);
	aws_iot_mqtt_disconnect(&client);
	aws_iot_mqtt_free(&client);
	vTaskDelete(NULL);
}

