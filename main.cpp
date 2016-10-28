#include <windows.h>
#include <iostream>
#include "RobotMsg.pb.h" 
#include "MQTTClient.h"

 
int setupMQTT(MQTTClient &roboClient)
{
	MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
	MQTTClient_willOptions wopts = MQTTClient_willOptions_initializer;
	std::string  macadress=  "e3::34::43::a2";

	 
	int rc = MQTTClient_create(&roboClient, "tcp://iot.eclipse.org:1883", macadress.c_str(), MQTTCLIENT_PERSISTENCE_DEFAULT, NULL);

	opts.keepAliveInterval = 20;
	opts.cleansession = 1;
	opts.will = &wopts;
	opts.will->message = "4711 disconnected";
	opts.will->qos = 1;
	opts.will->retained = 0;
	opts.will->topicName = "Robo/Disconnect";


	try
	{
		int rc = MQTTClient_connect(roboClient, &opts);

		if(rc != MQTTCLIENT_SUCCESS)  std::cout << " error \n";
	}
	catch(...)
	{
		 
	}

	return  0;
}

	
	int main(int argc, char** argv)
	{
		 
		std::cout << "Hello ProtoMQTT  \n";
	
		RobotMsg msg;
	    std::cout << "byte size is " <<  msg.ByteSize() << "\n";
		std::cout << "size is "  << sizeof(msg) << "\n";
	
	 
	msg.set_robotstate(RobotMsg_RobotStates_Unkown);
	msg.set_devicename(std::string("Kraftwerk"));
	std::cout << "byte size is " <<  msg.ByteSize() << "\n";
	
	
	MQTTClient roboClient;
	int rc = setupMQTT(roboClient);

	if(NULL != roboClient)
	{
		int i=0;

		do 
		{
			 
			MQTTClient_deliveryToken dt;
			MQTTClient_message pubmsg = MQTTClient_message_initializer;
			pubmsg.payload = (void*)(msg.SerializeAsString()).c_str();
			pubmsg.payloadlen =(int)(msg.SerializeAsString()).size();
			

			pubmsg.retained = 0;
			std::string topic = "Robo/Info";
			int rc = MQTTClient_publish(roboClient, topic.c_str(), pubmsg.payloadlen, pubmsg.payload, pubmsg.qos, pubmsg.retained, &dt);

			if (pubmsg.qos > 0)
			{
				rc = MQTTClient_waitForCompletion(roboClient, dt, 5000L);
			}

			Sleep(500);

		} while (1);
		

	}

	
	return 0;
}